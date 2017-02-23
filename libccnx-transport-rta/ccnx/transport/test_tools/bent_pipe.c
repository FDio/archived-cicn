/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>

#include <LongBow/runtime.h>
#include <LongBow/debugging.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_EventTimer.h>
#include <parc/algol/parc_EventQueue.h>
#include <parc/algol/parc_EventBuffer.h>
#include <parc/algol/parc_EventSocket.h>
#include "bent_pipe.h"

#define MAX_CONN 10

struct packet_wrapper {
    struct timeval deadline;
    int ingress_fd;
    uint8_t  *pbuff;
    size_t pbuff_len;

    TAILQ_ENTRY(packet_wrapper) list;
};

struct bentpipe_conn {
    int client_fd;
    PARCEventQueue      *bev;
    BentPipeState           *parent;

    // after reading a header, this is how long the next message is
    size_t msg_length;

    // queue to send stuff up the connection
    unsigned bytes_in_queue;
    unsigned count_in_queue;
    struct timeval last_deadline;
    TAILQ_HEAD(, packet_wrapper) output_queue;

    PARCEventTimer *timer_event;
};

struct bentpipe_state {
    char                    *local_name;
    PARCEventScheduler           *base;
    PARCEventSocket *listenerUnix;

    struct bentpipe_conn conns[MAX_CONN];
    unsigned conn_count;

    pthread_t router_thread;

    bool use_params;
    double loss_rate;
    unsigned buffer_bytes;
    double mean_sec_delay;
    double bytes_per_sec;

    pthread_mutex_t startup_mutex;
    pthread_cond_t startup_cond;
    unsigned magic;
    bool startup_running;

    // used to signal into the thread to stop
    bool killme;
    PARCEventTimer *keep_alive_event;

    // these track the state of sigpipe before we
    // go into a write, so we can re-set the state

    // sigpipe_pending indicates that it was pending before our call
    bool sigpipe_pending;

    // sigpipe_blocked means that it was blocked before our call, so
    // don't unblock it when we're done
    bool sigpipe_blocked;

    // debugging output
    bool chattyOutput;
};

typedef struct {
    uint32_t pid;
    uint32_t fd;
    uint32_t length;
    uint32_t pad;     // pad out to 16 bytes
} __attribute__ ((packed)) localhdr;

static int setup_local(BentPipeState*bp);
static void
listener_cb(int fd, struct sockaddr *sa, int socklen, void *user_data);

static void *run_bentpipe(void *arg);

static void conn_readcb(PARCEventQueue *bev, PARCEventType event, void *connection);
static void reflect(struct bentpipe_conn *conn, uint8_t *pbuff, size_t pbuff_len);

static void
queue_with_delay(struct bentpipe_conn *conn, uint8_t *pbuff, size_t pbuff_len, int i);

static void timer_cb(int fd, PARCEventType what, void *user_data);
static void set_timer(struct bentpipe_conn *conn, struct timeval delay);
static void keepalive_cb(int fd, PARCEventType what, void *user_data);
void conn_errorcb(PARCEventQueue *bev, PARCEventQueueEventType events, void *user_framework);

#define MAGIC 0x01020304

// ===============

static void
lock_bentpipe(BentPipeState *bp)
{
    int res = pthread_mutex_lock(&bp->startup_mutex);
    assertTrue(res == 0, "error from pthread_mutex_lock: %d", res);
}

