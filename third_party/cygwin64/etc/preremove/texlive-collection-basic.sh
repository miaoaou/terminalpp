/usr/bin/touch /var/lib/texmf/postinstall/texlive-collection-basic_rm.lsr
/usr/bin/updmap-sys --nohash --nomkmap \
--disable euler.map \
--disable cm.map \
--disable cmextra.map \
--disable cyrillic.map \
--disable latxfont.map \
--disable symbols.map \
--disable cmtext-bsr-interpolated.map \
--disable dummy-space.map \
--disable mathpple.map \

/usr/bin/fmtutil-sys --disablefmt luahbtex/luahbtex
/usr/bin/fmtutil-sys --disablefmt dviluatex/luatex
/usr/bin/fmtutil-sys --disablefmt luatex/luatex
/usr/bin/fmtutil-sys --disablefmt mf/mf-nowin
/usr/bin/fmtutil-sys --disablefmt etex/pdftex
/usr/bin/fmtutil-sys --disablefmt pdfetex/pdftex
/usr/bin/fmtutil-sys --disablefmt pdftex/pdftex
/usr/bin/fmtutil-sys --disablefmt tex/tex
/usr/libexec/update_tlpdb remove \
amsfonts \
bibtex \
bibtex.x86_64-cygwin \
cm \
collection-basic \
colorprofiles.doc \
colorprofiles \
dvipdfmx \
dvipdfmx.x86_64-cygwin \
dvips \
dvips.x86_64-cygwin \
ec \
enctex \
etex-pkg \
etex \
glyphlist \
graphics-def \
hyph-utf8 \
hyphen-base \
hyphenex \
ifplatform \
iftex \
knuth-lib \
knuth-local \
kpathsea \
kpathsea.x86_64-cygwin \
lua-alt-getopt \
luahbtex \
luahbtex.x86_64-cygwin \
luatex \
luatex.x86_64-cygwin \
makeindex \
makeindex.x86_64-cygwin \
metafont \
metafont.x86_64-cygwin \
mflogo \
mfware \
mfware.x86_64-cygwin \
modes \
pdftex \
pdftex.x86_64-cygwin \
plain \
tex-ini-files \
tex.doc \
tex \
tex.x86_64-cygwin \
texlive-common \
texlive-en \
texlive-msg-translations \
texlive-scripts \
texlive-scripts.x86_64-cygwin \
texlive.infra.doc \
texlive.infra \
texlive.infra.x86_64-cygwin \
tlshell.doc \
tlshell \
tlshell.x86_64-cygwin \
unicode-data \
xdvi \
xdvi.x86_64-cygwin \

