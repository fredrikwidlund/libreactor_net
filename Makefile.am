ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS} -I m4
AM_CFLAGS = -std=gnu11 -O3 -flto -fuse-linker-plugin -I$(srcdir)/src/picohttpparser
AM_LDFLAGS = -static

SOURCE_FILES = \
src/reactor_net/reactor_http_server.c \
src/reactor_net/reactor_resolver.c \
src/reactor_net/reactor_rest_server.c \
src/reactor_net/reactor_tcp_client.c \
src/reactor_net/reactor_tcp_server.c \
src/picohttpparser/picohttpparser.c

HEADER_FILES = \
src/reactor_net/reactor_http_server.h \
src/reactor_net/reactor_resolver.h \
src/reactor_net/reactor_rest_server.h \
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
maintainer-clean-local:; rm -rf autotools m4 libreactor_net-?.?.?

### unit tests ###

CHECK_CFLAGS = -std=gnu11 -O0 -g -ftest-coverage -fprofile-arcs -I$(srcdir)/src -I$(srcdir)/src/picohttpparser
CHECK_LDADD = -L. -lreactor_net_test -ldynamic -lcmocka
CHECK_LDFLAGS_EXTRA = \
-Wl,--wrap=malloc \
-Wl,--wrap=calloc \
-Wl,--wrap=realloc \
-Wl,--wrap=aligned_alloc \
-Wl,--wrap=epoll_create1 \
-Wl,--wrap=epoll_wait \
-Wl,--wrap=epoll_ctl \
-Wl,--wrap=fcntl \
-Wl,--wrap=read \
-Wl,--wrap=timerfd_create \
-Wl,--wrap=sigprocmask \
-Wl,--wrap=signalfd

check_LIBRARIES = libreactor_net_test.a
libreactor_net_test_a_CFLAGS = $(CHECK_CFLAGS)
libreactor_net_test_a_SOURCES = $(SOURCE_FILES) $(HEADER_FILES)

check_PROGRAMS = test/reactor_resolver
test_reactor_resolver_CFLAGS = $(CHECK_CFLAGS)
test_reactor_resolver_LDADD = $(CHECK_LDADD)
test_reactor_resolver_LDFLAGS = $(CHECK_LDFLAGS_EXTRA)
test_reactor_resolver_SOURCES = test/reactor_resolver.c test/stubs.c

dist_noinst_SCRIPTS = test/valgrind.sh test/coverage.sh

TESTS = $(check_PROGRAMS) test/coverage.sh test/valgrind.sh

CLEANFILES = {.,src/reactor_net,src/picohttpparser,test}/*.{gcno,gcda,gcov}
