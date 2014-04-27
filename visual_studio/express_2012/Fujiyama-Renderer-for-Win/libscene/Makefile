#Copyright (c) 2011-2014 Hiroshi Tsubokawa
#See LICENSE and README

# Generates .def file for core DLL

srcdir  := ../../../../src
target  := libscene.def
dllname := libscene

.PHONY: $(target)

$(target):
	@echo '  making' $@
	@echo 'LIBRARY' $(dllname) > $@
	@echo 'EXPORTS' >> $@
	@cat $(srcdir)/*.h \
	| sed -e's/struct //g' \
	| sed -e's/const //g' \
	| sed -e's/\*/ /g' \
	| awk -F'[ ()]+' '/^extern / {if ($$2 != "\"C\"") print "\t" $$3}' >> $@
