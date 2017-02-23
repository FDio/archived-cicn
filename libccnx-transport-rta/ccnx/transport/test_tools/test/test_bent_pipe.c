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

//
//  test_bent_pipe.c
//  Libccnx
//

#include "../bent_pipe.c"

#include <stdio.h>
#include <sys/un.h>
#include <strings.h>
#include <sys/queue.h>
#include <errno.h>

#include <LongBow/unit-test.h>
#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#define CHATTY 1

static const char *local_name = "/tmp/alpha";

LONGBOW_TEST_RUNNER(BentPipe)
{
    LONGBOW_RUN_TEST_FIXTURE(CreateDestroy);
    LONGBOW_RUN_TEST_FIXTURE(System);
}

LONGBOW_TEST_RUNNER_SETUP(BentPipe)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(BentPipe)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ================================

LONGBOW_TEST_FIXTURE(CreateDestroy)
{
    LONGBOW_RUN_TEST_CASE(CreateDestroy, create_destroy);
    LONGBOW_RUN_TEST_CASE(CreateDestroy, create_start_stop_destroy);
}

LONGBOW_TEST_FIXTURE_SETUP(CreateDestroy)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(CreateDestroy)
{
    if (parcMemory_Outstanding() != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(CreateDestroy, create_destroy)
{
    BentPipeState *bp = bentpipe_Create(local_name);
    bentpipe_Destroy(&bp);
}

LONGBOW_TEST_CASE(CreateDestroy, create_start_stop_destroy)
{
    BentPipeState *bp = bentpipe_Create(local_name);
    bentpipe_Start(bp);
    bentpipe_Stop(bp);
    bentpipe_Destroy(&bp);
}

// ================================

BentPipeState *system_bp;

LONGBOW_TEST_FIXTURE(System)
{
    LONGBOW_RUN_TEST_CASE(System, two_connections);
}

LONGBOW_TEST_FIXTURE_SETUP(System)
{
    system_bp = bentpipe_Create(local_name);
    bentpipe_Start(system_bp);

    printf("%s created system_bp %p, running %d\n", __func__, (void *) system_bp, system_bp->startup_running);

    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(System)
{
    printf("%s stopping system_bp %p, running %d\n", __func__, (void *) system_bp, system_bp->startup_running);

    bentpipe_Stop(system_bp);
    bentpipe_Destroy(&system_bp);
    if (parcMemory_Outstanding() != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }

    return LONGBOW_STATUS_SUCCEEDED;
}

int
connect_to_bentpipe(const char *pipe_name)
{
    int res;
    struct sockaddr_un addr_unix;
    int fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket PF_LOCAL");
    }

    assertFalse(fd < 0, "socket PF_LOCAL error");

    memset(&addr_unix, 0, sizeof(addr_unix));
    addr_unix.sun_family = AF_UNIX;
    strcpy(addr_unix.sun_path, pipe_name);

    res = connect(fd, (struct sockaddr *) &addr_unix, sizeof(addr_unix));
    if (res < 0) {
        perror("connect");
    }
    assertTrue(res == 0, "error on connect");
    return fd;
}

#define MAXSEND 1024
#define CONN_COUNT 3
#define MAXPENDING 128

struct sendlist {
    uint8_t buffer[MAXSEND];
    size_t length;
    unsigned refcount;
};

struct fdstate {
    int fd;

    struct sendlist *expected[MAXPENDING];
    unsigned head;
    unsigned tail;
    unsigned count_expected;

    unsigned count_send;
    unsigned count_recv;

    // these are for the next message being read
    size_t total_read_length;
    size_t current_read_length;
    uint8_t pbuff[MAXSEND + 16];
};

void
sendbuffer(int fd, struct fdstate *state)
{
    struct sendlist *entry = parcMemory_AllocateAndClear(sizeof(struct sendlist));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(struct sendlist));
    size_t len, total_len;
    localhdr *hdr = (localhdr *) entry->buffer;
    int i;
    ssize_t res;

    len = random() % (MAXSEND - sizeof(localhdr) - 1) + 1;
    hdr->pid = getpid();
    hdr->fd = fd;
    hdr->length = (int) len;

    total_len = len + sizeof(localhdr);

    entry->length = total_len;
    entry->refcount = 0;

    for (i = 0; i < CONN_COUNT; i++) {
        if (state[i].fd != fd) {
            entry->refcount++;
            state[i].expected[ state[i].tail ] = entry;
            state[i].tail = (state[i].tail + 1) % MAXPENDING;
            state[i].count_expected++;

            assertFalse(state[i].tail == state[i].head,
                        "%s buffer wrap around on fd %d",
                        __func__,
                        state[i].fd);

            if (CHATTY) {
                printf("conn %2d added expected cnt %u length %zu\n",
                       i,
                       state[i].count_expected,
                       entry->length);
            }
        } else {
            state[i].count_send++;

            if (CHATTY) {
                printf("conn %2d sent count %u\n", i, state[i].count_send);
            }
        }
    }

    res = write(fd, entry->buffer, entry->length);
    assertTrue(res == entry->length, "%s write error %s\n", __func__, strerror(errno));
}

static int
compare_sends(struct fdstate *s, uint8_t *buffer, size_t buffer_len)
{
    struct sendlist *expected = s->expected[ s->head ];
    int res;

    s->head = (s->head + 1) % MAXPENDING;

    assertTrue(expected->length == buffer_len, "%s lengths do not match, expected %zu got %zu\n",
               __func__, expected->length, buffer_len);

    if (expected->length != buffer_len) {
        return -1;
    }

    res = memcmp(expected->buffer, buffer, buffer_len);
    assertTrue(res == 0, "%s buffers did not match\n", __func__);
    if (res != 0) {
        return -1;
    }

    assertTrue(expected->refcount > 0, "%s invalid refcount\n", __func__);

    expected->refcount--;
    if (expected->refcount == 0) {
        memset(expected, 0, sizeof(struct sendlist));
        parcMemory_Deallocate((void **) &expected);
    }

    return 0;
}

/*
 * Create two connections to bent pipe and make sure they reflect
 */
LONGBOW_TEST_CASE(System, two_connections)
{
    struct fdstate state[CONN_COUNT];
    fd_set fdset, readset;
    int number_writes = 100;
    int count_writes = 0;
    int i;
    struct timeval timeout = { 0, 10000 };
    unsigned pending_expected = 0;

    assertNotNull(system_bp, "%s running with null system_bp\n", __func__);

    FD_ZERO(&fdset);
    for (i = 0; i < CONN_COUNT; i++) {
        memset(&state[i], 0, sizeof(struct fdstate));
        state[i].fd = connect_to_bentpipe(local_name);
        FD_SET(state[i].fd, &fdset);
    }

    sleep(1);

    assertTrue(system_bp->conn_count == CONN_COUNT, "bp conn count wrong");

    while (count_writes < number_writes || pending_expected > 0) {
        int res;
        memcpy(&readset, &fdset, sizeof(readset));

        res = select(FD_SETSIZE, &readset, NULL, NULL, &timeout);
        if (res < 0) {
            perror("select");
            abort();
        }

        if (res > 0) {
            if (CHATTY) {
                printf("%s got res %d\n", __func__, res);
            }
            for (i = 0; i < CONN_COUNT; i++) {
                if (FD_ISSET(state[i].fd, &readset)) {
                    ssize_t res;
                    localhdr *hdr = (localhdr *) state[i].pbuff;

                    if (state[i].total_read_length == 0) {
                        size_t remaining = sizeof(localhdr) - state[i].current_read_length;
                        // we need to read a header
                        res = read(state[i].fd,
                                   state[i].pbuff + state[i].current_read_length,
                                   remaining);

                        assertFalse(res < 0, "%s got read error: %s", __func__, strerror(errno));

                        state[i].current_read_length += res;
                        if (state[i].current_read_length == sizeof(localhdr)) {
                            state[i].total_read_length = sizeof(localhdr) + hdr->length;

                            if (CHATTY) {
                                printf("%s conn %d fd %d set total length %zu\n",
                                       __func__,
                                       i,
                                       state[i].fd,
                                       state[i].total_read_length);
                            }
                        }
                    }

                    if (state[i].current_read_length < state[i].total_read_length) {
                        size_t remaining = state[i].total_read_length - state[i].current_read_length;
                        // we need to read a header
                        res = read(state[i].fd,
                                   state[i].pbuff + state[i].current_read_length,
                                   remaining);

                        assertFalse(res < 0, "%s got read error: %s", __func__, strerror(errno));
                        state[i].current_read_length += res;
                    }

                    if (state[i].current_read_length == state[i].total_read_length) {
                        // verify that it's the same as the top expected stack
                        res = compare_sends(&state[i], state[i].pbuff, state[i].total_read_length);

                        assertTrue(res == 0, "%s invalid receive compare\n", __func__);

                        state[i].count_recv++;
                        state[i].count_expected--;

                        if (CHATTY) {
                            printf("%s conn %d fd %d cnt_recv %u cnt_expected %u\n",
                                   __func__,
                                   i, state[i].fd, state[i].count_recv, state[i].count_expected);
                        }

                        // done with it
                        state[i].current_read_length = 0;
                        state[i].total_read_length = 0;
                    }
                }
            }
        }

        if ((random() % 4) == 0) {
            // do a write
            int out = random() % CONN_COUNT;

            if (CHATTY) {
                printf("%s sendbuffer for conn %d fd %d\n", __func__, out, state[out].fd);
            }

            sendbuffer(state[out].fd, state);
            count_writes++;
        }

        pending_expected = 0;
        for (i = 0; i < CONN_COUNT; i++) {
            pending_expected += state[i].count_expected;
        }
    }

    for (i = 0; i < CONN_COUNT; i++) {
        printf("conn %2d fd %2d send %4u recv %4u\n",
               i,
               state[i].fd,
               state[i].count_send,
               state[i].count_recv);

        assertTrue(state[i].count_recv == number_writes - state[i].count_send + 1,
                   "%s conn %d incorrect counts\n",
                   __func__);
        close(state[i].fd);
    }
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(BentPipe);
    exit(longBowMain(argc, argv, testRunner, NULL));
}
