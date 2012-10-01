#include "QtKeygrab.h"

#include <iostream>
#include "X11UtilizableUtils/keystroke.h"

Keygrabber::Keygrabber() {
	init_keystrokes();
	//int mods = Mod_Control | Mod_Super | Mod_Alt;
	int mods = Mod_Control | Mod_Alt;
	for (int i = XK_a; i <= XK_z; i++) {
		reg_keystroke(mods, i);
	}
}

void Keygrabber::run() {
	_stop = false;
	int keysym;
	while (!_stop) {
		if (get_keystroke(NULL, &keysym)) {
			emit keyHit(keysym);
		}
	}
}

void Keygrabber::quit() {
	_stop = true;
}
