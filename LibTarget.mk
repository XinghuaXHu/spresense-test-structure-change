
$(SDKDIR)$(DELIM)..$(DELIM)test$(DELIM)libtest$(LIBEXT): context
	$(Q) $(MAKE) -C $(dir $@) TOPDIR="$(TOPDIR)" SDKDIR="$(SDKDIR)" $(notdir $@)

lib$(DELIM)libtest$(LIBEXT): $(SDKDIR)$(DELIM)..$(DELIM)test$(DELIM)libtest$(LIBEXT)
	$(Q) install $< $@

EXTLIBS += lib$(DELIM)libtest$(LIBEXT)
