#include <QApplication>
#include <unistd.h>
#include "popup.h"


int main(int argc, char* argv[]) {
        QApplication app(argc, argv);
		app.setStyleSheet("QPushButton { font: 20pt }");
        Popup *d = new Popup();
		d->setChar('a');
		//d->open();
        return app.exec();
}
