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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../rta_WebService.c"
#include <LongBow/unit-test.h>
#include <parc/algol/parc_SafeMemory.h>

#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

LONGBOW_TEST_RUNNER(rta_WebService)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(rta_WebService)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(rta_WebService)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, rtaWebService_Create_Destroy);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

struct sigaction save_sigchld;
struct sigaction save_sigpipe;

static void
blockSigChild()
{
    struct sigaction ignore_action;
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;

    sigaction(SIGCHLD, NULL, &save_sigchld);
    sigaction(SIGPIPE, NULL, &save_sigpipe);

    sigaction(SIGCHLD, &ignore_action, NULL);
    sigaction(SIGPIPE, &ignore_action, NULL);
}

static void
unblockSigChild()
{
    sigaction(SIGCHLD, &save_sigchld, NULL);
    sigaction(SIGPIPE, &save_sigpipe, NULL);
}

LONGBOW_TEST_CASE(Global, rtaWebService_Create_Destroy)
{
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "error on socketpair: (%d) %s", errno, strerror(errno));

    RtaFramework *framework = rtaFramework_Create(fds[1]);

    // we should be runing on port 9090, so the string popen() gets
    // will look like this:
    // tcp4       0      0  127.0.0.1.9090                 *.*                    LISTEN

    blockSigChild();
    FILE *fp = popen("netstat -an -p tcp", "r");
    assertNotNull(fp, "Got null opening netstat for reading");

    char str[1035];
    bool found = false;
    while (fgets(str, sizeof(str) - 1, fp) != NULL) {
        if (strstr(str, "127.0.0.1.9090") != NULL) {
            found = true;
            break;
        }

        if (strstr(str, "127.0.0.1:9090") != NULL) {
            found = true;
            break;
        }
    }

    pclose(fp);

    rtaFramework_Destroy(&framework);

    close(fds[0]);
    close(fds[1]);
    unblockSigChild();

    assertTrue(found, "Did not find 127.0.0.1.9090 in netstat output");
}


LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, rtaWebService_ProcessHelloRequest);
    LONGBOW_RUN_TEST_CASE(Local, rtaWebService_ProcessRequest);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Local, rtaWebService_ProcessHelloRequest)
{
#ifndef __APPLE__
    testSkip("Test broken on non-darwin");
#endif

    blockSigChild();
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "error on socketpair: (%d) %s", errno, strerror(errno));

    RtaFramework *framework = rtaFramework_Create(fds[0]);
    rtaFramework_Start(framework);
    rtaFramework_WaitForStatus(framework, FRAMEWORK_RUNNING);

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(9090);

    failure = connect(fd, (struct sockaddr *) &sin, sizeof(sin));
    assertFalse(failure, "error on connect: (%d) %s", errno, strerror(errno));

    char request[] = "GET /hello HTTP/1.1\r\n\r\n";
    ssize_t write_length = write(fd, request, sizeof(request));
    assertFalse(write_length < 0, "Error writing: (%d) %s", errno, strerror(errno));


    struct truth_s {
        char *line;
    } truth[] = {
        { .line = "HTTP/1.1 200 OK\r\n"                             },
        { .line = ""                                                }, // do not care line for Date
        { .line = "Content-Length: 18\r\n"                          },
        { .line = "Content-Type: text/html; charset=ISO-8859-1\r\n" },
        { .line = "\r\n"                                            },
        { .line = "Requested: /hello\n"                             },
        { .line = NULL                                              }
    };

    // read response line by line
    FILE *fh = fdopen(fd, "r");
    int count = 0;
    while (!feof(fh) && truth[count].line != NULL) {
        assertNotNull(truth[count].line, "read too many lines: %d", count);

        char response[16384];
        fgets(response, sizeof(response), fh);
        if (truth[count].line[0] != '\0') {
            bool result = strcmp(truth[count].line, response) == 0;

            if (!result) {
                // we need to cleanup the server or the next test will fail
                rtaFramework_Shutdown(framework, fds[1]);
                rtaFramework_Destroy(&framework);
                close(fds[0]);
                close(fds[1]);
                unblockSigChild();
                assertTrue(result, "mismatched lines, expected '%s' got '%s'", truth[count].line, response);
            }
        }
        count++;
    }
    fclose(fh);

    rtaFramework_Shutdown(framework, fds[1]);
    rtaFramework_Destroy(&framework);
    close(fds[0]);
    close(fds[1]);

    unblockSigChild();
}

LONGBOW_TEST_CASE(Local, rtaWebService_ProcessRequest)
{
#ifndef __APPLE__
    testSkip("Test broken on non-darwin");
#endif

    blockSigChild();
    int fds[2];
    int failure = socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    assertFalse(failure, "error on socketpair: (%d) %s", errno, strerror(errno));

    RtaFramework *framework = rtaFramework_Create(fds[0]);
    rtaFramework_Start(framework);
    rtaFramework_WaitForStatus(framework, FRAMEWORK_RUNNING);

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(9090);

    failure = connect(fd, (struct sockaddr *) &sin, sizeof(sin));
    assertFalse(failure, "error on connect: (%d) %s", errno, strerror(errno));

    char request[] = "GET /foo HTTP/1.1\r\n\r\n";
    write(fd, request, sizeof(request));

    struct truth_s {
        char *line;
    } truth[] = {
        { .line = "HTTP/1.1 404 Document was not found\r\n"     },
        { .line = "Content-Type: text/html\r\n"                 },
        { .line = "Connection: close\r\n"                       },
        { .line = ""                                            }, // do not care line for Date
        { .line = "Content-Length: 116\r\n"                     },
        { .line = "\r\n"                                        },
        { .line = "<HTML><HEAD>\n"                              },
        { .line = "<TITLE>404 Document was not found</TITLE>\n" },
        { .line = "</HEAD><BODY>\n"                             },
        { .line = "<H1>Document was not found</H1>\n"           },
        { .line = "</BODY></HTML>\n"                            },
        { .line = NULL                                          }
    };

    // read response line by line
    FILE *fh = fdopen(fd, "r");
    int count = 0;
    while (!feof(fh) && truth[count].line != NULL) {
        assertNotNull(truth[count].line, "read too many lines: %d", count);

        char response[16384];
        fgets(response, sizeof(response), fh);
        if (truth[count].line[0] != '\0') {
            assertTrue(strcmp(truth[count].line, response) == 0, "mismatched lines, expected '%s' got '%s'", truth[count].line, response);
        }
        count++;
    }
    fclose(fh);

    rtaFramework_Shutdown(framework, fds[1]);
    rtaFramework_Destroy(&framework);
    close(fds[0]);
    close(fds[1]);
    unblockSigChild();
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(rta_WebService);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
