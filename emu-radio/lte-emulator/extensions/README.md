The extensions are basically either some rewriting or some inheriting
of existing classes in ns3 such that the emulation of lte channel in ns3
can be enabled.

some of the original lte helper and lte network device implementation
does not support the integration with tap devices defined in ns3. So some
critical extensions or modifications of those imlementations must be done
in order to enable lte channel emulation in ns3.

To avoid making changes directly on the ns3 codebase, which may lead to needs
of recompiling ns3 library and other packages depending on ns3, here all the
modifications or extensions are implemented by rewriting or inheriting of existing
ns3 class. These extensions shall be linked with lte emulator program and
ns3 library such that those critical modifications can take into play when running
lte emulator program.

Since all those modifications are for purpose of enabling integration lte network
with tap device in ns3, all the extension files/classes have "tap" in their namings.
