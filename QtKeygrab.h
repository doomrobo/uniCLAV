#ifndef QTKEYGRAB_H
#define QTKEYGRAB_H

#include <QThread>

class Keygrabber : public QThread {

	Q_OBJECT

private:
	bool _stop;
public:
	Keygrabber();
	void run();

public slots:
	void quit();

signals:
	void keyHit(int);
};

#endif /*QTKEYGRAB_H*/