static void
unlock_bentpipe(BentPipeState *bp)
{
    int res = pthread_mutex_unlock(&bp->startup_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

static void
wait_bentpipe(BentPipeState *bp)
{
    int res = pthread_cond_wait(&bp->startup_cond, &bp->startup_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

static void
signal_bentpipe(BentPipeState *bp)
{
    int res = pthread_cond_signal(&bp->startup_cond);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

/**
 * if SIGPIPE was pending before we are called, don't do anything.
 * Otherwise, mask SIGPIPE
 */
static void
capture_sigpipe(BentPipeState *bp)
{
#if !defined(SO_NOSIGPIPE)
    sigset_t pending;
    sigemptyset(&pending);
    sigpending(&pending);
    bp->sigpipe_pending = sigismember(&pending, SIGPIPE);
    if (!bp->sigpipe_pending) {
        sigset_t sigpipe_mask;
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);

        sigset_t blocked;
        sigemptyset(&blocked);
        pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &blocked);
        bp->sigpipe_blocked = sigismember(&blocked, SIGPIPE);
    }
#endif
}

static void
release_sigpipe(BentPipeState *bp)
{
#if !defined(SO_NOSIGPIPE)
    // If sigpipe was previously pending, we didnt block it, so
    // nothing new to do
    if (!bp->sigpipe_pending) {
        sigset_t pending;

        sigset_t sigpipe_mask;
        sigemptyset(&sigpipe_mask);
        sigaddset(&sigpipe_mask, SIGPIPE);

        sigemptyset(&pending);
        sigpending(&pending);

        if (!bp->sigpipe_blocked) {
            pthread_sigmask(SIG_UNBLOCK, &sigpipe_mask, NULL);
        }
    }
#endif
}

BentPipeState *
bentpipe_Create(const char *local_name)
{
    assertNotNull(local_name, "Parameter local_name must be non-null");

    struct timeval keepalive_timeout = { .tv_sec = 0, .tv_usec = 500000 };

    BentPipeState *bp = parcMemory_AllocateAndClear(sizeof(struct bentpipe_state));
    assertNotNull(bp, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct bentpipe_state));

    bp->local_name = parcMemory_StringDuplicate((char *) local_name, 1024);
    bp->conn_count = 0;
    bp->base = parcEventScheduler_Create();
    bp->use_params = false;
    bp->chattyOutput = false;

    if (!bp->base) {
        fprintf(stderr, "Could not initialize PARCEventScheduler!\n");
        return NULL;
    }

    pthread_mutex_init(&bp->startup_mutex, NULL);
    pthread_cond_init(&bp->startup_cond, NULL);
    bp->startup_running = false;
    bp->magic = MAGIC;
    bp->keep_alive_event = parcEventTimer_Create(bp->base, PARCEventType_Persist, keepalive_cb, bp);
    parcEventTimer_Start(bp->keep_alive_event, &keepalive_timeout);

    for (int i = 0; i < MAX_CONN; i++) {
        bp->conns[i].bytes_in_queue = 0;
        TAILQ_INIT(&bp->conns[i].output_queue);
        bp->conns[i].timer_event = parcEventTimer_Create(bp->base, 0, timer_cb, &bp->conns[i]);
    }
    setup_local(bp);
    capture_sigpipe(bp);
    return bp;
}

void
bentpipe_Destroy(BentPipeState **bpPtr)
{
    BentPipeState *bp;
    assertNotNull(bpPtr, "%s got null double pointer\n", __func__);
    bp = *bpPtr;
    assertNotNull(bp, "%s got null dereference\n", __func__);

    assertFalse(bp->startup_running, "calling destroy on a running bentpipe\n");

    int i;
    for (i = 0; i < MAX_CONN; i++) {
        if (bp->conns[i].client_fd > 0) {
            // this closes it too
            parcEventQueue_Destroy(&(bp->conns[i].bev));
            bp->conns[i].client_fd = 0;
        }
        parcEventTimer_Destroy(&(bp->conns[i].timer_event));
    }

    parcEventTimer_Destroy(&(bp->keep_alive_event));
    parcEventSocket_Destroy(&(bp->listenerUnix));
    parcEventScheduler_Destroy(&(bp->base));
    bp->conn_count = 0;

    int failure = unlink(bp->local_name);
    if (failure) {
        printf("Error unlinking '%s': (%d) %s\n", bp->local_name, errno, strerror(errno));
    }

    release_sigpipe(bp);
    parcMemory_Deallocate((void **) &(bp->local_name));
    parcMemory_Deallocate((void **) &bp);
}

void
bentpipe_SetChattyOutput(BentPipeState *bp, bool chattyOutput)
{
    bp->chattyOutput = chattyOutput;
}

int
bentpipe_Start(BentPipeState *bp)
{
    assertFalse(bp->startup_running, "bentpipe_Start already running");
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


    assertTrue(pthread_create(&bp->router_thread, &attr, run_bentpipe, bp) == 0, "pthread_create failed.");
//    if (pthread_create(&bp->router_thread, &attr, run_bentpipe, bp) != 0) {
//        perror("pthread_create router thread");
//        abort();
//    }

    lock_bentpipe(bp);
    while (!bp->startup_running) {
        wait_bentpipe(bp);
    }
    unlock_bentpipe(bp);

    return 0;
}

int
bentpipe_Stop(BentPipeState *bp)
{
    assertTrue(bp->magic == MAGIC, "Magic is not magic value! got %08X", bp->magic);
    assertTrue(bp->startup_running, "Calling stop on a stopped bentpipe\n");

    bp->killme = true;

    // wait until exist
    lock_bentpipe(bp);
    while (bp->startup_running) {
        wait_bentpipe(bp);
    }
    unlock_bentpipe(bp);

    return 0;
}

// ==================================

void
listener_errorcb(PARCEventScheduler *base, int error, char *errorString, void *addr_unix)
{
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", error, errorString);

    parcEventScheduler_Stop(base, NULL);
}

static int
setup_local(BentPipeState*bp)
{
    // cleanup anything left on file system
    unlink(bp->local_name);

    struct sockaddr_un addr_unix;
    memset(&addr_unix, 0, sizeof(addr_unix));

    addr_unix.sun_family = PF_UNIX;
    strcpy(addr_unix.sun_path, bp->local_name);

    if (bp->chattyOutput) {
        printf("bent_pipe Creating '%s'", bp->local_name);
    }

    bp->listenerUnix = parcEventSocket_Create(bp->base,
                                              listener_cb,
                                              listener_errorcb,
                                              (void *) bp,
                                              (struct sockaddr*) &addr_unix,
                                              sizeof(addr_unix));

    assertNotNull(bp->listenerUnix, "parcEventSocket_Create failed: unix %s", bp->local_name);

    return 0;
}

static void *
run_bentpipe(void *arg)
{
    BentPipeState *bp = (BentPipeState *) arg;

    if (bp->chattyOutput) {
        printf("%s starting\n", __func__);
    }

    // The keepalive timer will signal that we have started running
    parcEventScheduler_Start(bp->base, PARCEventSchedulerDispatchType_Blocking);

    if (bp->chattyOutput) {
        printf("%s exiting\n", __func__);
    }

    lock_bentpipe(bp);
    bp->startup_running = false;
    signal_bentpipe(bp);
    unlock_bentpipe(bp);

    pthread_exit(NULL);
}

static struct bentpipe_conn *
allocate_connection(BentPipeState *bp)
{
    if (bp->conn_count == MAX_CONN) {
        printf("allocate_connection: connection count is %d, maximum count is %d\n",
               bp->conn_count, MAX_CONN);
        return NULL;
    }

    for (int i = 0; i < MAX_CONN; i++) {
        if (bp->conns[i].client_fd == 0) {
            bp->conn_count++;
            return &bp->conns[i];
        }
    }

    // should never get here
    abort();
}

static void
deallocate_connection(BentPipeState *bp, struct bentpipe_conn *conn)
{
    assertTrue(bp->conn_count > 0, "invalid state, called deallocate_connection when conn_count is zero");

    conn->client_fd = 0;

    if (bp->chattyOutput) {
        printf("destroying connection %p eventqueue %p\n", (void *) conn, (void *) conn->bev);
    }

    parcEventQueue_Disable(conn->bev, PARCEventType_Read);
    parcEventQueue_Destroy(&(conn->bev));

    // this unschedules any callbacks, but the timer is still allocated
    // timer_event is freed in bentPipe_Destroy()
    parcEventTimer_Stop(conn->timer_event);

    bp->conn_count--;
}

/*
 * Server accepts a new client
 */
static void
listener_cb(int fd, struct sockaddr *sa, int socklen, void *user_data)
{
    BentPipeState *bp = (BentPipeState *) user_data;

    // allocate a connection
    struct bentpipe_conn *conn = allocate_connection(bp);

    conn->parent = bp;
    conn->client_fd = fd;

    // Set non-blocking flag
    int flags = fcntl(fd, F_GETFL, NULL);
    assertTrue(flags != -1, "fcntl failed to obtain file descriptor flags (%d)\n", errno);
    int failure = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    assertFalse(failure, "fcntl failed to set file descriptor flags (%d)\n", errno);

#if defined(SO_NOSIGPIPE)
    int set = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *) &set, sizeof(int));
#endif

    conn->bev = parcEventQueue_Create(bp->base, fd, PARCEventQueueOption_CloseOnFree | PARCEventQueueOption_DeferCallbacks);
    if (!conn->bev) {
        fprintf(stderr, "Error constructing parcEventQueue!");
        parcEventScheduler_Abort(bp->base);
        return;
    }

    parcEventQueue_SetCallbacks(conn->bev, conn_readcb, NULL, conn_errorcb, (void *) conn);
    parcEventQueue_Enable(conn->bev, PARCEventType_Read);

    if (bp->chattyOutput) {
        printf("%s accepted connection on fd %d conn %p eventqueue %p\n",
               __func__, fd, (void *) conn, (void *) conn->bev);
    }
}

