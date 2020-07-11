noinst_PROGRAMS = nfs-test nfs-test2 nfsclient-raw
ATF_TESTS_C= nfs-test3 
noinst_LTLIBRARIES = libnfsutils.la libauditutils.la
libnfsutils_la_SOURCES = nfs-utils.h nfs-utils.c
libauditutils_la_SOURCES = audit-utils.c audit-utils.h

AM_CPPFLAGS = \
	-I$(abs_top_srcdir)/include \
	-I$(abs_top_srcdir)/include/nfsc \
	-I$(abs_top_srcdir)/include/win32 \
	-I$(abs_top_srcdir)/mount \
	-I$(abs_top_srcdir)/nfs \
	-I$(abs_top_srcdir)/nfs4 \
	-I$(abs_top_srcdir)/portmap \
	-I$(abs_top_srcdir)/rquota \
	"-D_U_=__attribute__((unused))"

COMMON_LIBS = ../lib/libnfs.la libnfsutils.la -lpopt libauditutils.la
nfs_test_LDADD = $(COMMON_LIBS)
nfsclient_raw_LDADD = $(COMMON_LIBS)
nfs_test2_LDADD = $(COMMON_LIBS)
nfs_test3_LDADD = $(COMMON_LIBS)

.include <bsd.test.mk>
