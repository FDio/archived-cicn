# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AUTOMAKE_OPTIONS = foreign subdir-objects

AM_CFLAGS = -Wall -Werror -march=native -I@TOOLKIT_INCLUDE@ \
    -DDPDK=1 \
    -I/usr/include/vpp-dpdk

lib_LTLIBRARIES = cicn_plugin.la cicn_api_test_plugin.la
cicn_plugin_oper_sources = \
	cicn/cicn.c \
	cicn/cicn_face.c \
	cicn/cicn_fib.c \
	cicn/cicn_hashtb.c \
	cicn/cicn_hello.c \
	cicn/cicn_mgmt.c \
	cicn/cicn_parser.c \
	cicn/cicn_pg.c \
	cicn/cicn_pcs.c \
	cicn/node.c

cicn_plugin_la_SOURCES = $(cicn_plugin_oper_sources)

# UT sources, optionally included in plugin for execution under "cicn test"
cicn_plugin_test_sources = \
	cicn/test/test_cicn.c \
	cicn/test/test_cicn_hash.c cicn/test/test_cicn_hash_cdata.c

# include/exclude cicn ut modules in plugin, compile modules regardless
if WITH_CICN_TEST
cicn_plugin_la_SOURCES += $(cicn_plugin_test_sources)
else
# libtest_cicn.la is never used, only built if tests omitted from cicn_plug_la
# (goal is to force compilation in all cases)
noinst_LTLIBRARIES = libtest_cicn.la
libtest_cicn_la_SOURCES = $(cicn_plugin_test_sources)
libtest_cicn_la_CPPFLAGS = $(cicn_plugin_la_CPPFLAGS)
endif

# CICN_VPP_PLUGIN is only set for vpp plugin, not api client library plugin
cicn_plugin_la_CPPFLAGS = \
	-DCICN_VPP_PLUGIN=1
cicn_plugin_la_LDFLAGS = -module

BUILT_SOURCES = cicn/cicn.api.h

SUFFIXES = .api.h .api

%.api.h: %.api
	mkdir -p `dirname $@` ; \
	$(CC) $(CPPFLAGS) -E -P -C -x c $^ \
	| vppapigen --input - --output $@ --show-name $@

nobase_include_HEADERS =			\
  cicn/cicn_all_api_h.h			\
  cicn/cicn_msg_enum.h			\
  cicn/cicn_api.h			\
  cicn/cicn.api.h

cicn_api_test_plugin_la_SOURCES = \
  cicn/cicn_api_test.c cicn/cicn_plugin.api.h cicn/cicn_api.h
cicn_api_test_plugin_la_CPPFLAGS = \
	-DCICN_VPP_CLIENTLIB=1
cicn_api_test_plugin_la_LDFLAGS = -module

install-data-hook:
	mkdir -p /usr/lib/vpp_plugins
	mkdir -p /usr/lib/vpp_api_test_plugins
	cp -p $(prefix)/lib/cicn_plugin.so.*.*.* \
		/usr/lib/vpp_plugins/cicn_plugin.so
	cp -p $(prefix)/lib/cicn_api_test_plugin.so.*.*.* \
		/usr/lib/vpp_api_test_plugins/cicn_api_test_plugin.so
	rm -f $(prefix)/lib/cicn_plugin.*
	rm -f $(prefix)/lib/cicn_api_test_plugin.*
