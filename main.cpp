#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>

#include "TestObject.hpp"

class FunctorEvent : public QEvent
{
public:
	FunctorEvent(std::function<void ()> f)
		: QEvent(static_cast<QEvent::Type>(TypeId))
		, m_functor(f)
	{
	}
	
	static const int TypeId = 4567;
	
	void run()
	{
		m_functor();
	}
	
private:
	std::function<void ()> m_functor;
};

class FunctorRunner : public QObject
{
public:
	FunctorRunner()
		: QObject(nullptr)
	{
	}
	
	bool event(QEvent * e) override
	{
		if (e->type() == FunctorEvent::TypeId) {
			auto runner = dynamic_cast<FunctorEvent*>(e);
			qDebug() << "FunctorRunner::event - running functor";
			runner->run();
			qDebug() << "FunctorRunner::event - finished running functor";
			return true;
		}
		return QObject::event(e);
	}
};

void runQCoreApplication(std::promise<QCoreApplication*> * promisedApp, std::promise<FunctorRunner*> * promisedRunner)
{
	qDebug() << "creating qapp";
	
	static int argc = 1;
	static char * argv = "qt-sub-application";
	auto app_ = new QCoreApplication(argc, &argv);
	promisedApp->set_value(app_);
	
	auto runner = new FunctorRunner;
	QObject::connect(app_, SIGNAL(aboutToQuit()), runner, SLOT(deleteLater()));
	promisedRunner->set_value(runner);
	
	qDebug() << "calling qapp->exec()";
	app_->exec();
	qDebug() << "finished qapp->exec()";
}

void mainEventLoop()
{
	int count = 0;
	const int numIterations = 30;
	
	std::thread qCoreApp;
	std::promise<QCoreApplication*> promisedApp;
	auto futureApp = promisedApp.get_future();
	QCoreApplication * app = nullptr;
	
	std::promise<FunctorRunner*> promisedRunner;
	auto futureRunner = promisedRunner.get_future();
	
	qDebug() << "started main application 'other' event loop";
	
	while (count++ < numIterations) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		
		// At some point in the parent app's lifetime, it launches another thread containing
		// the QCoreApplication, along with a few objects doing various things.  This is a
		// long-lived thread.
		if (count == 2) {
			qCoreApp = std::thread(runQCoreApplication, &promisedApp, &promisedRunner);
		}
		else if (count == 4) {
			app = futureApp.get();
			auto runner = futureRunner.get();
			
			auto createObjectWithTimer = [app]()
				{
					auto obj = new TestObject;
					QObject::connect(app, SIGNAL(aboutToQuit()), obj, SLOT(deleteLater()));
				};
			app->postEvent(runner, new FunctorEvent(createObjectWithTimer));
		}
	}
	
	// Before the main application finishes, it will ask the qcoreapp to shut down.
	app->quit();
	qCoreApp.join();
	
	std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main(int argc, char* argv[])
{
	// The parent application has its own event loop (Tcl/Tk, if that matters).
	auto parentApplication = std::thread(mainEventLoop);
	
	
	parentApplication.join();
}
