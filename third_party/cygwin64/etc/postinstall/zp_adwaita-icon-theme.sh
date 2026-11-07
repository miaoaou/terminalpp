if [ ! -f /usr/share/icons/Adwaita/icon-theme.cache ]
then
    /usr/bin/gtk-update-icon-cache --force /usr/share/icons/Adwaita || :
fi
