
top_srcdir = ../

include $(top_srcdir)/common.mk

INCLUDES = -I. -I../BITMAP -I../

RSTPLIBS = lacp
RSTPLIBF = lib$(RSTPLIBS).a
CFILES = lacp_statmch.c lacp_mux.c  lacp_port.c lacp_rx.c lacp_sel.c lacp_sys.c lacp_tx.c

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






