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
 * Utility function to write the test data to data files.  The data files will be in a format
 * you can use to import with "text2pcap".  For example:
 *
 *     text2pcap -u 9695,9695 file
 *
 * would add a fake UPD/IP/Ethernet header with UDP ports 9695 for source and destination
 *
 */

#include <stdio.h>

#include <ccnx/common/codec/schema_v0/testdata/testrig_truthSet.h>
#include <ccnx/common/codec/schema_v1/testdata/v1_testrig_truthSet.h>

/*
 * typedef struct testrig_truth_table
 * {
 * const char *      testname;
 * uint8_t *         packet;
 * size_t            length;
 *
 * TlvErrorCodes     expectedError;
 *
 * // the array is terminated by a T_INVALID value
 * // for "arrayIndexOrTypeKey"
 * TruthTableEntry * entry;
 * } TruthTable;
 *
 */

static void
writePacket(TruthTable *table)
{
    char filename[1024];
    snprintf(filename, 1024, "%s.txt", table->testname);
    FILE *fh = fopen(filename, "w+");
    printf("name %s\n", filename);

    int linewidth = 8;
    for (int i = 0; i < table->length; i++) {
        if ((i % linewidth) == 0) {
            fprintf(fh, "\n%06X  ", i);
        }
        fprintf(fh, "%02X ", table->packet[i]);
    }
    fprintf(fh, "\n");
    fclose(fh);
}

static void
loopTruthTable(TruthTable truthset[])
{
    for (int i = 0; truthset[i].packet != NULL; i++) {
        writePacket(&truthset[i]);
    }
}

int
main(int argc, char **argv)
{
    loopTruthTable(interests_truthSet);
    loopTruthTable(contentObject_truthSet);
    loopTruthTable(cpi_truthSet);

    loopTruthTable(v1_interests_truthSet);
    loopTruthTable(v1_contentObject_truthSet);
    loopTruthTable(v1_cpi_truthSet);
}
