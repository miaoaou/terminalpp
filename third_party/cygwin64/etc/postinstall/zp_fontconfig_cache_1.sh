mkdir -p /usr/share/fonts/microsoft
# remove any broken links (-L -type l together)
/usr/bin/find -L /usr/share/fonts/microsoft -type l -delete

/usr/bin/find "`cygpath -W`"/Fonts/ -name '*.ttf' -exec grep -FlZ 'Microsoft Corp' '{}' + | while read -d $'\0' f
do
	if [ ! -e "/usr/share/fonts/microsoft/${f##*/}" ]
	then
		ln -s "$f" /usr/share/fonts/microsoft/
	fi
done

/usr/libexec/fc-cache-1 -s || :
