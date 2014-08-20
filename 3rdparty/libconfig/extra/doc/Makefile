HTMLS = libconfig.html
TEXINFOS = libconfig.texi
MAKEINFOHTML = $(MAKEINFO) --html
MAKEINFO = makeinfo

all: html

.SUFFIXES: .html .texi

libconfig.html: $(TEXINFOS)

clean:
	@-test -z "$(HTMLS)" || rm -rf libconfig.html

.PHONY: all clean html

html: $(HTMLS)
	$(MAKEINFOHTML) --no-split $(TEXINFOS)
