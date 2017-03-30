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

########################################
#
# Find the VPP libraries and includes
# This module sets:
#  VPP_DPDK_FOUND: True if VPP was found
#  VPP_DPDK_INCLUDE_DIR:  The VPP include dir
#

set(VPP_DPDK_SEARCH_PATH_LIST
    ${VPP_DPDK_ROOT}
    $ENV{VPP_DODK_ROOT}
    /usr/local
    /opt
    /usr)

find_path(VPP_DPDK_INCLUDE_DIR vpp-dpdk/rte_version.h
        HINTS ${VPP_DPDK_SEARCH_PATH_LIST}
        PATH_SUFFIXES include
        DOC "Find the VPP-DPDK includes")

set(VPP_DPDK_INCLUDE_DIRS ${VPP_DPDK_INCLUDE_DIR} ${VPP_DPDK_INCLUDE_DIR}/vpp-dpdk)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VppDpdk DEFAULT_MSG VPP_DPDK_INCLUDE_DIR)
