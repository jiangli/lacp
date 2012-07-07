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

top_srcdir = .

include $(top_srcdir)/common.mk

LIBS= -lutil -lreadline  -ltermcap
INCLUDES = -IBITMAP -I$(RSTPLIBDIR) -I$(IPCDIR) -I$(CLIDIR) -I. 

EXECUTABLE= bridge mngr
RSTPLIBDIR = LACP
RSTPLIBS = lacp
RSTPLIBF = $(RSTPLIBDIR)/lib$(RSTPLIBS).a

IPCDIR   = IPC
IPCLIBS  = ipc
IPCLIBF  = $(IPCDIR)/lib$(IPCLIBS).a

CLIDIR   = CLI
CLILIBS  = cli
CLILIBF  = $(CLIDIR)/lib$(CLILIBS).a

BITMAPDIR   = BITMAP
BITMAPLIBS  = bitmap
BITMAPLIBF  = $(BITMAPDIR)/lib$(BITMAPLIBS).a

MNGRCFILES = mngr.c trace.c
MNGROFILES = mngr.o trace.o
BRIDGECFILES = bridge.c lac_cli.c lac_out.c trace.c
BRIDGEOFILES = $(BRIDGECFILES:.c=.o)

CFILES= $(MNGRCFILES) $(BRIDGECFILES)
OFILES = $(CFILES:.c=.o)

.SUFFIXES: .S .c .o .s

all: libs $(EXECUTABLE)

mngr: ${MNGROFILES} $(IPCLIBF) $(CLILIBF) $(BITMAPLIBF)
	$(CC) ${MNGROFILES} $(LIBS) \
		-L$(IPCDIR) -l$(IPCLIBS) -L$(CLIDIR) -l$(CLILIBS) -L$(BITMAPDIR) -l$(BITMAPLIBS) -o $@

bridge: ${BRIDGEOFILES} $(RSTPLIBF) $(IPCLIBF) $(CLILIBF)
	$(CC) $(CFLAGS) ${BRIDGEOFILES} $(RSTPLIBDIR)/liblacp.a $(LIBS) \
		-L$(IPCDIR) -l$(IPCLIBS) -L$(CLIDIR) -l$(CLILIBS) -L$(BITMAPDIR) -l$(BITMAPLIBS) -o $@

libs:
	cd $(RSTPLIBDIR); make
	cd $(IPCDIR); make
	cd $(CLIDIR); make

$(RSTPLIBF):
	cd $(RSTPLIBDIR); make

$(IPCLIBF):
	cd $(IPCDIR); make

$(CLILIBF):
	cd $(CLIDIR); make

$(BITMAPLIBF):
	cd $(BITMAPDIR); make

clean:
	cd $(IPCDIR); make clean
	cd $(CLIDIR); make clean
	cd $(RSTPLIBDIR); make clean
	rm -f all *.o *.a *.so *~ $(EXECUTABLE) *.bak make.depend

indent: $(CFILES)
	cd $(IPCDIR); make indent
	cd $(CLIDIR); make indent
	cd $(RSTPLIBDIR); make indent
	for i in $(CFILES) ; do \
	( indent $(INDENT_OPTIONS) $$i) ; \
	done

depend:
	cd $(IPCDIR); make depend
	cd $(CLIDIR); make depend
	cd $(RSTPLIBDIR); make depend
	echo "# DO NOT DELETE THIS LINE -- make  depend  depends  on it." > make.depend
	makedepend -- $(DEFS) $(CFLAGS)  -- $(CFILES) -f make.depend

-include make.depend