/*
 * Return 1 if we read an entire message and we can try another one.
 * return 0 if we're waiting for bytes.
 */
static int
single_read(PARCEventQueue *bev, struct bentpipe_conn *conn)
{
    BentPipeState *bp = conn->parent;
    PARCEventBuffer *input = parcEventBuffer_GetQueueBufferInput(bev);
    size_t read_length = parcEventBuffer_GetLength(input);
    int ret = 0;

    if (bp->chattyOutput) {
        printf("single_read: connid %d read %zu bytes\n", conn->client_fd, read_length);
    }

    if (read_length == 0) {
        // 0 length means EOF, close the connection
        if (bp->chattyOutput) {
            printf("single_read: connid %d read_length %zu, EOF, closing connection\n",
                   conn->client_fd,
                   read_length);
        }
        deallocate_connection(conn->parent, conn);
        parcEventBuffer_Destroy(&input);
        return 0;
    }

    // head we read the message header, which sets the message length?
    if (conn->msg_length == 0) {
        // did we read a whole header?
        if (read_length >= sizeof(localhdr)) {
            // yes we did, we can now read the message header then try to read the whole message
            // Note that this does not remove the header from the buffer

            localhdr *msg_hdr = (localhdr *) parcEventBuffer_Pullup(input, sizeof(localhdr));
            conn->msg_length = msg_hdr->length;

            assertTrue(conn->msg_length > 0, "single_read: msg_hdr length is 0!");
            assertTrue(conn->msg_length < 64000, "single_read: msg_hdr length too large: %zu", conn->msg_length);

            if (bp->chattyOutput) {
                printf("single_read: %s start read_length %zu msg_length %zu\n", __func__, read_length, conn->msg_length);
            }
        } else {
            // no we did not, wait for more data before procesing.

            if (bp->chattyOutput) {
                printf("single_read: %s short read %zu\n", __func__, read_length);
            }
        }
    }

    // if read_length < sizeof(localhdr), then this is false.
    // Otherwise, we've read the header and set msg_length, so if read_length
    // is greater than the sum we have a full message plus header

    if (read_length >= sizeof(localhdr) + conn->msg_length) {
        size_t pbuff_len = sizeof(localhdr) + conn->msg_length;
        int bytes_removed;

        uint8_t *pbuff = parcMemory_Allocate(pbuff_len);
        assertNotNull(pbuff, "parcMemory_Allocate(%zu) returned NULL", pbuff_len);

        // dequeue into packet buffer
        bytes_removed = parcEventBuffer_Read(input, (void *) pbuff, pbuff_len);
        assertTrue(bytes_removed == pbuff_len, "parcEventBuffer read wrong length, expected %zu got %d", pbuff_len, bytes_removed);

        // now reset message length for next packet
        conn->msg_length = 0;

        if (bp->chattyOutput) {
            printf("connid %d msg_length %zu read_length %zu, resetting low water mark\n",
                   conn->client_fd,
                   pbuff_len,
                   read_length);

            longBowDebug_MemoryDump((char *) pbuff, pbuff_len);
        }

        // reflect will free the memory
        reflect(conn, pbuff, pbuff_len);

        // we could do more after this
        if (read_length > pbuff_len) {
            ret = 1;
        }
    } else {
        // we do not have an entire message, so skip and wait for more to be read
    }
    parcEventBuffer_Destroy(&input);
    return ret;
}

