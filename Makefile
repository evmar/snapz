# I tried to use waf but their examples wouldn't even run.  :(

all: snapz

packages=--pkg gtk+-2.0 --pkg libsoup-2.4 --pkg json-glib-1.0

.PHONY: debug
debug: snapz.vala
	valac -g --save-temps $(packages) snapz.vala

snapz: snapz.vala
	valac --Xcc=-O2 $(packages) snapz.vala -o snapz
