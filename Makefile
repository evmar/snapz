# I tried to use waf but their examples wouldn't even run.  :(

snapz: opt

.PHONY: debug opt
debug: snapz.vala
	valac -g --save-temps --pkg gtk+-2.0 snapz.vala

opt: snapz.vala
	valac --Xcc=-O2 --pkg gtk+-2.0 snapz.vala
