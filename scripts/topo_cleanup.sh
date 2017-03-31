#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "usage: ./topo_cleanup.sh [topology_file.json]"
	exit 1
fi

TOPOLOGY=$1

function get_by_type()
{
	echo $(python3 -c "import json, pprint; j = json.loads(open('$1').read()); print(' '.join(r['name'] for r in j['resources'] if r['type'] == '$2'))")
}

#grep configuration from JSON
SERVERS=$(get_by_type $TOPOLOGY LxcContainer)
NETWORK=$(sed -n '/network/{s/.*"\([^"]*\)"$/\1/;p}' $TOPOLOGY)

# delete spurious interfaces on host
route -n | awk '{print $8}' | tail -n -2 | grep vh | sort | uniq | xargs

# Remove containers
for server in $SERVERS; do
	(lxc stop $server; lxc delete $server) &
done
wait

#Kill the emulators
killall -9 wifi_emulator
killall -9 lte_emulator
kill -9 $(ps aux | awk  '/\/usr\/bin\/python3 \/usr\/bin\/netmon/ {print $2}')
service netmon stop

# Clean ports on OVS
for x in $(ovs-vsctl show | sed -n '/Bridge "br0"/,/Bridge/p' | grep "No such device" | sed "s/.*device \(.*\) (No such device).*/\1/"  ); do echo $x; ovs-vsctl del-port br0 $x; done

for i in $(ip link | grep unk | cut -d: -f 2 | cut -d @ -f 1); do ip link delete $i; done
for i in $(ip link | grep tmp-veth | cut -d: -f 2 | cut -d @ -f 1); do ip link delete $i; done
for i in $(ip link | grep tap- | cut -d: -f 2 | cut -d @ -f 1); do ip link delete $i; done
for i in $(ip link | grep mv- | cut -d: -f 2 | cut -d @ -f 1); do ip link delete $i; done

# Remove bridge
echo "Removing bridge..."
ovs-vsctl --if-exists del-br br0

# Remove taps
echo "Removing interface..."
for i in $(ip link show | egrep "(tap|macvlan|macvtap)" | cut -d : -f 2 | cut -d @ -f 1); do echo " - delete $i"; ip link delete $i; done

#Remove routes
echo "Removing stale routes"
NETWORK=$(echo $NETWORK | cut -d'/' -f1 | rev | cut -d"." -f2- | rev | sed "s/\./\\\\./g")
eval $(ip route show | sed -n '/$NETWORK\./ {s/^\(.*\) dev \(.*\)  scope link.*$/route del \1 dev \2;/;p}')
