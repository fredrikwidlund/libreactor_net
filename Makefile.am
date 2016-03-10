ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
AM_CFLAGS = -std=gnu11 -O3 -flto -fuse-linker-plugin -I$(srcdir)/src/picohttpparser
AM_LDFLAGS = -static

SOURCE_FILES = \
src/reactor_net/reactor_resolver.c \
src/reactor_net/reactor_tcp_client.c \
src/reactor_net/reactor_tcp_server.c

HEADER_FILES = \
src/reactor_net/reactor_resolver.h \
src/reactor_net/reactor_tcp_client.h \
src/reactor_net/reactor_tcp_server.h

MAIN_HEADER_FILES = \
src/reactor_net.h

AUTOMAKE_OPTIONS = subdir-objects
lib_LTLIBRARIES = libreactor_net.la
libreactor_net_la_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)

headerfilesdir = $(includedir)/reactor_net
headerfiles_HEADERS = $(HEADER_FILES)

mainheaderdir = $(includedir)
mainheader_HEADERS = $(MAIN_HEADER_FILES)

MAINTAINERCLEANFILES = aclocal.m4 config.h.in configure Makefile.in libreactor_net-?.?.?.tar.gz
maintainer-clean-local:; rm -rf autotools m4 libreactor_net-?.?.

### unit tests ###

dist_noinst_SCRIPTS = test/valgrind.sh test/coverage.sh

CLEANFILES = {.,src/reactor_net,src/picohttpparser,test}/*.{gcno,gcda,gcov}
