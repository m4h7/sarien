# $Id: Makefile,v 1.3 2001/07/05 03:45:06 cmatsuoka Exp $

XCFLAGS =
OBJS	=
DFILES	= Makefile
CFILES	=
DDIRS	= include filesys sound graphics core
XDFILES =
LIB	=

all:
	for i in $(DDIRS); do		\
		(cd $$i; $(MAKE) all);	\
	done

include ../Rules