static void
conn_readcb(PARCEventQueue *bev, PARCEventType event, void *user_data)
{
    struct bentpipe_conn *conn = (struct bentpipe_conn *) user_data;

    // drain the input buffer
    while (single_read(bev, conn)) {
        // empty
    }
}

/*
 * reflect a message to other connections
 *
 * We should use the zero-copy deferred write...
 */
static void
reflect(struct bentpipe_conn *conn, uint8_t *pbuff, size_t pbuff_len)
{
    int i;
    BentPipeState *bp = conn->parent;

    for (i = 0; i < MAX_CONN; i++) {
        if (bp->conns[i].client_fd > 0 && bp->conns[i].client_fd != conn->client_fd) {
            int res;

            if (bp->chattyOutput) {
                printf("%s connid %d adding buffer length %zu\n", __func__, conn[i].client_fd, pbuff_len);
            }

            if (bp->use_params) {
                uint8_t *copy = parcMemory_Allocate(pbuff_len);
                assertNotNull(copy, "parcMemory_Allocate(%zu) returned NULL", pbuff_len);
                memcpy(copy, pbuff, pbuff_len);
                queue_with_delay(conn, copy, pbuff_len, i);
            } else {
                res = parcEventQueue_Write(bp->conns[i].bev, pbuff, pbuff_len);
                assertTrue(res == 0, "%s got parcEventQueue_Write error\n", __func__);

                localhdr *msg_hdr = (localhdr *) pbuff;
                assertTrue(msg_hdr->length + sizeof(localhdr) == pbuff_len,
                           "msg_hdr messed up!  expected %zu got %zu",
                           msg_hdr->length + sizeof(localhdr),
                           pbuff_len);
            }
        }
    }

    parcMemory_Deallocate((void **) &pbuff);
}

