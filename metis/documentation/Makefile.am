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

SUBDIRS = manpage-source manpage

docs: doxygen-metis

doxygen-metis: metis.doxygen
	${DOXYGEN_BIN} -s metis.doxygen
	${LONGBOW_DOXYGEN_BIN_REPORT} -a --doxygenlog metis-doxygen.log

website: longbow.log
	$(MAKE) -C Jekyll

# The Doxygen output directory removed here is set in metis.doxygen
clobber: clean
	${RM} -rf metis-documentation

CLEANFILES=metis-doxygen.log
