#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QMap>
#include <QList>
#include <QThread>

#include "QtKeygrab.h"

class QChar;
class QKeyEvent;
class QGridLayout;

class Popup : public QWidget {

	Q_OBJECT

private:
	char currentChar;
	QMap<char, QList<QString> > charMap;

	Keygrabber grab;

	void clearLayout();
	//bool event(QEvent *);

	QGridLayout *grid;

public:
	Popup(QWidget *parent = 0);
	int loadChars(const char *);
	void setChar(char);

public slots:
	void open(void);

private slots:
	void buttonHandler(void);
	void handleKeystroke(int);
	void unFocused(QWidget *, QWidget *);
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
};

#endif /* DIALOG_H */
