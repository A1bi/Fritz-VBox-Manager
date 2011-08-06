#include <QRegExp>
#include <QDebug>
#include <QCryptographicHash>
#include <QtXml/QDomDocument>

#include "fritzbox.h"


void Fritzbox::init() {
	tamType = 0;
	getPageContent("index_inhalt.html");
	connect(reply, SIGNAL(finished()), this, SLOT(finishAuth()));

	loginFtp();
	connect(&ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(finishFtpJob(int, bool)));
}

void Fritzbox::finishAuth() {
	// extract session id for authentication
	QString page(reply->readAll());
	destroyReply();

	QRegExp rx;
	rx.setPattern("<input type=\"hidden\" name=\"sid\" value=\"([a-zA-Z0-9]{16})\"");

	if (rx.indexIn(page) > -1) {
		sId = rx.cap(1);

	} else {
		emit error("Unbekannter Fehler");
	}

	retrieveSettings();
}

void Fritzbox::finishGlobalJob() {
	if (ftpFinished && webFinished) {
		emit finishedJob();
	}
}

void Fritzbox::update() {
	webFinished = false;
	ftpFinished = false;

	retrieveSettings();
	updateCurAnnouncementHash();
}

void Fritzbox::emitError(QString msg) {
	emit error(msg);
}

void Fritzbox::enableVB(bool enabled) {
	queryItemsList list;
	list.push_back(queryItem("Active", (enabled) ? "1" : "0"));

	modifySettings(list);
}

QUrl Fritzbox::getUrl(QString getPage) {
	QUrl url("http://" + host);
	url.setPath("/cgi-bin/webcm");
	url.addQueryItem("sid", sId);
	url.addQueryItem("getpage", "../html/" + getPage);
	url.addQueryItem("var:pagename", "fondevices");
	url.addQueryItem("var:menu", "home");

	return url;
}

void Fritzbox::getPageContent(QString getPage) {
	// prepare url
	QUrl url = getUrl(getPage);

	// request
	QNetworkRequest request(url);
	// execute
	reply = network.get(request);
}

void Fritzbox::retrieveSettings() {
	// execute
	getPageContent();
	connect(reply, SIGNAL(finished()), this, SLOT(parseSettings()));
}

void Fritzbox::modifySettings(queryItemsList list) {
	// prepare post data
	QByteArray data;
	for (queryItemsList::iterator iter = list.begin(); iter != list.end(); iter++) {
		data.append("tam:settings/TAM" + QString::number(tamId) + "/" + iter->first + "=" + iter->second + "&");
	}

	//additional stuff
	QUrl url = getUrl();
	data.append(url.encodedQuery());

	// request
	QNetworkRequest request(url);
	// execute
	reply = network.post(request, data);
	connect(reply, SIGNAL(finished()), this, SLOT(parseSettings()));

}

void Fritzbox::parseSettings() {
	// extract session id for authentication
	pageCache = reply->readAll();

	QRegExp rx;
	rx.setPattern("<input type=\"hidden\" name=\"tam:settings/TAM" + QString::number(tamId) + "/([a-zA-z]{1,15})\" value=\"([a-zA-Z0-9.-_ ]{1,50})\"");

	int pos = 0;
	while ((pos = rx.indexIn(pageCache, pos)) != -1) {
		QString value = rx.cap(2);
		QString key = rx.cap(1);

		// get name and enabled setting
		if (key == "Name") {
			VBname = value;
		} else if (key == "Active") {
			VBenabled = (value == "1") ? true : false;
		}

		pos += rx.matchedLength();
	}

	emit updatedSettings();

	if (!webFinished) {
		webFinished = true;
		finishGlobalJob();
	} else {
		emit finishedJob();
	}

}

Fritzbox::FBVBoxes Fritzbox::getVBoxes() {

	FBVBoxes vboxes;

	QRegExp rx, rx2;
	rx.setPattern("<input type=\"hidden\" name=\"tam:settings/TAM([0-9])/Display\" value=\"1\" id=\"uiPostDisplay([0-9])\" disabled />");

	int pos = 0;
	while ((pos = rx.indexIn(pageCache, pos)) != -1) {
		QString id = rx.cap(1);

		// get name
		rx2.setPattern("<input type=\"hidden\" name=\"tam:settings/TAM" + id + "/Name\" value=\"([a-zA-Z0-9.-_ ]{1,50})\" id=\"uiPostName" + id + "\" disabled/>");
		qDebug() << rx2.indexIn(pageCache);

		// create vbox
		vboxes.push_back(pair<int, QString>(id.toInt(), rx2.cap(1)));

		pos += rx.matchedLength();
	}

	return vboxes;

}

void Fritzbox::destroyReply() {
	reply->deleteLater();
	reply = 0;
}

QString Fritzbox::getCurAnnouncementFilename() {
	return "uvp." + QString::number(tamId) + ".00" + QString::number(tamType);
}

void Fritzbox::deleteCurAnnouncement() {
	ftp.remove(getCurAnnouncementFilename());
}

void Fritzbox::loginFtp() {
	if (ftp.LoggedIn) {
	//	ftp.close();
	}
	ftp.connectToHost(host);
	lastFtpJob.first = ftp.login("ftpuser", passwd);
	lastFtpJob.second = FTPconnect;
}

