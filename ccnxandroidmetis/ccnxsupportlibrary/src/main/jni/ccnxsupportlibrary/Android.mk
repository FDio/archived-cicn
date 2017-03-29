##############################################################################
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
##############################################################################

LOCAL_PATH := $(call my-dir)

ARMDIST_ROOT := $(DISTILLERY_ROOT_DIR)

ARMDIST := $(ARMDIST_ROOT)/usr
ARMDEPS := $(ARMDIST_ROOT)/usr
METIS_INC := $(ARMDIST_ROOT)/usr/include

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow-ansiterm
LOCAL_SRC_FILES := $(ARMDIST)/lib/liblongbow-ansiterm.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow-textplain
LOCAL_SRC_FILES := $(ARMDIST)/lib/liblongbow-textplain.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libparc
LOCAL_SRC_FILES := $(ARMDIST)/lib/libparc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libmetis
LOCAL_SRC_FILES := $(ARMDIST)/lib/libmetis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_control
LOCAL_SRC_FILES := $(ARMDIST)/lib/libccnx_api_control.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_notify
LOCAL_SRC_FILES := $(ARMDIST)/lib/libccnx_api_notify.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_portal
LOCAL_SRC_FILES := $(ARMDIST)/lib/libccnx_api_portal.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_transport_rta
LOCAL_SRC_FILES := $(ARMDIST)/lib/libccnx_transport_rta.a
include $(PREBUILT_STATIC_LIBRARY)
include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_common
LOCAL_SRC_FILES := $(ARMDIST)/lib/libccnx_common.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libevent
LOCAL_SRC_FILES := $(ARMDEPS)/lib/libevent.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libssl
LOCAL_SRC_FILES := $(ARMDEPS)/lib/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libcrypto
LOCAL_SRC_FILES := $(ARMDEPS)/lib/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow
LOCAL_SRC_FILES := $(ARMDIST)/lib/liblongbow.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE        := ccnxsupportlibrary


LOCAL_SRC_FILES     := \
	Metis_wrap.c

LOCAL_CFLAGS		:= $(M_CFLAGS) $(OS_CFLAGS) -I$(ARMDIST)/include -I$(METIS_INC)

LOCAL_CFLAGS		+= -I$(ARMDIST)/include -I$(METIS_INC)
LOCAL_CFLAGS        += -std=c99 -g


LOCAL_LDLIBS        := -ldl -llog $(OS_LDFLAGS)

LOCAL_STATIC_LIBRARIES := \
    libmetis \
    libccnx_api_portal \
    libccnx_api_control \
    libccnx_api_notify \
    libccnx_transport_rta \
    libccnx_common \
    libparc \
	liblongbow \
    liblongbow-textplain \
    liblongbow-ansiterm \
	libssl \
	libevent \
	libcrypto

include $(BUILD_SHARED_LIBRARY)