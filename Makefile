# I tried to use waf but their examples wouldn't even run.  :(

snapz: snapz.vala
	valac --pkg gtk+-2.0 snapz.vala

.PHONY: debug
debug: snapz.vala
	valac -g --pkg gtk+-2.0 snapz.vala
