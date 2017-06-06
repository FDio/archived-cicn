package com.metis.ccnx.utility;

/**
 * Created by angelomantellini on 18/05/2017.
 */

public class Constants {
    public static final String DEFAULT_NEXT_HOP_IP = "10.60.17.200";
    public static final String DEFAULT_NEXT_HOP_PORT = "11111";
    public static final String DEFAULT_PREFIX = "ccnx:/webserver";
    public static final String ENABLED = "Enabled";
    public static final String DISABLED = "Disabled";
    public static final String METIS_FORWARDER_PREFERENCES = "metisForwarderPreferences";
    public static final String DEFAULT_SOURCE_INTERFACE = "eth0";
    public static final String DEFAULT_SOURCE_PORT = "11111";
    public static final String DEFAULT_CONFIGURATION = "add listener tcp local0 127.0.0.1 9695\n" +
            "add listener udp remote0 %%source_ip%% %%source_port%%\n" +
            "add connection udp conn0 %%next_hop_ip%% %%next_hop_port%% %%source_ip%% %%source_port%%\n" +
            "add route conn0 %%prefix%% 1";



            //"add connection udp conn0 %%next_ip%% %%next_port_ip%% %%source_ip%% %%source_port%%\n" +
            //"add route conn0 %%prefix%%";
    public static final String SOURCE_IP = "%%source_ip%%";
    public static final String SOURCE_PORT = "%%source_port%%";
    public static final String NEXT_HOP_IP = "%%next_hop_ip%%";
    public static final String NEXT_HOP_PORT = "%%next_hop_port%%";
    public static final String PREFIX = "%%prefix%%";
    public static final String NETMASK = "%%netmask%%";
    public static final String CONFIGURATION_PATH = "Configuration";
    public static final String CONFIGURATION_FILE_NAME = "metis_forwarder.conf";
    public static final int FOREGROUND_SERVICE = 101;
}
