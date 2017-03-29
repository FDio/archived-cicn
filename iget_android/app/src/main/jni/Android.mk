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
LOCAL_PATH_SAVED := $(LOCAL_PATH)
ARMDIST_ROOT := $(DISTILLERY_ROOT_DIR)/usr

include $(CLEAR_VARS)
LOCAL_MODULE    := libboost_system
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libboost_system.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libevent
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libevent.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libcrypto
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libcrypto.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libssl
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libssl.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libcrystax
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libcrystax.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow-ansiterm
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/liblongbow-ansiterm.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow-textplain
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/liblongbow-textplain.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := liblongbow
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/liblongbow.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libparc
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libparc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_common
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libccnx_common.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_notify
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libccnx_api_notify.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_control
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libccnx_api_control.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_transport_rta
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libccnx_transport_rta.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libccnx_api_portal
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libccnx_api_portal.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libicnet
LOCAL_SRC_FILES := $(ARMDIST_ROOT)/lib/libicnet.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := iget-wrapper
LOCAL_SRC_FILES := iget-wrapper.cpp
LOCAL_LDLIBS := -llog
LOCAL_CPPFLAGS := -I$(ARMDIST_ROOT)/include

LOCAL_STATIC_LIBRARIES := \
    libicnet \
    libccnx_api_portal \
    libccnx_transport_rta \
    libccnx_api_control \
    libccnx_api_notify \
    libccnx_common \
    libparc \
    liblongbow \
    liblongbow-textplain \
    liblongbow-ansiterm \
    libevent \
    libboost_system \
    libcrystax \
    libcrypto \
    libssl
include $(BUILD_SHARED_LIBRARY)
