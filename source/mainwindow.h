#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "fritzbox.h"
#include "preferences.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void enableVB();
	void showAbout();
	void showPreferences();
	void updateStatus();
	void finishedJob();
	void showError(QString);
	void updateAnnouncements();
	void updateAll();
	void saveNewAnnouncement();
	void addNewAnnouncement();
	void setAnnouncement();
	void initFBox();

private:
	Ui::MainWindow *ui;
	QSettings* settings;
	Fritzbox fBox;
	Preferences* preferences;

	void showWaiting(bool);

};

#endif
