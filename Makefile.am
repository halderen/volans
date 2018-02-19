MAINTAINERCLEANFILES=$(srcdir)/Makefile.in
CLEANFILES=$(srcdir)/*~ schema_mysql.h core* vgcore*

ACLOCAL_AMFLAGS = -Im4

volans_CFLAGS = @MYSQL_CFLAGS@ @SQLITE3_CFLAGS@ -D_REENTRANT -O2
#volans_LDFLAGS = @MYSQL_LDFLAGS@ @SQLITE3_LDFLAGS@
volans_LDFLAGS = @MYSQL_LDFLAGS@ @LDNS_LIBS@ -levent -ldl

# libvolans_la_SOURCES = model.c

check_PROGRAMS = test
# lib_LTLIBRARIES = libvolans.la
bin_PROGRAMS = volans

install-exec-hook:
	ln $(DESTDIR)$(bindir)/volans$(EXEEXT) \
	   $(DESTDIR)$(bindir)/volansd$(EXEEXT)

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/volansd$(EXEEXT)

BUILT_SOURCES = schema_mysql.h
EXTRA_DIST = schema_mysql.sql schema_sqlite.sql

volans_SOURCES = volans.c utilities.c utilities.h backend.h \
                 daemonize.c cmdhandler.c cmdhandler.h \
                 marshalling.c httpd.c rpc.c httpd.h proto.h \
                 iterator.c iteratorgeneric.c iteratorarray.c \
                 dictionary.c views.c table.c commitlog.c index.c \
                 zoneinput.c zoneoutput.c zonesign.c zonesignrecord.c names.c \
                 uthash.h utarray.h utmm.h utstring.h utlist.h \
                 utringbuffer.h utvector.h pkcs11.h \
                 backendsqlite.c backendmysql.c

if USE_SQLITE3
volans_SOURCES += schema_sqlite.h
endif
if USE_MYSQL
volans_SOURCES += schema_mysql.h
endif

volans_LDADD = schema_mysql.o schema_sqlite.o $(LIBCURL)

test_SOURCES = tests/opendnssectest.c schema_mysql.h
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
