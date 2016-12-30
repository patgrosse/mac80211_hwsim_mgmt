SHELL=/bin/sh
MAKE = make
SUBDIRS ?= hwsim_mgmt
BIN = hwsim_mgmt/hwsim_mgmt
BINDIR = /usr/bin

all:
	@for i in $(SUBDIRS); do \
	echo "make all in $$i..."; \
	(cd $$i; $(MAKE) all); done

clean:
	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) clean); done

install: all
	install -m 0755 $(BIN) $(BINDIR)
