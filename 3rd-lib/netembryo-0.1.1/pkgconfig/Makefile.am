pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnetembryo.pc

if HAVE_SCTP
pkgconfig_DATA += libnetembryo-sctp.pc
noinst_DATA = libnetembryo-sctp-uninstalled.pc
endif

CLEANFILES = libnetembryo-sctp.pc libnetembryo-sctp-uninstalled.pc

libnetembryo-sctp%: libnetembryo%
	ln -s $< $@