void Fritzbox::finishFtpJob(int id, bool errorId) {
		if (lastFtpJob.first != id) return;

		switch (lastFtpJob.second) {

			case FTPconnect:
				if (errorId) {
					emit error("Authentifizierung fehlgeschlagen.");
					break;
				}
				// set default dir
				lastFtpJob.first = ftp.list("/");
				lastFtpJob.second = FTPdefaultDir;
				connect(&ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(finishFtpList(QUrlInfo)));
				break;

			case FTPenterVoicebox:
				if (errorId) {
					emit error("Die vorliegende Dateistruktur entspricht nicht der einer Fritzbox.");
					break;
				}
				lastFtpJob.first = ftp.cd("announcements");
				lastFtpJob.second = FTPcheckAnnouncementsDir;
				break;

			case FTPcheckAnnouncementsDir:
				if (errorId) {
					ftp.mkdir("announcements");
				}
				lastFtpJob.first = ftp.get(getXMLfilename());
				lastFtpJob.second = FTPloadAnnouncementsXML;
				break;

			case FTPloadAnnouncementsXML: {
				if (!errorId) {
					QDomDocument announcementsXML;
					announcementsXML.setContent(ftp.readAll());
					QDomNodeList announcementList = announcementsXML.elementsByTagName("item");
					for (int i = 0; i < announcementList.size(); i++) {
						QDomNode announcement = announcementList.at(i);
						announcements.push_back(pair<QString, QString>(announcement.firstChildElement("hash").text(), announcement.firstChildElement("name").text()));
					}
				}
				ftp.cd("..");
				updateCurAnnouncementHash();
				break;
			}

			case FTPupdateHash: {
				if (errorId) {
					if (++tamType > 2) {
						emit error("Es wurde keine aufgezeichnete Ansage gefunden.");
					} else {
						updateCurAnnouncementHash();
					}
					break;
				}
				QByteArray data(ftp.readAll());
				curAnnouncementHash = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex().constData();
				emit updatedAnnouncement();

				if (!ftpFinished) {
					ftpFinished = true;
					finishGlobalJob();
				}
				break;
			}

			case FTPgetNew: {
				QByteArray data(ftp.readAll());
				ftp.cd("announcements");
				lastFtpJob.first = ftp.put(data, curAnnouncementHash);
				lastFtpJob.second = FTPsaveNew;
				break;
			}

			case FTPsaveNew:
				if (errorId) {
					emit error("Konnte Ansage nicht speichern.");
					announcements.pop_back();
				} else {
					saveXMLfile();
				}
				emit savedNewAnnouncement();
				break;

			case FTPloadAnnouncement: {
				QByteArray data(ftp.readAll());
				ftp.cd("..");
				deleteCurAnnouncement();
				lastFtpJob.first = ftp.put(data, getCurAnnouncementFilename());
				lastFtpJob.second = FTPsaveAnnouncement;
				break;
			}

			case FTPsaveAnnouncement:
				emit updatedAnnouncement();
				emit finishedJob();
				break;

			case FTPsaveAnnouncementsXML:
				ftp.cd("..");
				break;

			default:
				break;

		}

}

void Fritzbox::updateCurAnnouncementHash() {
	// read current announcement file
	lastFtpJob.first = ftp.get(getCurAnnouncementFilename());
	lastFtpJob.second = FTPupdateHash;
}

void Fritzbox::finishFtpList(QUrlInfo url) {
	if (lastFtpJob.second == FTPdefaultDir) {
		lastFtpJob.first = ftp.cd(url.name() + "/FRITZ/voicebox");
		lastFtpJob.second = FTPenterVoicebox;
	}
}

void Fritzbox::saveNewAnnouncement(QString name) {
	announcements.push_back(pair<QString, QString>(curAnnouncementHash, name));

	lastFtpJob.first = ftp.get(getCurAnnouncementFilename());
	lastFtpJob.second = FTPgetNew;
}

void Fritzbox::setAnnouncement(int index) {
	curAnnouncementHash = announcements.at(index).first;
	ftp.cd("announcements");
	lastFtpJob.first = ftp.get(curAnnouncementHash);
	lastFtpJob.second = FTPloadAnnouncement;
}

QString Fritzbox::getXMLfilename() {
	return "announcements" + QString::number(tamId) + ".xml";
}

void Fritzbox::saveXMLfile() {
	QDomDocument xml;
	QDomElement root = xml.createElement("announcements");
	xml.appendChild(root);

	for (FBannouncements::iterator iter = announcements.begin(); iter != announcements.end(); iter++) {
		QDomElement ann = xml.createElement("item");
		root.appendChild(ann);

		QDomElement hash = xml.createElement("hash");
		ann.appendChild(hash);
		QDomText hashText = xml.createTextNode(iter->first);
		hash.appendChild(hashText);

		QDomElement name = xml.createElement("name");
		ann.appendChild(name);
		QDomText nameText = xml.createTextNode(iter->second);
		name.appendChild(nameText);
	}

	lastFtpJob.first = ftp.put(xml.toByteArray(), getXMLfilename());
	lastFtpJob.second = FTPsaveAnnouncementsXML;
}
