# patches to ns3 wifi module

## General description of patch to ns3 wifi

For the wifi emulator, we use ns3 based emulation to create a emulated 802.11n wifi channel. We choose 802.11n to have high throughput and to incorporate some of the latest features of 802.11 wifi technology.

However, the codes for wifi 802.11n model in ns3 is not bug free up even in the latest ns3.25 release. We integrated some bug fixes for 802.11n wifi model from both ns3 dev branch and our own bug fixes. 

The fixes/patches are intended for issues in 802.11n wifi model of ns3 including:
minstrel rate adaptation, RSSI based wifi handover(missing), block ack agreement destroyment and reinitialization in scenario with wifi handover and various exceptions handling when stations are mobile and makes frequent handovers.

## Notes

* Our patches are with respect to ns3.24.1 official release(can be downloaded here: https://www.nsnam.org/ns-3-24/download/)
* all the patched file should be in ns3/src/wifi/ folder of ns3 codebase
* We may port the patch to the latest ns3 release in the future.

## instruction to apply the patch
you just need to replace the original files in ns3/src/wifi with the ones that are patched, to do so:
* cd ns3-patch
* cp -r wifi/ path/to/ns3/src/wifi

