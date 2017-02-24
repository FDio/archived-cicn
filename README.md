= vICN

== Description

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


== Dependencies

vICN requires a version of Python >= 3.5, and has the following dependendies.
We refer to them using the name of debian/ubuntu packages:

python3 (>= 3.5)
python3-sympy
python3-requests
python3-websockets
python3-networkx
python3-pyparsing
python3-autobahn
python3-pylxd (>=2.2.2, use pip3 if necessary, depends on libssl-dev)
  python3-pbr
  python3-requests-unixsocket


== Getting started

You can have a look at the tutorials available in the fd.io wiki:

Gettting started:
  https://wiki.fd.io/view/Vicn

Setup a Dumbbell topology using vICN:
  https://wiki.fd.io/view/Setup_a_Dumbbell_topology_using_vICN