/**
 * Queue a packet for later delivery.  We calculate the needed delay and insert it
 * in the connection's output_queue.  If there is not a timer running (i.e. there are now
 * exactly 1 elements in the queue), we start the timer for the connection.
 *
 * If the output queue is full, the packet might be freed and not added to the output queue.
 */
static void
queue_with_delay(struct bentpipe_conn *conn, uint8_t *pbuff, size_t pbuff_len, int i)
{
    BentPipeState *bp = conn->parent;
    double delay_sec;

    struct timeval now;
    struct timeval delay_tv;
    struct timeval deadline;

    gettimeofday(&now, NULL);

    // 1) Apply loss rate
    if (drand48() < bp->loss_rate) {
        if (bp->chattyOutput) {
            printf("%s random drop\n", __func__);
        }
        parcMemory_Deallocate((void **) &pbuff);
        return;
    }

    // 2) will it fit?
    if (pbuff_len + bp->conns[i].bytes_in_queue >= bp->buffer_bytes) {
        if (bp->chattyOutput) {
            printf("%s queue full\n", __func__);
        }
        parcMemory_Deallocate((void **) &pbuff);
        return;
    }

    // 3) Determine delay
    delay_sec = (double) pbuff_len / bp->bytes_per_sec;

    // 4) exponential delay
    delay_sec += -1 * log(drand48()) * bp->mean_sec_delay;

    delay_tv.tv_sec = (time_t) floor(delay_sec);
    delay_tv.tv_usec = (suseconds_t) floor((delay_sec - floor(delay_sec)) * 1E+6);

    struct packet_wrapper *wrapper = parcMemory_Allocate(sizeof(struct packet_wrapper));
    assertNotNull(wrapper, "parcMemory_Allocate(%zu) returned NULL", sizeof(struct packet_wrapper));
    wrapper->ingress_fd = conn->client_fd;
    wrapper->pbuff = pbuff;
    wrapper->pbuff_len = pbuff_len;

    timeradd(&now, &delay_tv, &deadline);

    wrapper->deadline = deadline;
    bp->conns[i].last_deadline = deadline;
    bp->conns[i].bytes_in_queue += pbuff_len;

    bp->conns[i].count_in_queue++;
    TAILQ_INSERT_TAIL(&bp->conns[i].output_queue, wrapper, list);

    if (bp->chattyOutput) {
        printf("%s queue %d fd %d count %d\n", __func__, i, bp->conns[i].client_fd, bp->conns[i].count_in_queue);
    }

    // if this is first item in queue, set a timer
    if (bp->conns[i].count_in_queue == 1) {
        set_timer(&bp->conns[i], delay_tv);
    }
}

int
bentpipe_Params(BentPipeState *bp, double loss_rate, unsigned buffer_bytes, double mean_sec_delay, double bytes_per_sec)
{
    bp->use_params = true;
    bp->loss_rate = loss_rate;
    bp->buffer_bytes = buffer_bytes;
    bp->mean_sec_delay = mean_sec_delay;
    bp->bytes_per_sec = bytes_per_sec;
    return 0;
}

