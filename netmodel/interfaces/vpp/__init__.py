#!/usr/bin/env python3

# Dependency: vpp-api-python

import asyncio
import asyncio.subprocess
import collections
import copy
import logging
import pyparsing as pp
import socket
import time

from netmodel.model.attribute           import Attribute
from netmodel.model.filter              import Filter
from netmodel.model.query               import Query, ACTION2STR, ACTION_UPDATE
from netmodel.model.query               import ACTION_SUBSCRIBE, ACTION_UNSUBSCRIBE
from netmodel.model.object              import Object
from netmodel.model.type                import Double, String
from netmodel.network.interface         import Interface as BaseInterface
from netmodel.network.packet            import Packet

log = logging.getLogger(__name__)

DEFAULT_INTERVAL = 1 # s
KEY_FIELD = 'device_name'

def parse(s):
    kw_name = pp.Keyword('Name')
    kw_idx = pp.Keyword('Idx')
    kw_state = pp.Keyword('State')
    kw_counter = pp.Keyword('Counter')
    kw_count = pp.Keyword('Count')

    kw_up = pp.CaselessKeyword('up')
    kw_down = pp.CaselessKeyword('down')
    kw_rx_packets = pp.CaselessKeyword('rx packets')
    kw_rx_bytes = pp.CaselessKeyword('rx bytes')
    kw_tx_packets = pp.CaselessKeyword('tx packets')
    kw_tx_bytes = pp.CaselessKeyword('tx bytes')
    kw_drops = pp.CaselessKeyword('drops')
    kw_ip4 = pp.CaselessKeyword('ip4')
    kw_ip6 = pp.CaselessKeyword('ip6')
    kw_tx_error = pp.CaselessKeyword('tx-error')
    kw_rx_miss = pp.CaselessKeyword('rx-miss')

    header = kw_name + kw_idx + kw_state + kw_counter + kw_count

    interface = (pp.Word(pp.alphanums + '/' + '-').setResultsName('device_name') + \
             pp.Word(pp.nums).setResultsName('index') + \
             pp.oneOf(['up', 'down']).setResultsName('state') + \
             pp.Optional(kw_rx_packets + pp.Word(pp.nums).setResultsName('rx_packets')) + \
             pp.Optional(kw_rx_bytes + pp.Word(pp.nums).setResultsName('rx_bytes')) + \
             pp.Optional(kw_tx_packets + pp.Word(pp.nums).setResultsName('tx_packets')) + \
             pp.Optional(kw_tx_bytes + pp.Word(pp.nums).setResultsName('tx_bytes')) + \
             pp.Optional(kw_drops + pp.Word(pp.nums).setResultsName('drops')) + \
             pp.Optional(kw_ip4 + pp.Word(pp.nums).setResultsName('ip4')) + \
             pp.Optional(kw_ip6 + pp.Word(pp.nums).setResultsName('ip6')) + \
             pp.Optional(kw_rx_miss + pp.Word(pp.nums).setResultsName('rx_miss')) + \
             pp.Optional(kw_tx_error + pp.Word(pp.nums).setResultsName('tx_error'))
    ).setParseAction(lambda t: t.asDict())

    bnf = (
            header.suppress() +
            pp.OneOrMore(interface)
    ).setParseAction(lambda t: t.asList())

    return bnf.parseString(s, parseAll = True).asList()

class VPPInterface(Object):
    __type__ = 'vpp_interface'

    node = Attribute(String)
    device_name = Attribute(String)
    bw_upstream = Attribute(Double) # bytes
    bw_downstream = Attribute(Double) # bytes

