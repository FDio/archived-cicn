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
include ../../config.mk

.PHONY: manpages

SUBDIRS =

OUTDIR= ../manpage

MAN = $(OUTDIR)/metis_daemon.1 $(OUTDIR)/metis_control.1 $(OUTDIR)/metis.cfg.5
TXT = $(OUTDIR)/metis_daemon.1.txt $(OUTDIR)/metis_control.1.txt $(OUTDIR)/metis.cfg.5.txt
HTML = $(OUTDIR)/metis_daemon.1.html $(OUTDIR)/metis_control.1.html $(OUTDIR)/metis.cfg.5.html
PDF = $(OUTDIR)/metis_daemon.1.pdf $(OUTDIR)/metis_control.1.pdf $(OUTDIR)/metis.cfg.5.pdf

all:
	@echo "usage: make manpages"

manpages: $(TXT) $(MAN) $(HTML) $(PDF)

$(OUTDIR):
	mkdir -p $(OUTDIR)

######
$(OUTDIR)/metis_daemon.1: metis_daemon.sgml $(OUTDIR)
	docbook-to-man $< > $@

$(OUTDIR)/metis_daemon.1.txt: $(OUTDIR)/metis_daemon.1 $(OUTDIR)
	groff -man -Tutf8 $< > $@

$(OUTDIR)/metis_daemon.1.html: $(OUTDIR)/metis_daemon.1 $(OUTDIR)
	groff -man -Thtml $< > $@

$(OUTDIR)/metis_daemon.1.pdf: $(OUTDIR)/metis_daemon.1 $(OUTDIR)
	groff -man -Tps $< | ps2pdf - $@

######
$(OUTDIR)/metis_control.1: metis_control.sgml $(OUTDIR)
	docbook-to-man $< > $@

$(OUTDIR)/metis_control.1.txt: $(OUTDIR)/metis_control.1 $(OUTDIR)
	groff -man -Tascii $< > $@

$(OUTDIR)/metis_control.1.html: $(OUTDIR)/metis_control.1 $(OUTDIR)
	groff -man -Thtml $< > $@

$(OUTDIR)/metis_control.1.pdf: $(OUTDIR)/metis_control.1 $(OUTDIR)
	groff -man -Tps $< | ps2pdf - $@

######
$(OUTDIR)/metis.cfg.5: metis.cfg.sgml $(OUTDIR)
	docbook-to-man $< > $@

$(OUTDIR)/metis.cfg.5.txt: $(OUTDIR)/metis.cfg.5 $(OUTDIR)
	groff -man -Tascii $< > $@

$(OUTDIR)/metis.cfg.5.html: $(OUTDIR)/metis.cfg.5 $(OUTDIR)
	groff -man -Thtml $< > $@

$(OUTDIR)/metis.cfg.5.pdf: $(OUTDIR)/metis.cfg.5 $(OUTDIR)
	groff -man -Tps $< | ps2pdf - $@


######
#clobber: clean
#	${RM} -rf $(OUTDIR)
#
#CLEANFILES=$(TXT) $(MAN) $(HTML)

