MAINTAINERCLEANFILES=$(srcdir)/Makefile.in
CLEANFILES=$(srcdir)/*~

ACLOCAL_AMFLAGS = -Im4

#test_CFLAGS = @CUNIT_INCLUDES@
#test_AM_LDLIBS = @CUNIT_LIBS@
test_LDADD = @CUNIT_LIBS@ -lmysqlclient

opendnssec_CFLAGS = @MYSQL_CFLAGS@
opendnssec_LDFLAGS = @MYSQL_LDFLAGS@
opendnssecd_CFLAGS = @MYSQL_CFLAGS@
opendnssecd_LDFLAGS = @MYSQL_LDFLAGS@

check_PROGRAMS = test
bin_PROGRAMS = opendnssec
sbin_PROGRAMS = opendnssecd

BUILT_SOURCES = schema_mysql.h

opendnssec_SOURCES = opendnssec.c model.c utilities.h schema_mysql.h
opendnssec_LDADD = schema_mysql.o
opendnssecd_SOURCES = opendnssec.c model.c utilities.h schema_mysql.h
opendnssecd_LDADD = schema_mysql.o

test_SOURCES = tests/opendnssectest.c

# opendnssec$(EXEEXT): opendnssec.c

TESTS = $(check_PROGRAMS)

%.o %.h : %.sql
	if [ "`tail -c 1 $(<) | tr '\000' X`" != X -o \
	     "`tail -c 1 $(<) | tr '\000' Y`" != Y ]; then \
	  echo -n 0 | tr 0 '\000' >> $(<) ; \
	fi
	objcopy --readonly-text -I binary -O elf64-x86-64 -B i386:x86-64 \
        --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
        --redefine-sym _binary_$*_sql_start=$* \
        --redefine-sym _binary_$*_sql_end=$*_end \
        --redefine-sym _binary_$*_sql_size=$*_size_sym \
	$(<) $(*).o
	echo "extern const char $(*)[];" > $(*).h
	echo "extern const char $(*)_end[];" >> $(*).h
	echo "extern const char $(*)_size_sym[];" >> $(*).h
	echo "#define $(*)_size ((int)$(*)_size_sym)" >> $(*).h
