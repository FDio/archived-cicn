##### VPP SETUP #####

CMD_VPP_STOP_SERVICE = 'systemctl stop vpp.service'
CMD_VPP_DISABLE = 'systemctl disable vpp.service'

# 'sleep 1' ensures that VPP has enough time to start
CMD_VPP_START = '''
systemctl start vpp
sleep 1
'''
CMD_VPP_STOP = '''
systemctl stop vpp
killall -9 vpp_main || true
'''
CMD_VPP_ENABLE_PLUGIN = 'vppctl {plugin} enable'

##### VPP INTERFACES #####

CMD_VPP_CREATE_IFACE = '''
# Create vpp interface from netmodel.network.interface.device_name} with mac {self.parent.mac_address}
vppctl create host-interface name {vpp_interface.parent.device_name} hw-addr {vpp_interface.parent.mac_address}
vppctl set interface state {vpp_interface.device_name} up
'''
CMD_VPP_SET_IP = 'vppctl set int ip address {netdevice.device_name} {netdevice.ip_address}/{netdevice.prefix_len}'
CMD_VPP_SET_UP = 'vppctl set int state {netdevice.device_name} up'

##### VPP IP ROUTING #####

CMD_VPP_ADD_ROUTE = 'vppctl set ip arp static {route.interface.vppinterface.device_name} {route.ip_address} {route.mac_address}'
CMD_VPP_DEL_ROUTE = 'vppctl set ip arp del static {route.interface.vppinterface.device_name} {route.ip_address} {route.mac_address}'
CMD_VPP_ADD_ROUTE_GW = 'vppctl ip route add {route.ip_address}/32 via {route.gateway} {route.interface.vppinterface.device_name}'
CMD_VPP_DEL_ROUTE_GW = 'vppctl ip route del {route.ip_address}/32 via {route.gateway} {route.interface.vppinterface.device_name}'

##### VPP CICN PLUGIN #####

CMD_VPP_CICN_GET = "timeout 1 vppctl cicn show" #We timeout if vpp is not started
CMD_VPP_ADD_ICN_ROUTE = 'vppctl cicn cfg fib add prefix {route.prefix} face {route.face.id}'
CMD_VPP_ADD_ICN_FACE = 'vppctl cicn cfg face add local {face.src_ip}:{face.src_port} remote {face.dst_ip}:{face.dst_port}'

CMD_VPP_CICN_GET_CACHE_SIZE = 'vppctl cicn show | grep "CS entries" | grep -Eo "[0-9]+"'
CMD_VPP_CICN_SET_CACHE_SIZE = 'vppctl cicn control param cs size {self.cache_size}'
