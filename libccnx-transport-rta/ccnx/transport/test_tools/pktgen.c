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

/**
 * Generate packets
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <LongBow/runtime.h>

typedef enum {
    MODE_SEND,
    MODE_REPLY
} PktGenMode;

typedef enum {
    ENCAP_ETHER,
    ENCAP_UDP
} PktGenEncap;

typedef enum {
    PKTGEN_STREAM,
    PKTGEN_STOPWAIT
} PktGenFlow;

typedef struct {
    PktGenMode mode;
    PktGenEncap encap;
    PktGenFlow flow;

    char *ifname;
    char *etherOrIp;
    char *etherType;
    unsigned count;

    struct timeval startTime;
    struct timeval stopTime;
    unsigned packetCount;
} PktGen;

// ======================================================================

static void
usage(void)
{
    printf("usage: \n");
    printf("  This program functions as a requester and a responder.  They operate in a pair.\n");
    printf("  The test can run over raw Ethernet encapsulation or over UDP\n");
    printf("  The <count> parameter can be an integer or use a 'kmg' suffix for 1000, 1E+6, or 1E+9\n");
    printf("\n");
    printf("  pktgen send  ether <ifname> <dstmac> [ethertype] count <n> (stream | stopwait)\n");
    printf("  pktgen reply ether <ifname> [count <n>]\n");
    printf("\n");
    printf("  This mode sends either a stream or stop-and-wait request to an Ethernet peer\n");
    printf("  pktgen send  udp <ifname> <dstip> <dstport> count <n> (stream | stopwait)\n");
    printf("  pktgen reply udp <ifname> [count <n>]\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    This uses the standard Ethertype of 0x0801.  The replier will stay running forever.\n");
    printf("       pktgen send  ether em1 bc:30:5b:f2:2f:60 count 1M stream\n");
    printf("       pktgen reply ether em1\n");
    printf("\n");
    printf("    This uses a custom ethertype.  The replier will stay running forever.\n");
    printf("       pktgen send  ether em1 bc:30:5b:f2:2f:60 0x9000 count 1M stream\n");
    printf("       pktgen reply ether em1\n");
    printf("\n");
    printf("    An example with UDP\n");
    printf("       pktgen send  udp em1 10.1.0.2 9695 count 1M stopwait\n");
    printf("       pktgen reply udp em1\n");
    printf("\n");
}

static PktGen
parseCommandLine(int argc, char *argv[argc])
{
    PktGen pktgen;
    memset(&pktgen, 0, sizeof(PktGen));

    usage();

    return pktgen;
}

// ======================================================================

static void
generateEther(PktGen *pktgen)
{
    printf("Generating %u ethernet interest messages\n", pktgen->count);
}

static void
replyEther(PktGen *pktgen)
{
    printf("replying up to %u ethernet content objects messages\n", pktgen->count);
}

// ======================================================================

static void
generateUdp(PktGen *pktgen)
{
    printf("Generating %u UDP interest messages\n", pktgen->count);
}

static void
replyUdp(PktGen *pktgen)
{
    printf("replying up to %u UDP content objects messages\n", pktgen->count);
}


// ======================================================================

static void
displayStatistics(PktGen *pktgen)
{
    printf("stats.... coming soon\n");
}

// ======================================================================

static void
runSender(PktGen *pktgen)
{
    switch (pktgen->encap) {
        case ENCAP_ETHER:
            generateEther(pktgen);
            break;

        case ENCAP_UDP:
            generateUdp(pktgen);
            break;

        default:
            trapIllegalValue(pktgen.encap, "Unknown encapsulation: %d", pktgen->encap);
    }
}

static void
runReplier(PktGen *pktgen)
{
    switch (pktgen->encap) {
        case ENCAP_ETHER:
            replyEther(pktgen);
            break;

        case ENCAP_UDP:
            replyUdp(pktgen);
            break;

        default:
            trapIllegalValue(pktgen.encap, "Unknown encapsulation: %d", pktgen->encap);
    }
}

// ======================================================================

int
main(int argc, char *argv[argc])
{
    PktGen pktgen = parseCommandLine(argc, argv);

    switch (pktgen.mode) {
        case MODE_SEND:
            runSender(&pktgen);
            break;

        case MODE_REPLY:
            runReplier(&pktgen);
            break;

        default:
            trapIllegalValue(pktgen.mode, "Unknown mode: %d", pktgen.mode);
    }

    displayStatistics(&pktgen);
}
