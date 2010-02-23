# I tried to use waf but their examples wouldn't even run.  :(

all: snapz

.PHONY: debug
debug: snapz.vala
	valac -g --save-temps --pkg gtk+-2.0 snapz.vala

snapz: snapz.vala
	valac --Xcc=-O2 --pkg gtk+-2.0 snapz.vala -o snapz