static
void
keepalive_cb(int fd, PARCEventType what, void *user_data)
{
    struct bentpipe_state *bp = (struct bentpipe_state *) user_data;

    if (!bp->startup_running) {
        // indicate to anyone waiting that we're really running
        if (bp->chattyOutput) {
            printf("%s signalling startup_running\n", __func__);
        }

        lock_bentpipe(bp);
        bp->startup_running = true;
        signal_bentpipe(bp);
        unlock_bentpipe(bp);
        return;
    }

    if (bp->killme) {
        parcEventScheduler_Abort(bp->base);
        return;
    }
}

/**
 * Each connection has its own timer.  The timer is used to defer sending packets to a later time,
 * such as to realize traffic shaping.
 */
static void
timer_cb(int fd, PARCEventType what, void *user_data)
{
    struct bentpipe_conn *conn = (struct bentpipe_conn *) user_data;
    BentPipeState *bp = conn->parent;
    struct timeval now;

    gettimeofday(&now, NULL);
    while (!TAILQ_EMPTY(&conn->output_queue)) {
        int res;
        struct packet_wrapper *wrapper = TAILQ_FIRST(&conn->output_queue);

        assertTrue(conn->count_in_queue > 0, "invalid state: count_in_queue is 0");

        if (timercmp(&now, &wrapper->deadline, <)) {
            break;
        }

        conn->bytes_in_queue -= wrapper->pbuff_len;
        conn->count_in_queue--;

        res = parcEventQueue_Write(conn->bev, wrapper->pbuff, wrapper->pbuff_len);
        assertTrue(res == 0, "got parcEventQueue_Write error\n");

        if (bp->chattyOutput) {
            printf("%3.9f output conn %d bytes %zu\n",
                   now.tv_sec + now.tv_usec * 1E-6, conn->client_fd, wrapper->pbuff_len);
        }

        TAILQ_REMOVE(&conn->output_queue, wrapper, list);

        parcMemory_Deallocate((void **) &(wrapper->pbuff));
        parcMemory_Deallocate((void **) &wrapper);
    }

    if (!TAILQ_EMPTY(&conn->output_queue)) {
        struct packet_wrapper *wrapper = TAILQ_FIRST(&conn->output_queue);
        struct timeval delay;
        timersub(&wrapper->deadline, &now, &delay);

        if (bp->chattyOutput) {
            printf("connid %d scheduling next timer delay %.6f\n",
                   conn->client_fd, delay.tv_sec + 1E-6 * delay.tv_usec);
        }

        assertTrue(delay.tv_sec >= 0 && delay.tv_usec >= 0,
                   "Got negative delay: now %.6f deadline %.6f delay %.6f",
                   now.tv_sec + 1E-6 * now.tv_usec,
                   wrapper->deadline.tv_sec + 1E-6 * wrapper->deadline.tv_usec,
                   delay.tv_sec + 1E-6 * delay.tv_usec);

        if (delay.tv_sec == 0 && delay.tv_usec < 1000) {
            delay.tv_usec = 1000;
        }

        set_timer(conn, delay);
    }
}

static void
set_timer(struct bentpipe_conn *conn, struct timeval delay)
{
    BentPipeState *bp = conn->parent;
    // this replaces any prior events

    if (delay.tv_usec < 1000) {
        delay.tv_usec = 1000;
    }

    if (delay.tv_sec < 0) {
        delay.tv_sec = 0;
    }

    if (bp->chattyOutput) {
        printf("%s connid %d delay %.6f timer_event %p\n",
               __func__,
               conn->client_fd,
               delay.tv_sec + 1E-6 * delay.tv_usec,
               (void *) conn->timer_event);
    }

    parcEventTimer_Start(conn->timer_event, &delay);
}

void
conn_errorcb(PARCEventQueue *bev, PARCEventQueueEventType events, void *user_data)
{
    if (events & PARCEventQueueEventType_EOF) {
        struct bentpipe_conn *conn = (struct bentpipe_conn *) user_data;

        if (conn->parent->chattyOutput) {
            printf("%s Got EOF on connid %d fd %d socket\n",
                   __func__,
                   conn->client_fd,
                   parcEventQueue_GetFileDescriptor(bev));
        }

        deallocate_connection(conn->parent, conn);
    }

    if (events & PARCEventQueueEventType_Error) {
        struct bentpipe_conn *conn = (struct bentpipe_conn *) user_data;

        printf("%s Got error on connid %d fd %d socket: %s\n",
               __func__,
               conn->client_fd,
               parcEventQueue_GetFileDescriptor(bev),
               strerror(errno));

        deallocate_connection(conn->parent, conn);
    }
}
