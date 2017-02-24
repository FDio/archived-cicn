# Wifi/LTE Emulator

## General description

**Wifi and LTE Emulator** are two applications able to emulate a wifi and a LTE connection between
two machines.

The script emulates a virtual scenario with many Stations/UE connected to one Access Point/enodeB.

In particular:

* The stations/UE are able to move around the access point/enodeB
* The access point/enodeB and the stations/UE are connected through Wi-Fi 802.11n/LTE...
* The wi-fi rate adaptation algorithm is Minstrel
* The 802.11 frames are sent enabling frame aggregation

## Dependencies

To be able to build/run the script the following libraries have to be installed. On Ubuntu:

*
    **libns3**

    If you are inside the cisco network you can directly install the library from our repository:

    ```bash
    # echo "deb http://pirl-ndn-1.cisco.com/packages/ $(grep -oP "UBUNTU_CODENAME=\K\w+" /etc/os-release)/" > /etc/apt/sources.list.d/pirl-ndn-repo.list
    # apt-get update
    # apt install ns3sx libns3sx-dev
    ```

    Otherwise you need to download and install the latest version of libns3 by downloading it from
    https://www.nsnam.org/releases/.

    A complete guide to do it can be find at the address
    https://www.nsnam.org/docs/tutorial/html/getting-started.html

    Then you need to apply the patches inside the ns3-patch folder:

    ```bash
    $ cp ns3-patch/tap-bridge.cc $PATH_TO_NS3_FOLDER/src/tap-bridge/model/tap-bridge.cc
    $ cp ns3-patch/epc-tft-classifier.cc $PATH_TO_NS3_FOLDER/src/lte/epc-tft-classifier.cc
    ```

    And then compile ns3 following its building guide.

* **libboost**

    ```bash
    $ sudo apt install libboost-all-dev
    ```

* **libwesocketpp**
    ```bash
    $ sudo apt install libwebsocketpp-dev
    ```

* **libsqlite3**

    ```bash
    $ sudo apt install libsqlite3-dev
    ```

* **libxml2**

    ```bash
    $ sudo apt install libxml2-dev
    ```

## Installation

Under the wifi-emulator/lte-emulator folder:

```bash
$ cmake . && make && sudo make install
```

## Getting started

A trivial scenario consists in 2 machines (AP/enodeB and Station/UE) connected together through this
script.
It requires:

* One virtual machine acting as AP/enodeB
* One virtual machine acting as Station/UE
* Two linux bridges
* Two tap devices

In this example the virtual machines are going to be linux containers, but it works with any virtual
machine.

Linux container reference: [LXD](https://linuxcontainers.org/lxd/getting-started-cli/)

The two linux containers are named *left* and *right*, and all the corresponding bridges/taps are
going to have a similar name.

1. Create the two bridges:

    ```bash
    # ip link add name br-left type bridge && ip link add name br-right type bridge
    ```

2. Create the two taps devices

    ```bash
    # ip tuntap add name tap-left mode tap && ip tuntap add name tap-right mode tap
    ```

3. Configure the taps

    ```bash
    # ip link set dev tap-left promisc on && ip link set dev tap-right promisc on
    # ip addr add dev tap-left 0.0.0.0 && ip addr add dev tap-right 0.0.0.0
    # ip link set tap-left up && ip link set tap-right up
    ```

4. Connect the taps to the corresponding bridges:

    ```bash
    # ip link set tap-left master br-left && ip link set tap-right master br-right
    ```

5. Bring up the bridges

    ```bash
    # ip link set br-left up && ip link set br-right up
    ```

6. Create the two linux containers, bridging them to the right bridge

    ```bash
    $ lxc launch ubuntu:xenial left -p && lxc config device add left eth0 nic nictype=bridged parent=br-left hwaddr=00:16:3e:00:00:01
    $ lxc launch ubuntu:xenial right -p && lxc config device add right eth0 nic nictype=bridged parent=br-right hwaddr=00:16:3e:00:00:02
    ```

7. Restart the containers

    ```bash
    $ lxc restart left && lxc restart right
    ```

8. Assign an IP address to each container

    ```bash
    $ lxc exec left -- ip addr add 192.168.1.1/24 brd + dev eth0 && lxc exec right -- ip addr add 192.168.1.2/24 brd + dev eth0
    ```

9. Run the emulator script. In this case the left container will be the access point/enodeB while
the right container will be the station/UE.

    802.11n:

    ```bash
    # wifi_emulator --bs-x=0.0 --bs-mac=00:16:3e:00:00:01 --bs-y=0.0 --n-sta=1 --sta-list=right --bs-name=left \
                    --experiment-id=exp1 --sta-taps=tap-right --bs-tap=tap-left --sta-macs=00:16:3e:00:00:02   \
                    --control-port=41010
    ```

    LTE

    ```bash
    # lte_emulator --isFading=true --distance=3500 --bs-x=0.0 --bs-mac=00:16:3e:00:00:01 --bs-y=0.0 --n-sta=1          \
                   --sta-list=right --bs-name=left  --experiment-id=exp1 --sta-taps=tap-right --bs-tap=tap-left        \
                   --sta-macs=00:16:3e:00:00:02   --control-port=41010 --sta-ips=192.168.1.2/24 --bs-ip=192.168.1.1/24 \
                   --printIP=true --logging=true --txBuffer=8000000
    ```

10. Test connectivity by a simple ping test

    ```bash
    $ lxc exec left -- ping 192.168.1.2
    ``

## Talking with the simulation

This script also allows to be controlled and to export internal statistics through a websocket
connection. So far the only available parameters are the mobility model/position of the stations/UE
inside the simulation and the physical tranmission rate.

For setting/retrieving one of these parameters a json object with the following format has to be
sent through the websocket connection:

{object_name: <interface>, fields: <x,y,rate>, params: <x=1,y=2..>, action: <select, update, subscribe..>, filter: [['x', '==', '21']], last: <true, false>}

This json object represents a SQL query/update. In particular:


* **object_name** is the name of the object to query/update. So far the only supported object is
**interface**.

* **fields** is a list representing the name of the field to select. The supported fields are the
**x and y** coordinates of the station/ue and the physical transmission rate.

* **params** is a dictionary containing the keys/values to update. The keys are the fields, while
the values are the new values to update.

* **action** is the action to perform. So far **update, select and subscribe** are supported.

* **filter** is a list containing the filters to select a particular object inside the simulation.
The only filter available is the **node_id**.
