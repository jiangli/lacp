#************************************************************************
#* RSTP library - Rapid Spanning Tree (802.1t, 802.1w)
#* Copyright (C) 2001-2003 Optical Access
#* Author: Alex Rozin
#*
#* This file is part of RSTP library.
#*
#* RSTP library is free software; you can redistribute it and/or modify it
#* under the terms of the GNU Lesser General Public License as published by the
#* Free Software Foundation; version 2.1
#*
#* RSTP library is distributed in the hope that it will be useful, but
#* WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
#* General Public License for more details.
#*
#* You should have received a copy of the GNU Lesser General Public License
#* along with RSTP library; see the file COPYING.  If not, write to the Free
#* Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#* 02111-1307, USA.
#**********************************************************************/

top_srcdir = ../

include $(top_srcdir)/common.mk

INCLUDES = -I. -I../BITMAP

RSTPLIBS = lacp
RSTPLIBF = lib$(RSTPLIBS).a
CFILES = statmch.c  lac_in.c   \
lac_pdu.c lac_mux.c  lac_port.c lac_rx.c lac_sel.c lac_sys.c lac_test.c lac_tx.c

OFILES= $(CFILES:.c=.o)

.SUFFIXES: .c .o

.c.o:
	$(COMPILE) -c $<  -o $@

all: $(RSTPLIBF)

$(RSTPLIBF): $(OFILES)
	-rm -f $@
	$(AR) $(ARFLAGS) $@ $(OFILES)
	$(RANLIB)  $@

indent: $(CFILES)
	for i in $(CFILES) ; do \
	( indent $(INDENT_OPTIONS) $$i) ; \
	done

clean:
	rm -f *.o *.a *.so *~ $(TARGET) *.bak make.depend

depend:
	echo "# DO NOT DELETE THIS LINE -- make  depend  depends  on it." > make.depend
	makedepend -- $(DEFS) $(INCLUDES) $(CFLAGS) -- $(CFILES) -f make.depend

-include make.depend






