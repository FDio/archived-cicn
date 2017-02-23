#
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
#


include ../config.mk

docs: doxygen

doxygen: Makefile stage1 stage2

stage1: stage1-message doxygen-libccnx-stage1 doxygen-libccnx_api_control-stage1 doxygen-libccnx_api_notify-stage1 doxygen-libccnx_api_portal-stage1  doxygen-librta-stage1
stage2: stage2-message doxygen-libccnx-stage2 doxygen-libccnx_api_control-stage2 doxygen-libccnx_api_notify-stage2 doxygen-libccnx_api_portal-stage2  doxygen-librta-stage2

# Stage 1

stage1-message:
	@echo
	@echo !!  Running doxygen to create *only* TAG files.
	@echo

doxygen-libccnx-stage1: libccnx-stage1.doxygen
	${DOXYGEN_BIN} -s libccnx-stage1.doxygen >& doxygen-libccnx-stage1.log

doxygen-libccnx_api_control-stage1: libccnx_api_control-stage1.doxygen
	${DOXYGEN_BIN} -s libccnx_api_control-stage1.doxygen >& doxygen-libccnx_api_control-stage1.log

doxygen-libccnx_api_notify-stage1: libccnx_api_notify-stage1.doxygen
	${DOXYGEN_BIN} -s libccnx_api_notify-stage1.doxygen >& doxygen-libccnx_api_notify-stage1.log

doxygen-libccnx_api_portal-stage1: libccnx_api_portal-stage1.doxygen
	${DOXYGEN_BIN} -s libccnx_api_portal-stage1.doxygen >& doxygen-libccnx_api_portal-stage1.log

doxygen-librta-stage1: librta-stage1.doxygen
	${DOXYGEN_BIN} -s librta-stage1.doxygen >& doxygen-librta-stage1.log

# Stage 2

stage2-message:
	@echo
	@echo !!  Now running doxygen to create documentation, referencing previously generated TAG files.
	@echo

doxygen-libccnx-stage2: libccnx-stage2.doxygen
	${DOXYGEN_BIN} -s libccnx-stage2.doxygen >& doxygen-libccnx-stage2.log
	${LONGBOW_DOXYGEN_BIN_REPORT} --doxygenlog doxygen-libccnx-stage2.log

doxygen-libccnx_api_control-stage2: libccnx_api_control-stage2.doxygen
	${DOXYGEN_BIN} -s libccnx_api_control-stage2.doxygen >& doxygen-libccnx_api_control-stage2.log
	${LONGBOW_DOXYGEN_BIN_REPORT} --doxygenlog doxygen-libccnx_api_control-stage2.log

doxygen-libccnx_api_notify-stage2: libccnx_api_notify-stage2.doxygen
	${DOXYGEN_BIN} -s libccnx_api_notify-stage2.doxygen >& doxygen-libccnx_api_notify-stage2.log
	${LONGBOW_DOXYGEN_BIN_REPORT} --doxygenlog doxygen-libccnx_api_notify-stage2.log

doxygen-libccnx_api_portal-stage2: libccnx_api_portal-stage2.doxygen
	${DOXYGEN_BIN} -s libccnx_api_portal-stage2.doxygen >& doxygen-libccnx_api_portal-stage2.log
	${LONGBOW_DOXYGEN_BIN_REPORT} --doxygenlog doxygen-libccnx_api_portal-stage2.log

doxygen-librta-stage2: librta-stage2.doxygen
	${DOXYGEN_BIN} -s librta-stage2.doxygen >& doxygen-librta-stage2.log
	${LONGBOW_DOXYGEN_BIN_REPORT} --doxygenlog doxygen-librta-stage2.log

website: doxygen
	$(MAKE) -C Jekyll

# The Doxygen output directory removed here is set in libparc.conf
clobber: clean
	${RM} -r libccnx-documentation
	${RM} -r libccnx_api_control-documentation
	${RM} -r libccnx_api_keyvalue-documentation
	${RM} -r libccnx_api_mqueue-documentation
	${RM} -r libccnx_api_notify-documentation
	${RM} -r libccnx_api_portal-documentation
	${RM} -r librta-documentation

CLEANFILES=doxygen.log doxygen-libccnx.log doxygen-libccnx_api_control.log \
	doxygen-libccnx_api_keyvalue.log doxygen-libccnx_api_mqueue.log \
	doxygen-libccnx_api_notify.log doxygen-libccnx_api_notify.log \
	doxygen-libccnx_api_portal.log doxygen-librta.log *.doctags
