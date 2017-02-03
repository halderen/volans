MAINTAINERCLEANFILES=$(srcdir)/Makefile.in
CLEANFILES=$(srcdir)/*~ schema_mysql.h

ACLOCAL_AMFLAGS = -Im4

volans_CFLAGS = @MYSQL_CFLAGS@ -D_REENTRANT
volans_LDFLAGS = @MYSQL_LDFLAGS@
volansd_CFLAGS = @MYSQL_CFLAGS@ -D_REENTRANT
volansd_LDFLAGS = @MYSQL_LDFLAGS@

libvolans_la_SOURCES = model.c

check_PROGRAMS = test
lib_LTLIBRARIES = libvolans.la
bin_PROGRAMS = volans
sbin_PROGRAMS = volansd

BUILT_SOURCES = schema_mysql.h

volans_SOURCES = volans.c model.c utilities.h schema_mysql.h
volans_LDADD = schema_mysql.o
volansd_SOURCES = volans.c model.c utilities.h schema_mysql.h
volansd_LDADD = schema_mysql.o

test_SOURCES = tests/opendnssectest.c model.c schema_mysql.h
test_CFLAGS = @CUNIT_INCLUDES@ @MYSQL_CFLAGS@
#test_AM_LDLIBS = @CUNIT_LIBS@
test_LDFLAGS = @MYSQL_LDFLAGS@
test_LDADD = @CUNIT_LIBS@ schema_mysql.o -lmysqlclient

# volans$(EXEEXT): volans.c

TESTS = $(check_PROGRAMS)

%.h %.o : %.sql
	-if [ "`tail -c 1 schema_mysql.sql | tr '\000' X`" != X -o \
	     "`tail -c 1 schema_mysql.sql | tr '\000' Y`" != Y ]; then \
	  echo -n 0 | tr 0 '\000' >> $(<) ; \
	fi
	-objcopy --readonly-text -I binary -O elf64-x86-64 -B i386:x86-64 \
        --rename-section .data=.progmem.data,contents,alloc,load,readonly,data \
        --redefine-sym _binary_$*_sql_start=$* \
        --redefine-sym _binary_$*_sql_end=$*_end \
        --redefine-sym _binary_$*_sql_size=$*_size_sym \
	$(<) $(*).o
	echo "extern const char " $(*)"[];" > $(*).h
	echo "extern const char " $(*)_end"[];" >> $(*).h
	echo "extern const char " $(*)_size_sym"[];" >> $(*).h
	echo "#define $(*)_size ((int)$(*)_size_sym)" >> $(*).h

#schema_mysql.h:	schema_mysql.o
#	echo "extern const char schema_mysql[];" >            schema_mysql.h
#	echo "extern const char schema_mysql_end[];"          >> schema_mysql.h
#	echo "extern const char schema_mysql_size_sym[];"     >> schema_mysql.h
#	echo "#define schema_mysql_size ((int)schema_mysql_size_sym)" >> schema_mysql.h
