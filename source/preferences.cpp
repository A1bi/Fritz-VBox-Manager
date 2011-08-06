#include "preferences.h"

Preferences::Preferences(Fritzbox* f, QSettings* s, QWidget *parent) : QMainWindow(parent), ui(new Ui::Preferences), settings(s), fBox(f), modified(false) {
	ui->setupUi(this);

	ui->actionGeneral->setChecked(true);
	ui->hostname->setText(settings->value("host", "fritz.box").toString());

	connect(fBox, SIGNAL(finishedJob()), this, SLOT(finishTest()));
}

void Preferences::saveChanges() {
	if (sender() == ui->hostname || sender() == ui->passwd) {
		QLineEdit* lEdit = (QLineEdit*) sender();
		QString value = lEdit->text();
		if (value != "" && value != settings->value(sender() == ui->hostname ? "host" : "passwd")) {
			modified = true;

			if (sender() == ui->hostname) {
				fBox->setHost(value);
			} else {
				fBox->setPasswd(value);
			}
		}

	} else if (this->sender() == ui->VBbox) {
		modified = true;
	}
}

void Preferences::closeEvent(QCloseEvent* event) {
	if (modified) {

		if (ui->VBbox->currentIndex() > 0) {
			for (Fritzbox::FBVBoxes::iterator iter = vboxesCache.begin(); iter != vboxesCache.end(); iter++) {
				if (iter->second == ui->VBbox->currentText()) {
					settings->setValue("tamId", iter->first);
				}
			}
		}

		settings->setValue("host", ui->hostname->text());

		emit settingsModified();
	}

	event->accept();
}

void Preferences::testConnection() {
	ui->centralwidget->setEnabled(false);

	fBox->init();
}

void Preferences::finishTest() {
	vboxesCache = fBox->getVBoxes();

	ui->VBbox->clear();
	ui->VBbox->addItem("-- Bitte auswählen --");
	for (Fritzbox::FBVBoxes::iterator iter = vboxesCache.begin(); iter != vboxesCache.end(); iter++) {
		ui->VBbox->addItem(iter->second);
		if (iter->first == settings->value("tamId")) {
			ui->VBbox->setCurrentIndex(ui->VBbox->count()-1);
		}
	}
	ui->VBbox->setEnabled(true);
	ui->centralwidget->setEnabled(true);
}
