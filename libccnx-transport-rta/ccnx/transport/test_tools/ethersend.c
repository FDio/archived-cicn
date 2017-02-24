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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <netinet/in.h>

typedef uint8_t u_char;
typedef uint16_t u_short;
typedef uint32_t u_int;


#include <pcap.h>

#define ETHERTYPE 0x0801

struct _packet {
    u_int8_t dst[6];
    u_int8_t src[6];
    u_int16_t ethertype;
    u_int8_t data[0];
} __attribute__((packed));


static void send_file(pcap_t *handle, uint8_t smac[], uint8_t dmac[], const char *filename);


static
void
printhex(u_int8_t *buffer, int length)
{
    int i;
    for (i = 0; i < length; i++) {
        printf("%02X", buffer[i]);
    }
}



static void
send_file(pcap_t *handle, uint8_t smac[], uint8_t dmac[], const char *filename)
{
    struct stat statbuf;

    struct _packet *packet = malloc(1500);
    memset(packet, 0, 1500);

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file: ");
        abort();
    }

    if (fstat(fd, &statbuf) < 0) {
        perror("fstat error");
        abort();
    }

    uint8_t *src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (src == MAP_FAILED) {
        perror("mmap error");
        abort();
    }

    assert(statbuf.st_size <= 1500);

    size_t len = sizeof(struct _packet) + statbuf.st_size;
    printf("Sending config/query size %zu\n", len);

    memcpy(packet->dst, dmac, 6);
    memcpy(packet->src, smac, 6);

    packet->ethertype = htons(ETHERTYPE);

    memcpy(packet->data, src, statbuf.st_size);

    int x = pcap_inject(handle, packet, len);
    printf("%s wrote %d bytes\n", __func__, x);

    free(packet);
}

static
void
get_mac_address(const char *deviceName, u_int8_t *mac)
{
    struct ifaddrs *ifap;
    struct ifaddrs *next;

    int x = getifaddrs(&ifap);
    if (x != 0) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    next = ifap;
    while (next != NULL) {
#if defined(__APPLE__)
        if (strstr(deviceName, next->ifa_name) != NULL && next->ifa_addr->sa_family == AF_LINK)
#elif defined(__linux__)
        if (strstr(deviceName, next->ifa_name) != NULL && next->ifa_addr->sa_family == AF_PACKET)
#else
#error Unsupported platform
#endif
        {
            memcpy(mac, next->ifa_addr->sa_data + 9, 6);
            break;
        }
        next = next->ifa_next;
    }
    freeifaddrs(ifap);
}

static void
macStringToArray(const char *string, size_t outputLength, uint8_t output[])
{
    assert(outputLength == 6);

    sscanf(string, "%02x:%02x:%02x:%02x:%02x:%02x", &output[0], &output[1], &output[2], &output[3], &output[4], &output[5]);
}

int
main(int argc, const char *argv[])
{
    if (argc != 4 || argv[1][0] == '-') {
        printf("usage: ethersend dev dst filename\n");
        printf("\n");
        printf("Will send filename as the payload of an ethernet frame to dst\n");
        printf("\n");
        printf("example: ethersend eth0 a8:20:66:3b:30:bc interest.bin\n");
        printf("\n");
        exit(EXIT_FAILURE);
    }

    pcap_t *handle;         /* Session handle */
    const char *dev = argv[1];          /* The device to sniff on */
    char errbuf[PCAP_ERRBUF_SIZE];  /* Error string */
    struct bpf_program fp;      /* The compiled filter */
    char filter_exp[1024];  /* The filter expression */
    bpf_u_int32 mask;       /* Our netmask */
    bpf_u_int32 net;        /* Our IP */

    sprintf(filter_exp, "ether proto 0x%04X", ETHERTYPE);

    printf("dev = %s\n", dev);

    /* Find the properties for the device */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }

    u_int8_t mymac[6];
    get_mac_address(dev, mymac);
    printf("My mac address: "); printhex(mymac, 6); printf("\n");

    u_int8_t dmac[6];
    macStringToArray(argv[2], 6, dmac);
    printf("dmac address  : "); printhex(dmac, 6); printf("\n");


    /* Open the session in promiscuous mode */
    handle = pcap_open_live(dev, 1500, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return (2);
    }

    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return (2);
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        return (2);
    }

    send_file(handle, mymac, dmac, argv[3]);


    pcap_close(handle);
    return (0);
}

