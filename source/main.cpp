#include <QApplication>
#include <QTextCodec>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Albisigns");
	QCoreApplication::setOrganizationDomain("albisigns.de");
	QCoreApplication::setApplicationName("Fritz VBox Manager");
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	MainWindow w;
	w.show();

	return a.exec();
}
