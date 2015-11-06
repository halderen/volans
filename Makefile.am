MAINTAINERCLEANFILES= $(srcdir)/Makefile.in

ACLOCAL_AMFLAGS = -Im4

#test_CFLAGS = @CUNIT_INCLUDES@
#test_AM_LDLIBS = @CUNIT_LIBS@
test_LDADD = @CUNIT_LIBS@

check_PROGRAMS = test
bin_PROGRAMS = opendnssec
sbin_PROGRAMS = opendnssecd

opendnssec_SOURCES = opendnssec.c utilities.h
opendnssecd_SOURCES = opendnssec.c utilities.h

test_SOURCES = tests/opendnssectest.c

opendnssec$(EXEEXT): opendnssec.c

TESTS = $(check_PROGRAMS)

#check:	test
#	./test
