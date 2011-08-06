#ifndef FRITZBOX_H
#define FRITZBOX_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QFtp>
#include <QtNetwork/QNetworkReply>
#include <vector>
using namespace std;


class Fritzbox : public QObject {
	Q_OBJECT

public:
	typedef vector<pair<int, QString> > FBVBoxes;
	typedef vector<pair<QString, QString> > FBannouncements;

	Fritzbox() : tamType(0), ftpFinished(false), webFinished(false) {}
	void init();
	void enableVB(bool);
	void update();
	void saveNewAnnouncement(QString);
	void setAnnouncement(int);
	FBVBoxes getVBoxes();
	void setTamId(int t) {
		tamId = t;
	}
	void setHost(QString a) {
		host = a;
	}
	void setPasswd(QString p) {
		passwd = p;
	}
	QString getVBName() {
		return VBname;
	}
	bool getVBEnabled() {
		return VBenabled;
	}
	QString getCurAnnouncementHash() {
		return curAnnouncementHash;
	}
	FBannouncements getAnnouncements() {
		return announcements;
	}

private:
	typedef pair<QString, QString> queryItem;
	typedef vector<queryItem> queryItemsList;
	enum ftpJob { FTPconnect, FTPdefaultDir, FTPenterVoicebox, FTPcheckAnnouncementsDir, FTPloadAnnouncementsXML, FTPupdateHash, FTPgetNew, FTPsaveNew, FTPloadAnnouncement, FTPsaveAnnouncement, FTPsaveAnnouncementsXML };

	void emitError(QString);
	void getPageContent(QString = "de/menus/menu2.html");
	void destroyReply();
	QUrl getUrl(QString = "de/menus/menu2.html");
	void retrieveSettings();
	void modifySettings(queryItemsList);
	void finishGlobalJob();
	QString getCurAnnouncementFilename();
	void deleteCurAnnouncement();
	void updateCurAnnouncementHash();
	void loginFtp();
	QString getXMLfilename();
	void saveXMLfile();

	QFtp ftp;
	pair<int, ftpJob> lastFtpJob;
	QNetworkAccessManager network;
	QNetworkReply* reply;
	QString host, passwd, sId, VBname, curAnnouncementHash, pageCache;
	int tamId, tamType;
	bool ftpFinished, webFinished, VBenabled;
	FBannouncements announcements;

private slots:
	void finishFtpJob(int, bool);
	void finishFtpList(QUrlInfo);
	void finishAuth();
	void parseSettings();

signals:
	void finishedJob();
	void updatedSettings();
	void updatedAnnouncement();
	void savedNewAnnouncement();
	void error(QString);

};

#endif
