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
BRIDGECFILES = bridge.c lacp_cli.c  lacp_ssp.c lacp_stub.c trace.c trunk_api.c
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
