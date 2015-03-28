#
#        Author:
#        Sep 9th, 2010
#

-include $(PROJ_ROOT)/Rules.mk
LIB_PROJ := $(shell pwd)

TARG = $(PROJ_ROOT)/lib/bin/libstor.a
SUB_LINK = L_TARG
SUB_LINK_PATH = $(LIB_PROJ)/lib/lib
SUB_LIBS = -lmk2fs -lcom_err -lblkid -luuid -lquota -lext2fs -le2p 

$(TARG):
	echo "-L$(LIB_PROJ)/lib -lstor -L$(SUB_LINK_PATH) $(SUB_LIBS)" > $(SUB_LINK)
	cp $(LIB_PROJ)/lib/libstor.a $(TARG) -rf

.PHONY:clean
clean:
	$(RM) -rf $(TARG) $(SUB_LINK)

