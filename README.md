vICN
=========

Description
-------------

This application is an ICN experimental testbed orchestrator. It allows to
quickly deploy experiments in a cluster using linux containers, IP tunnels and
linux kernel tools.

This tool provides the following features:

- Physical topology deployment
- Virtual topology deployment
- ICN Network setup
- Management tools

The architecture of this orchestrator is basically a master-slave architecture.
The master, vICN, controls the whole cluster of servers, by means of HTTPS REST
APIs and remote SSH.


Dependencies
-------------

vICN requires a version of Python >= 3.5, and has the following dependendies.
We refer to them using the name of debian/ubuntu packages:

- python3 (>= 3.5)
- libssl-dev
- python3-pip
- python3-daemon

Installation on Ubuntu
-------------

Download vICN by cloning the git repositoy.

    git clone -b vicn/master https://gerrit.fd.io/r/cicn vicn

If you plan to use vICN with Linux Containers, get the latest version of lxd from the LXD ppa:

   sudo add-apt-repository ppa:ubuntu-lxc/lxd-stable
   sudo apt-get update

Install the dependencies. Beware to install the daemon module of python through repositories as the pip version is broken.

    sudo apt-get install python3-pip libssl-dev python3-daemon

You can now install vICN:

    cd vicn
    sudo ./setup.py install

Getting started
--------------

You can have a look at the tutorials available in the fd.io wiki:

Gettting started:
  https://wiki.fd.io/view/Vicn

Setup a Dumbbell topology using vICN:
  https://wiki.fd.io/view/Setup_a_Dumbbell_topology_using_vICN
