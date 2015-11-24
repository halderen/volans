MAINTAINERCLEANFILES= $(srcdir)/Makefile.in

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

BUILT_SOURCES = schema-mysql.h

opendnssec_SOURCES = opendnssec.c model.c utilities.h schema-mysql.h
opendnssec_LDADD = schema-mysql.o
opendnssecd_SOURCES = opendnssec.c model.c utilities.h schema-mysql.h
opendnssecd_LDADD = schema-mysql.o

test_SOURCES = tests/opendnssectest.c

# opendnssec$(EXEEXT): opendnssec.c

TESTS = $(check_PROGRAMS)

clean-local:
	-rm -f schema-mysql.h

%.o %.h : %.sql
	if [ "`tail -c 1 $(<) | tr '\000' X`" != X -o \
	     "`tail -c 1 $(<) | tr '\000' Y`" != Y ]; then \
	  echo -n 0 | tr 0 '\000' >> $(<) ; \
	fi
	objcopy --readonly-text -I binary -O `objdump -i | head -n 2 | tail -n 1` -B `objdump -i | head -n 4 | tail -n 1`:`objdump -i | head -n 2 | tail -n 1 | sed 's/^[^-]*-//'` \
          --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
          --redefine-sym _binary_`echo $* | tr - _`_sql_start=`echo $* | tr .- _` \
          --redefine-sym _binary_`echo $* | tr - _`_sql_end=`echo $* | tr .- _`_end \
          --redefine-sym _binary_`echo $* | tr - _`_sql_size=`echo $* | tr .- _`_size_sym \
	  $(<) $(*).o
	echo "extern const char `echo $(*) | tr .- _`[];" > $(*).h
	echo "extern const char `echo $(*) | tr .- _`_end[];" >> $(*).h
	echo "extern const char `echo $(*) | tr .- _`_size_sym[];" >> $(*).h
	echo "#define `echo $(*) | tr .- _`_size ((int)`echo $(*) | tr .- _`_size_sym)" >> $(*).h
