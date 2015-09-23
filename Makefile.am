MAINTAINERCLEANFILES= $(srcdir)/Makefile.in

ACLOCAL_AMFLAGS = -Im4

AM_CFLAGS = @CUNIT_INCLUDES@
AM_LDLIBS = @CUNIT_LIBS@

check_PROGRAMS = test
bin_PROGRAMS = opendnssec
sbin_PROGRAMS = opendnssecd

opendnssec_SOURCES = opendnssec.c
opendnssecd_SOURCES = opendnssec.c

test_SOURCES = opendnssectest.c
opendnssec: opendnssec.c

check:	test
	./test
