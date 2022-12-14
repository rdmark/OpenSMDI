INSTALLDEST = /lib/smdi.so.2.0.05
LINKDEST = /lib/smdi.so.2
BINDIR = ./bin/linux
SMDI_SO_2 = $(BINDIR)/libsmdi.so.2
SMDI_O = $(BINDIR)/smdi.o
ASPI_LINUX_O = $(BINDIR)/aspi.linux.o
LIBRARY = libsmdi.so.2

default: $(SMDI_SO_2) $(SMDI_O)

$(SMDI_SO_2): smdi.c $(ASPI_LINUX_O) $(SMDI_O)
	gcc -shared -Wl,-soname,$(LIBRARY) -o $(SMDI_SO_2) $(SMDI_O) $(LIBPTHREADS) $(ASPI_LINUX_O)

$(SMDI_O): smdi.c
	gcc -c smdi.c -o $(SMDI_O)

$(ASPI_LINUX_O): aspi.linux.c
	gcc -c aspi.linux.c -o $(ASPI_LINUX_O)

install: $(SMDI_SO_2)
	@echo
	@echo "Installing $(LIBRARY) on your system"
	cp $(SMDI_SO_2) $(INSTALLDEST)
	rm -f $(LINKDEST)
	ln -s $(INSTALLDEST) $(LINKDEST)
	@echo

clean:
	@echo
	@echo "Removing all binaries"
	rm -f $(SMDI_O)
	rm -f $(ASPI_LINUX_O)
	rm -f $(SMDI_SO_2)
	@echo

dist:
	@echo
	@echo "Making distribution"
	cd .. ; rm -f OpenSMDI-0.05.tar.gz ; tar -c OpenSMDI-0.05 -f OpenSMDI-0.05.tar ; gzip OpenSMDI-0.05.tar ; cd OpenSMDI-0.05
	@echo

