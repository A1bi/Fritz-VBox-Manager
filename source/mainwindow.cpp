#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

#include "aboutdialog.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);

	ui->progressWidget->setVisible(false);
	ui->newAnnounceBox->setVisible(false);

	settings = new QSettings;
	preferences = new Preferences(&fBox, settings);

// #ifdef Q_WS_MAC
//    // Mac OS X specific things

//    // dock menu on Mac
//    QMenu* dockMenu = new QMenu;
//    // add info items
//    QAction* status = new QAction("Status: deaktiviert", this);
//    status->setDisabled(true);
//    dockMenu->addAction(status);

//    // add to dock
//    extern void qt_mac_set_dock_menu(QMenu*);
//    qt_mac_set_dock_menu(dockMenu);
//#endif

	fBox.setHost(settings->value("host", "fritz.box").toString());
	initFBox();

	connect(&fBox, SIGNAL(updatedSettings()), this, SLOT(updateStatus()));
	connect(&fBox, SIGNAL(updatedAnnouncement()), this, SLOT(updateAnnouncements()));
	connect(&fBox, SIGNAL(savedNewAnnouncement()), this, SLOT(addNewAnnouncement()));
	connect(&fBox, SIGNAL(finishedJob()), this, SLOT(finishedJob()));
	connect(&fBox, SIGNAL(error(QString)), this, SLOT(showError(QString)));
	connect(preferences, SIGNAL(settingsModified()), this, SLOT(initFBox()));

}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::initFBox() {
	showWaiting(true);

	fBox.setTamId(settings->value("tamId", 0).toInt());
	fBox.init();
}

void MainWindow::showWaiting(bool show) {
	ui->actionWidget->setDisabled(show);
	ui->toolBar->setDisabled(show);
	ui->progressWidget->setVisible(show);
}

void MainWindow::enableVB() {
	showWaiting(true);

	fBox.enableVB((this->sender() == ui->actionEnable));
}

void MainWindow::updateAnnouncements() {
	QString cur = fBox.getCurAnnouncementHash();
	bool newAnnouncement = true;
	ui->announcementBox->clear();
	Fritzbox::FBannouncements announcements = fBox.getAnnouncements();

	if (announcements.size() < 1) {
		ui->announcementBox->addItem("Keine Ansagen verfügbar");
		ui->announcementBox->setDisabled(true);

	} else {
		ui->announcementBox->setDisabled(false);
		for (Fritzbox::FBannouncements::iterator iter = announcements.begin(); iter != announcements.end(); iter++) {
			ui->announcementBox->addItem(iter->second);
			if (iter->first == cur) {
				ui->announcementBox->setCurrentIndex(ui->announcementBox->count()-1);
				newAnnouncement = false;
			}
		}
	}

	if (newAnnouncement) {
		ui->newAnnounceBox->setVisible(true);
	}

}

void MainWindow::addNewAnnouncement() {
	updateAnnouncements();

	ui->newAnnounceBox->setVisible(false);
	showWaiting(false);
}

void MainWindow::updateAll() {
	showWaiting(true);
	fBox.update();
}

void MainWindow::updateStatus() {
	bool enabled = fBox.getVBEnabled();
	ui->actionEnable->setDisabled(enabled);
	ui->actionDisable->setDisabled(!enabled);

	ui->nameLabel->setText(fBox.getVBName());
	ui->enabledLabel->setText((fBox.getVBEnabled()) ? "ja" : "nein");
}

void MainWindow::saveNewAnnouncement() {
	showWaiting(true);
	fBox.saveNewAnnouncement(ui->newAnnounceName->text());
}

void MainWindow::setAnnouncement() {
	ui->newAnnounceBox->setVisible(false);
	showWaiting(true);

	fBox.setAnnouncement(ui->announcementBox->currentIndex());
}

void MainWindow::finishedJob() {
	showWaiting(false);
}

void MainWindow::showAbout() {
	AboutDialog about;
	about.exec();
}

void MainWindow::showPreferences() {
	preferences->show();
}

void MainWindow::showError(QString msg) {
	QMessageBox msgBox(this);
	msgBox.setText("Es ist ein Fehler aufgetreten!");
	msgBox.setInformativeText("Fehler: " + msg + "\nBitte überprüfen Sie Ihre Einstellungen.");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.setWindowFlags(Qt::Sheet);
	msgBox.exec();

	//showPreferences();
}
