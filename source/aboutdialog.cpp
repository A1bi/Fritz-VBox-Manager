#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);
	ui->version->setText(ui->version->text().arg("0.8"));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
