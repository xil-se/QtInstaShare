#include "uploaddialog.h"
#include "urldialog.h"

#include <QNetworkRequest>
#include <QSystemTrayIcon>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QApplication>
#include <QFileDialog>
#include <QJsonObject>
#include <QMessageBox>
#include <QClipboard>
#include <QHttpPart>
#include <QMimeData>
#include <QBuffer>
#include <QScreen>
#include <QTimer>
#include <QFile>
#include <QMenu>

static UploadDialog *progressDialog = nullptr; // can't create QWidgets before QApplication
static QNetworkAccessManager networkMgr;
static QTimer progressTimer;
static QString secretWord;
static QString uploadUrl;

static const int timeoutMs = 5000;

static bool loadConfig()
{
	QFile configFile(qApp->applicationDirPath() + "/config.json");
	if(!configFile.exists()) {
		QMessageBox::critical(nullptr, QObject::tr("Epic fail"), QObject::tr("Config file (config.json) not found!"));
		return false;
	}

	if(!configFile.open(QFile::ReadOnly)) {
		QMessageBox::critical(nullptr, QObject::tr("Epic fail"), QObject::tr("Can't open config file (config.json)!"));
		return false;
	}

	const QJsonDocument config = QJsonDocument::fromJson(configFile.readAll());
	secretWord = config.object().value("secret").toString();
	uploadUrl  = config.object().value("url").toString();

	configFile.close();

	if(secretWord.isEmpty() || uploadUrl.isEmpty()) {
		QMessageBox::critical(nullptr, QObject::tr("Epic fail"), QObject::tr("You need to fill in the values in the config file (config.json)!"));
		return false;
	}

	return true;
}

static QNetworkReply *uploadPayload(const QHttpPart &payload, const qint64 &size)
{
	const QNetworkRequest request(uploadUrl);
	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	QHttpPart secretPart;
	secretPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"secret\""));
	secretPart.setBody(secretWord.toLatin1());

	multiPart->append(secretPart);
	multiPart->append(payload);

	QNetworkReply *reply = networkMgr.post(request, multiPart);
	QObject::connect(reply, &QNetworkReply::uploadProgress, [](qint64 bytes, qint64 total)
	{
		progressDialog->setValue(bytes);
		progressDialog->setMaximum(total);
	});
	QObject::connect(reply, &QNetworkReply::readyRead, [reply]()
	{
		progressTimer.stop();
		progressDialog->hide();

		const QByteArray data = reply->readAll();

		UrlDialog *urlDialog = new UrlDialog(data);
		QObject::connect(urlDialog, &QDialog::accepted, urlDialog, &QObject::deleteLater);

		urlDialog->setWindowTitle(QObject::tr("Your link"));
		urlDialog->show();

		reply->deleteLater();
	});
	multiPart->setParent(reply);

	// if the upload takes longer than timeoutMs, a progress dialog will be shown
	progressDialog->setValue(0);
	progressDialog->setMaximum(size);
	progressTimer.start(timeoutMs);

	return reply;
}

static void uploadFile(const QString &fileName)
{
	if(fileName.isEmpty())
		return;

	QFile *file = new QFile(fileName);
	if(!file->open(QIODevice::ReadOnly)) {
		QMessageBox::warning(nullptr, QObject::tr("Kinda fail"), QObject::tr("Failed to open file: %1").arg(file->errorString()));
		return;
	}

	QHttpPart payload;
	payload.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userfile\"; filename=\"file\""));
	payload.setBodyDevice(file);

	QNetworkReply *reply = uploadPayload(payload, file->size());
	file->setParent(reply);
}

static void uploadImage(const QImage &image)
{
	QBuffer *buffer = new QBuffer();
	buffer->open(QIODevice::ReadWrite);
	image.save(buffer, "PNG");

	buffer->seek(0);

	QHttpPart payload;
	payload.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userfile\"; filename=\"image\""));
	payload.setBodyDevice(buffer);

	QNetworkReply *reply = uploadPayload(payload, buffer->size());
	buffer->setParent(reply);
}

