#
# How to make Xtank documentation
#
# NOTE: This makefile was designed for use by GNU make.  I'm sure that
#       normal make doesn't work for it.
#

latexfiles = essential.latex mdesign.latex vdesign.latex xtanklib.latex
dvifiles = $(subst .latex,.dvi,$(latexfiles))
psfiles = $(subst .latex,.PS,$(latexfiles))

ps : $(psfiles)
	@echo "*** Done creating .PS files"

dvi : $(dvifiles)
	@echo "*** Done creating .dvi files"

install : $(psfiles)
	cp $(psfiles) ..
	@echo "*** Done installing .PS files"

#
# Dependency rules
#

%.dvi : %.latex
	latex $<

%.PS : %.dvi
	dvi2ps -r $< > $@