class VPPCtlInterface(BaseInterface):
    __interface__ = 'vppctl'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.register_object(VPPInterface)

        # Set of monitored interfaces
        self._interfaces = collections.defaultdict(int)
        self._running = False

        # interface -> (time, rx, tx)
        self._last = dict()

    async def _tick(self):
        while self._running:
            try:
                create = asyncio.create_subprocess_exec(
                    'vppctl_wrapper', 'show', 'int',
                    stdout=asyncio.subprocess.PIPE,
                )
                proc = await create
                await proc.wait()
                stdout = await proc.stdout.read()

                if proc.returncode:
                    log.error("error")
                    return

                interfaces = parse(stdout.decode())
                last = copy.copy(self._last)
                self._last = dict()
                now = time.time()
                for interface in interfaces:
                    if not interface['device_name'] in self._interfaces:
                        continue
                    tx = float(interface['tx_bytes'])
                    rx = float(interface['rx_bytes'])
                    self._last[interface['device_name']] = (now, rx, tx)

                    if not interface['device_name'] in last:
                        continue
                    prev_now, prev_rx, prev_tx = last[interface['device_name']]

                    # Per interface throughput computation
                    ret = {
                        'node'          : socket.gethostname(),
                        'device_name'   : interface['device_name'],
                        'bw_upstream'   : (tx - prev_tx) / (now - prev_now),
                        'bw_downstream' : (rx - prev_rx) / (now - prev_now),
                    }

                    f_list = [[KEY_FIELD, '==', interface['device_name']]]
                    del interface['device_name']
                    query = Query(ACTION_UPDATE, VPPInterface.__type__,
                            filter = Filter.from_list(f_list),
                            params = ret)
                    self.receive(Packet.from_query(query, reply = True))
            except Exception as e:
                import traceback; traceback.print_exc()
                log.error("Could not perform measurement {}".format(e))

            await asyncio.sleep(DEFAULT_INTERVAL)

    #--------------------------------------------------------------------------
    # Router interface
    #--------------------------------------------------------------------------

    def send_impl(self, packet):
        query = packet.to_query()

        if query.action not in (ACTION_SUBSCRIBE, ACTION_UNSUBSCRIBE):
            log.warning("Ignore unknown action {}".format(
                        ACTION2STR[query.action]))
            return

        # We currently simply extract it from the filter
        interfaces = set([p.value for p in query.filter if p.key == KEY_FIELD])

        for interface in interfaces:
            if query.action == ACTION_SUBSCRIBE:
                self._interfaces[interface] += 1
            else:
                self._interfaces[interface] -= 1

        all_interfaces = set([k for k, v in self._interfaces.items() if v > 0])

        if all_interfaces and not self._running:
            self._running = True
            asyncio.ensure_future(self._tick())
        elif not all_interfaces and self._running:
            self._running = False


#-------------------------------------------------------------------------------

if __name__ == '__main__':
    x="""              Name               Idx       State          Counter          Count
    TenGigabitEthernetc/0/1           1         up       rx packets               3511586
                                                         rx bytes              4785592030
                                                         tx packets               3511678
                                                         tx bytes               313021701
                                                         drops                          7
                                                         ip4                       161538
                                                         ip6                      3350047
                                                         tx-error                       2
    host-bh1                          4         up       rx packets                     5
                                                         rx bytes                     394
                                                         tx packets                    10
                                                         tx bytes                     860
                                                         drops                          4
                                                         ip6                            4
    host-bh2                          6         up       rx packets               3164301
                                                         rx bytes               287315869
                                                         tx packets               3164238
                                                         tx bytes              4290944332
                                                         drops                          4
                                                         ip4                       161539
                                                         ip6                      3002759
    host-bh3                          7         up       rx packets                 33066
                                                         rx bytes                 2446928
                                                         tx packets                 33060
                                                         tx bytes                47058708
                                                         drops                          5
                                                         ip6                        33065
    host-bh4                          5         up       rx packets                114407
                                                         rx bytes                 8466166
                                                         tx packets                114412
                                                         tx bytes               162905294
                                                         drops                          7
                                                         ip6                       114406
    host-bh5                          3         up       rx packets                150574
                                                         rx bytes                11142524
                                                         tx packets                150578
                                                         tx bytes               214407016
                                                         drops                          7
                                                         ip6                       150573
    host-bh6                          2         up       rx packets                 49380
                                                         rx bytes                 3654160
                                                         tx packets                 49368
                                                         tx bytes                70283976
                                                         drops                          9
                                                         ip6                        49377
    local0                            0        down      drops                          3
    """

    r = parse(x)
    print(r)
