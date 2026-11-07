/usr/bin/touch /var/lib/texmf/postinstall/texlive-collection-latex_rm.lsr
/usr/bin/updmap-sys --nohash --nomkmap \
--disable charter.map \
--disable fpls.map \
--disable pazo.map \
--disable utopia.map \

/usr/bin/fmtutil-sys --disablefmt dvilualatex/luatex
/usr/bin/fmtutil-sys --disablefmt latex/pdftex
/usr/bin/fmtutil-sys --disablefmt lualatex/luahbtex
/usr/bin/fmtutil-sys --disablefmt pdflatex/pdftex
/usr/bin/fmtutil-sys --disablefmt mptopdf/pdftex
/usr/libexec/update_tlpdb remove \
ae \
amscls \
amsmath \
atbegshi \
atveryend \
auxhook \
babel-english \
babel \
babelbib \
bigintcalc \
bitset \
bookmark \
carlisle \
collection-latex \
colortbl \
epstopdf-pkg \
etexcmds \
fancyhdr \
firstaid \
fix2col \
geometry \
gettitlestring \
graphics-cfg \
graphics \
grfext \
hopatch \
hycolor \
hyperref \
intcalc \
kvdefinekeys \
kvoptions \
kvsetkeys \
l3backend \
l3kernel \
l3packages \
latex-bin.doc \
latex-bin \
latex-bin.x86_64-cygwin \
latex-fonts \
latex-lab \
latex \
latexconfig \
letltxmacro \
ltxcmds \
ltxmisc \
mfnfss \
mptopdf.doc \
mptopdf \
mptopdf.x86_64-cygwin \
natbib \
oberdiek \
pagesel \
pdfescape \
pdftexcmds \
pslatex \
psnfss \
pspicture \
refcount \
rerunfilecheck \
stringenc \
tools \
uniquecounter \
url \

