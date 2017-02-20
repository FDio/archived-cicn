FD.IO CICN project: VPP plugin
==============================

The CICN forwarder

## Quick Start ##
```
From the code tree root

Using automake 
$ cd cicn-plugin 
$ autoreconf -i -f
$ mkdir -p build 
$ cd build
$ ../configure --with-plugin-toolkit
OR, to omit UT code
$ ../configure --with-plugin-toolkit  --without-cicn-test
$ make
$ sudo make install

Using cmake
$ cd cicn-plugin
$ mkdir -p build
$ cd build
$ cmake ..
Or, to omit UT code
$ cmake .. -DNO_UNIT_TEST=TRUE
$ make
$ sudo make install
```

## Introduction ##

The high-performance CCNx ICN forwarder as a plugin to VPP.

The plugin provides the following functionalities:
 
 - Fast packet processing
 - Interest aggregation
 - Content caching

## Using CICN plugin ##

### Platforms ###

CICN has been tested in:

- Ubuntu 16.04 LTS (x86_64)
- Ubuntu 14.04 LTS (x86_64)
- Debian Stable/Testing (2017-03-01)
- Red Hat Enterprise Linux 7
- CentOS 7


### Dependencies ###

Build dependencies:

- VPP 17.01

Hardware dependencies:

- [DPDK](http://DPDK.org/) compatible nic 

### Getting started ###
In order to start, the CICN plugin requires a running instance of VPP and at least one DPDK compatible nic. The steps required to successfully start CICN are:

- Setup the host to run VPP
- Configure VPP to use DPDK compatible niCS
- Start VPP
- Configure VPP interfaces
- Configure and start CICN

Detailed information for configuring VPP can be found at [https://wiki.fd.io/view/VPP](https://wiki.fd.io/view/VPP).

##### Setup the host for VPP #####

VPP requires the `uio` and `igb_uio` modules to be loaded in the kernel

```
$ sudo modprobe uio
$ sudo modprobe igb_uio
```
Hugepages must be enabled in the system

```
$ sudo sysctl -w vm.nr_hugepages=1024
```

If the DPDK interface we want to assign to VPP is up, we must bring it down 

```
$ sudo ifconfig <interface_name> down
```

##### Configure VPP #####
The file /etc/VPP/startup.conf contains a set of parameters to setup VPP at startup. The following example sets up VPP to use a DPDK interfaces:

``` shell
unix {
  nodaemon
  log /tmp/vpp.log
  full-coredump
}

api-trace {
  on
}

api-segment {
  gid vpp
}

DPDK {
  socket-mem 1024
  dev 0000:08:00.0
}
```
Where `0000:08:00.0` must be replaced with the actual PCI address of the DPDK interface

##### Start VPP #####

VPP can be started as a process or a service:

``` shell
Start VPP as a service in Ubuntu 16.04
$ sudo systemctl start vpp

Start VPP as a service in Ubuntu 14.04
$ sudo service vpp start

Start VPP as a process in both 16.04 and 14.04
$ sudo vpp -c /etc/vpp/startup.conf

```

##### Configure VPP interfaces #####

ICN communications run on top of IP and TCP, therefore we need to assign an IP address to the DPDK interface that enables IP connectivity through that interface

``` shell
Set an IP address on the DPDK interface
$ sudo vppctl set int ip address GigabitEthernet0/8/0 10.0.0.1/24

Bring the interface up
$ sudo vppctl set int state GigabitEthernet0/8/0 up
```
`GigabitEthernet0/8/0` is the name that VPP assigned to the DPDK interface. It must be replaced with the actual name of the DPDK interface ([`sudo vppctl show interfaces`](https://doCS.fd.io/vpp/17.01/clicmd_vnet_vnet.html#clicmd_show_interfaces) shows the available interfaces in VPP). 


##### Configure and start CICN #####

The following three commands enable CICN, set a face pointing to a remote host and add an entry in the FIB

``` shell
Start CICN plugin
$ sudo vppctl cicn enable

Create a face for the DPDK interface
$ sudo vppctl cicn cfg face add local 10.0.0.1:33302 remote 10.0.0.2:33302
Face ID: 1

Add a FIB entry
$ sudo vppctl cicn cfg fib add prefix /cicn face 1
```

- `10.0.0.1:33302` must be replaced with the IP address assigned to the DPDK interface in the previous step. The tcp port can be chosen as desired.
- `10.0.0.2:33302` must be replaced with the IP address assigned to the remote host we want to connect to
- `/cicn` must be replaced with the ICN prefix to forward through face 1

### CICN commands ###
A full list of the available commands for CICN is available through:

- `sudo vppctl cicn help`

If `help` is passed as a paramenter to a CICN command, the output will display information about the syntax of the command

##### CICN statistiCS ####

A fine grained set of statistic for the ongoing communication is available through:

- `sudo vppctl cicn show`

##### Setting PIT, FIB and CS parameters ####

If needed, it is possible to change the default values for the three CICN internal structures. The customizable parameters and the corresponding commands are:

- PIT size
	- `sudo vppctl cicn control param pit size <# of entries>`
- PIT entry lifetime (default, min and max)
	- `sudo vppctl cicn control param pit {dfltlife | minlife | maxlife} <seconds>`
- FIB size
	- `sudo vppctl cicn control param fib size <# of entries>`
- CS size
	- `sudo vppctl cicn control param cs size <# of entries>`

Changes on PIT, FIB and CS can be made only before starting CICN.


### License ###

This software is distributed under the following license:

```
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
```


