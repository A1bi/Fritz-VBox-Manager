#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QMainWindow>
#include <QSettings>
#include <QCloseEvent>

#include "ui_preferences.h"
#include "fritzbox.h"

namespace Ui {
	class Preferences;
}

class Preferences : public QMainWindow {
	Q_OBJECT

public:
	explicit Preferences(Fritzbox*, QSettings*, QWidget *parent = 0);
	~Preferences() {
		delete ui;
	}

private:
	Ui::Preferences *ui;
	QSettings* settings;
	Fritzbox* fBox;
	bool modified;
	Fritzbox::FBVBoxes vboxesCache;

	void closeEvent(QCloseEvent*);

private slots:
	void saveChanges();
	void testConnection();
	void finishTest();

signals:
	void settingsModified();

};

#endif // PREFERENCES_H
