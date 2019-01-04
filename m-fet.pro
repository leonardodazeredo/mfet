TEMPLATE = subdirs
SUBDIRS = src/src.pro src/src-cl.pro

unix {
translations.path = /usr/share/metafet/translations
translations.files = translations/*.qm

examples.path = /usr/share/m-fet
examples.files = examples/

desktop.path = /usr/share/applications
desktop.files = m-fet.desktop

documentation.path = /usr/share/doc/m-fet
documentation.files = doc/* COPYING README.md

manual.path = /usr/share/man/man1
manual.files = man/*

icon_png.path = /usr/share/pixmaps
icon_png.files = icons/m-fet.png

INSTALLS += desktop manual documentation icon_png
}
