/*
	uniCLAV
	Copyright (c) 2011 Michael Rosenberg

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/
#include <QWidget>
#include <QMap>
#include <QList>
#include <QChar>
#include <QPushButton>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>

#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "popup.h"

extern char keysymToChar(int);

#define letter(x) QString::fromWCharArray(L###x)

// Thank you, Adam Pierce: http://www.doctort.org/adam
XKeyEvent createKeyEvent(Display *display, Window &win,
                           Window &winRoot, bool press,
                           int keysym, int modifiers)
{
   XKeyEvent event;

   event.display     = display;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = true;
   //event.keycode   = XKeysymToKeycode(display, keysym);
   event.keycode 	 = keysym;
   //std::cout << "Keycode is " << event.keycode << std::endl;
   event.state       = modifiers;

   if(press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}

void center(QWidget *widget)
{
  int x, y;
  int screenWidth;
  int screenHeight;

  const int WIDTH = widget->geometry().width();
  const int HEIGHT = widget->geometry().height();

  QDesktopWidget *desktop = QApplication::desktop();

  screenWidth = desktop->width();
  screenHeight = desktop->height();

  x = (screenWidth - WIDTH) / 2;
  y = (screenHeight - HEIGHT) / 2;

  widget->setGeometry(x, y, WIDTH, HEIGHT);
}




Popup::Popup(QWidget *parent) {
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	grid = new QGridLayout(this);
	grid->setSpacing(2);
	loadChars(":/chars.txt");

	connect(&grab, SIGNAL(keyHit(int)), this, SLOT(handleKeystroke(int)));
	connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)),
			this, SLOT(unFocused(QWidget*, QWidget*)));
	grab.start();
}

/*bool Popup::event(QEvent *ev)
{
    if (ev->type() == QEvent::Show) {
       grabKeyboard();
    }
    return QWidget::event(ev);
}*/

void Popup::clearLayout() {
	QLayoutItem *item;
	while ((item = grid->takeAt(0))) {
		QWidget *button = item->widget();
		button->close();
		grid->removeWidget(button);
	}
}

void Popup::unFocused(QWidget *old, QWidget *now) {
	if (now == 0) {
		std::cout << "Lost focus!\n";
		grab.start();
		hide();
	} else {
		std::cout << "Gained focus!\n";
	}
}

void Popup::keyPressEvent(QKeyEvent *event){
	if (event->key() == Qt::Key_Shift) {
		//std::cout << "currentChar changed from '" << currentChar;
		currentChar -= 32; // Make it capital
		//std::cout << "' to '" << currentChar << "'\n";
		open();
	}
}

void Popup::keyReleaseEvent(QKeyEvent *event){
	if (event->key() == Qt::Key_Shift) {
		if (currentChar < 97)
			currentChar += 32; // Make it lowercase
		open();
	} else if (event->key() == Qt::Key_Escape) {
		//releaseKeyboard();
		hide();
		grab.start();
	}
}

void Popup::setChar(char ch) {
	currentChar = ch;
	std::cout << "setChar: " << currentChar << std::endl;
}

int Popup::loadChars(const char *filename) {
	char letter = 'a';
	bool cap = false;
	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly)) {
		return 1;
	}
	QTextStream in(&file);
	QString line = in.readLine();

	while (!line.isNull()) {
		if (line.length() == 0) {
			if (cap) {
				letter += 33; // Make lower-case + 1
				cap = false;
			}
			else {
				letter -= 32; // Make upper-case + 1
				cap = true;
			}
		} else {
			std::cout << "Loaded " << qPrintable(line) << std::endl;
			std::cout << "Added to " << letter << std::endl;
			charMap[letter] << line.trimmed();
		}

		if (letter > 'z')
			break;

		line = in.readLine();
	}

	file.close();
	return 0;
}

void Popup::open(void) {
	if (charMap.contains(currentChar)) {
		std::cout << "Contains!\n";

		grab.quit();

		clearLayout();
		int i = 0, j = 0;
		for (QList<QString>::iterator it = charMap[currentChar].begin();
			 it != charMap[currentChar].end(); it++) {

			QPushButton *btn = new QPushButton(*it, this);
			connect(btn, SIGNAL(clicked()), this, SLOT(buttonHandler()));
			grid->addWidget(btn, i, j);
			ushort t = (*it)[0].unicode();
			std::cout << "Added: " << t << std::endl;
			if (j == 4) {
				j = 0;
				i++;
			} else {
				j++;
			}
		}

		setLayout(grid);
		show();
		qApp->setActiveWindow(this);
		//activateWindow();
		center(this);
	}
}

void Popup::buttonHandler(void) {
	//releaseKeyboard();
	hide();
	QPushButton *btn = (QPushButton*)sender();

	char uni_str[6];
	sprintf(uni_str, "U%04x", btn->text()[0].unicode()); // The accepted format for
														 // XStringToKeysym()
	std::cout << "Unicode string: " << uni_str << std::endl;

	Display *dis = XOpenDisplay(0);
	if (dis == NULL)
		return;

	Window root = XDefaultRootWindow(dis);

	Window focus;
	int revert;
	//without the loop, focus is often 1 (which is impossible)
	do {
		XGetInputFocus(dis, &focus, &revert);
	} while (focus == 1);

	KeySym uni_keysym = XStringToKeysym(uni_str);
	KeySym syms[5] = {uni_keysym, uni_keysym, uni_keysym, uni_keysym, uni_keysym};
	XChangeKeyboardMapping(dis, 254, 5, syms, 1);

	XKeyEvent u = createKeyEvent(dis, focus, root, true, 254, 0);
	XSendEvent(dis, focus, True, KeyPressMask, (XEvent*)&u);
	u.type = KeyRelease; // Press and release the unicode key
	XSendEvent(dis, focus, True, KeyPressMask, (XEvent*)&u);

	XCloseDisplay(dis);
	grab.start();
}

void Popup::handleKeystroke(int keysym) {
	std::cerr << "Got keystroke!\n";
	setChar(keysymToChar(keysym));
	open();
}