static void uploadClipboard()
{
	const QClipboard *clipboard = qApp->clipboard();
	const QMimeData *mimeData = clipboard->mimeData();

	if(mimeData->hasImage()) {
		uploadImage(clipboard->image());
		return;
	}

	if(mimeData->hasText()) {
		const QByteArray data = clipboard->text().toLatin1();

		QHttpPart payload;
		payload.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userfile\"; filename=\"clipboard\""));
		payload.setBody(data);

		uploadPayload(payload, data.size());
		return;
	}

	if(mimeData->hasUrls()) {
		// only upload first local file we find
		QString fileName;
		for(int i = 0; i < mimeData->urls().size(); ++i) {
			if(!mimeData->urls().at(i).isLocalFile())
				continue;

			fileName = mimeData->urls().at(i).toLocalFile();
			break;
		}

		if(fileName.isEmpty()) {
			QMessageBox::warning(nullptr, QObject::tr("Kinda fail"), QObject::tr("Couldn't find any local files in clipboard."));
			return;
		}

		uploadFile(fileName);
		return;
	}

	QMessageBox::warning(nullptr, QObject::tr("Programmer fail"), QObject::tr("Could not recognize clipboard contents.\nTell the programmer to support \"%1\".").arg(mimeData->formats().join(',')));
}

static void uploadScreenshot(const int &screenId = 0)
{
	const QList<QScreen*> screens = QApplication::screens();

	qDebug() << __func__ << screenId;
	if(screens.size() - 1 < screenId)
		return;

	if(!screens[screenId]) {
		QMessageBox::critical(nullptr, QObject::tr("Epic fail"), QObject::tr("Failed to fetch your screen."));
		return;
	}

	const QRect geometry = screens[screenId]->geometry();
	const QPixmap screenshot = screens[screenId]->grabWindow(0, geometry.x(), geometry.y(), geometry.width(), geometry.height());
	uploadImage(screenshot.toImage());
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("InstaShare");
	app.setOrganizationName("xil.se");
	app.setWindowIcon(QIcon(":/xil.svg"));
	app.setQuitOnLastWindowClosed(false);

	// sanity
	if(!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, QObject::tr("Epic fail"), QObject::tr("You don't have a system tray!"));
		return -1;
	}

	if(!loadConfig())
		return -1;

	// setup progress stuff
	progressDialog = new UploadDialog();
	progressDialog->hide();
	QObject::connect(&app, &QApplication::aboutToQuit, progressDialog, &QObject::deleteLater);

	progressTimer.setSingleShot(true);
	QObject::connect(&progressTimer, &QTimer::timeout, progressDialog, &QWidget::show);

	// setup context menu
	QMenu contextMenu;
	QAction *clipboardAction = contextMenu.addAction(QIcon(":/clipboard.png"), QObject::tr("Clipboard"));
	QObject::connect(clipboardAction, &QAction::triggered, uploadClipboard);

	if(app.screens().size() == 1) {
		QAction *screenshotAction = contextMenu.addAction(QIcon(":/monitor.png"), QObject::tr("Screenshot"));
		QObject::connect(screenshotAction, &QAction::triggered, uploadScreenshot);
	} else {
		QMenu *screenShotMenu = contextMenu.addMenu(QIcon(":/monitors.png"), QObject::tr("Screenshot"));
		for(int i = 0; i < app.screens().size(); ++i) {
			QAction *screenshotAction = screenShotMenu->addAction(QIcon(":/monitor.png"), QString("Screen #%1").arg(i + 1));
			QObject::connect(screenshotAction, &QAction::triggered, [i]()
			{
				uploadScreenshot(i);
			});
		}
	}

	QAction *fileAction = contextMenu.addAction(QIcon(":/file.png"), QObject::tr("File"));
	QObject::connect(fileAction, &QAction::triggered, []()
	{
		const QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("File to upload"));
		uploadFile(fileName);
	});
	contextMenu.addSeparator();
	QAction *quitAction = contextMenu.addAction(QObject::tr("Quit"));
	QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);

	// setup tray icon
	QSystemTrayIcon trayIcon(QIcon(":/xil.svg"));
	trayIcon.setContextMenu(&contextMenu);
	trayIcon.show();

	return app.exec();
}
