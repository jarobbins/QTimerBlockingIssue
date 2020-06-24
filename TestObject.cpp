#include "TestObject.hpp"
#include <QDebug>
#include <QTimer>

TestObject::TestObject()
	: QObject(nullptr)
{
	qDebug() << "creating timer";
	auto t = new QTimer(this);
	qDebug() << "finished creating timer";
	t->setInterval(1000);
	connect(t, &QTimer::timeout, [](){ qDebug() << "timer callback"; });
	t->start();
	qDebug() << "started timer";
}
