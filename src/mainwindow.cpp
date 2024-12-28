/*****************************************************************************
* Copyright 2015-2019 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "fs/ns/navserver.h"
#include "fs/ns/navservercommon.h"
#include "optionsdialog.h"

#include "settings/settings.h"
#include "gui/dialog.h"
#include "gui/helphandler.h"
#include "gui/widgetstate.h"
#include "logging/logginghandler.h"
#include "logging/loggingguiabort.h"
#include "fs/sc/simconnectreply.h"
#include "fs/sc/datareaderthread.h"
#include "constants.h"
#include "fs/sc/xpconnecthandler.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QActionGroup>
#include <QDir>
#include <QRegularExpression>
#include <QTimer>

using atools::settings::Settings;
using atools::fs::sc::SimConnectData;
using atools::fs::sc::SimConnectReply;
using atools::gui::HelpHandler;

// "master" or "release/1.4"
const QString HELP_BRANCH = "develop/2.5"; // VERSION_NUMBER

/* Important: keep slash at the end. Otherwise Gitbook will not display the page properly */
const QString HELP_ONLINE_URL("https://www.littlenavmap.org/manuals/littlefgconnect/" + HELP_BRANCH + "/${LANG}/");
const QString HELP_OFFLINE_FILE("help/little-fgconnect-user-manual-${LANG}.pdf");

MainWindow::MainWindow()
  : ui(new Ui::MainWindow)
{
  qDebug() << Q_FUNC_INFO;

  aboutMessage =
    QObject::tr("<p>is the FlightGear connection agent for Little Navmap.</p>"
                "<p>This software is licensed under "
                "<a href=\"http://www.gnu.org/licenses/gpl-3.0\">GPL3</a> or any later version.</p>"
                "<p>The source code for this application is available at "
                "<a href=\"https://github.com/albar965\">Github</a>.</p>"
                "<p>More about my projects at "
                "<a href=\"https://www.littlenavmap.org\">www.littlenavmap.org</a>.</p>"
                "<p><b>Copyright 2015-2020 Alexander Barthel, Slawek Mikula</b></p>");

  // Show a dialog on fatal log events like asserts
  atools::logging::LoggingGuiAbortHandler::setGuiAbortFunction(this);

  ui->setupUi(this);

  readSettings();

  // Get the online indicator file which shows which help files are available online
  QString onlineFlagFile = atools::gui::HelpHandler::getHelpFile(
    QString("help") + QDir::separator() + "little-fgconnect-user-manual-${LANG}.online", "en");

  // Extract language from the file
  QRegularExpression regexp("little-fgconnect-user-manual-(.+)\\.online", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match = regexp.match(onlineFlagFile);
  if(match.hasMatch() && !match.captured(1).isEmpty())
    supportedLanguageOnlineHelp = match.captured(1);
  else
    supportedLanguageOnlineHelp = "en";

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();

  // Process the actual command line arguments given by the user
  parser.process(*QCoreApplication::instance());

  // Right align the help button
  QWidget *spacerWidget = new QWidget(ui->toolBar);
  spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ui->toolBar->insertWidget(ui->actionContents, spacerWidget);

  // Bind the log function to this class for category "gui"
  using namespace std::placeholders;
  atools::logging::LoggingHandler::setLogFunction(std::bind(&MainWindow::logGuiMessage, this, _1, _2, _3));

  // Create help handler for managing the Help menu items
  helpHandler = new atools::gui::HelpHandler(this, aboutMessage, GIT_REVISION);

  connect(ui->actionConnectFlightgear, &QAction::triggered, this, &MainWindow::startStopConnection);
  connect(ui->actionQuit, &QAction::triggered, this, &QMainWindow::close);
  connect(ui->actionResetMessages, &QAction::triggered, this, &MainWindow::resetMessages);
  connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::options);
  connect(ui->actionContents, &QAction::triggered, this, &MainWindow::showOnlineHelp);
  connect(ui->actionContentsOffline, &QAction::triggered, this, &MainWindow::showOfflineHelp);
  connect(ui->actionAbout, &QAction::triggered, helpHandler, &atools::gui::HelpHandler::about);
  connect(ui->actionAboutQt, &QAction::triggered, helpHandler, &atools::gui::HelpHandler::aboutQt);

  // Log messages have to be redirected through a message so that QTextEdit::append is not called on
  // a thread context different than main
  connect(this, &MainWindow::appendLogMessage, ui->textEdit, &QTextEdit::append);

  // Once visible start server and log messagess
  connect(this, &MainWindow::windowShown, this, &MainWindow::mainWindowShown, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
  qDebug() << Q_FUNC_INFO;

  dataReader->terminateThread();
  qDebug() << Q_FUNC_INFO << "dataReader terminated";

  delete dataReader;
  qDebug() << Q_FUNC_INFO << "dataReader deleted";

  delete xpConnectHandler;
  qDebug() << Q_FUNC_INFO << "xpConnectHandler deleted";

  atools::logging::LoggingHandler::setLogFunction(nullptr);
  qDebug() << Q_FUNC_INFO << "logging reset";

  delete helpHandler;
  qDebug() << Q_FUNC_INFO << "help handler deleted";

  delete ui;
  qDebug() << Q_FUNC_INFO << "ui deleted";

  atools::logging::LoggingGuiAbortHandler::resetGuiAbortFunction();

  qDebug() << "MainWindow destructor about to shut down logging";
  atools::logging::LoggingHandler::shutdown();
}

void MainWindow::showOnlineHelp()
{
  HelpHandler::openHelpUrlWeb(this, HELP_ONLINE_URL, supportedLanguageOnlineHelp);
}

void MainWindow::showOfflineHelp()
{
  HelpHandler::openHelpUrlFile(this, HelpHandler::getHelpFile(HELP_OFFLINE_FILE, "en" /* override */), "en");
}

void MainWindow::options()
{
  OptionsDialog dialog;

  Settings& settings = Settings::instance();
  unsigned int updateRateMs = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_UPDATE_RATE, 500).toUInt();
  int port = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_DEFAULT_PORT, 7755).toInt();
  bool fetchAiAircraft = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool();
  QString multiplayerServerHost = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_HOST, "mpserver03.flightgear.org").toString();
  int multiplayerServerPort = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_PORT, 5001).toInt();

  dialog.setUpdateRate(updateRateMs);
  dialog.setPort(port);
  dialog.setFetchAiAircraft(fetchAiAircraft);
  dialog.setMultiplayerServerHost(multiplayerServerHost);
  dialog.setMultiplayerServerPort(multiplayerServerPort);

  int result = dialog.exec();

  if(result == QDialog::Accepted)
  {
    settings.setValue(lfgc::SETTINGS_OPTIONS_UPDATE_RATE, static_cast<int>(dialog.getUpdateRate()));
    settings.setValue(lfgc::SETTINGS_OPTIONS_DEFAULT_PORT, dialog.getPort());
    settings.setValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, dialog.isFetchAiAircraft());
    settings.setValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_HOST, dialog.getMultiplayerServerHost());
    settings.setValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_PORT, dialog.getMultiplayerServerPort());

    settings.syncSettings();

    atools::fs::sc::Options options = atools::fs::sc::NO_OPTION;
    if(dialog.isFetchAiAircraft()) {
      options |= atools::fs::sc::FETCH_AI_AIRCRAFT;
    }
    dataReader->setSimconnectOptions(options);

    if(dialog.getUpdateRate() != updateRateMs)
    {
      // Update rate changed - restart data readers
      dataReader->terminateThread();
      dataReader->setUpdateRate(dialog.getUpdateRate());
      dataReader->start();
    }
  }
}

void MainWindow::resetMessages()
{
  Settings& settings = Settings::instance();
  settings.setValue(lfgc::SETTINGS_ACTIONS_SHOW_QUIT, true);
  settings.setValue(lfgc::SETTINGS_ACTIONS_SHOW_PORT_CHANGE, true);
}

void MainWindow::logGuiMessage(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
  if(type == QtFatalMsg)
    // Fatal will look like a crash anyway - bail out to avoid follow up errors
    return;

  if(context.category != nullptr && QString(context.category) == "gui")
  {
    // Define colors
    QString style;
    switch(type)
    {
      case QtDebugMsg:
        style = "color:darkgrey";
        break;
      case QtWarningMsg:
        style = "color:orange;font-weight:bold";
        break;
      case QtFatalMsg:
      case QtCriticalMsg:
        style = "color:red;font-weight:bold";
        break;
      case QtInfoMsg:
        break;
    }

    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd h:mm:ss");
    // Use a signal to update the text edit in the main thread context
    emit appendLogMessage("[" + now + "] <span style=\"" + style + "\">" + message + "</span>");
  }
}

void MainWindow::postLogMessage(QString message, bool warning)
{
  if(warning)
    qWarning(atools::fs::ns::gui).noquote().nospace() << message;
  else
    qInfo(atools::fs::ns::gui).noquote().nospace() << message;
}

void MainWindow::readSettings()
{
  qDebug() << Q_FUNC_INFO;

  verbose = Settings::instance().getAndStoreValue(lfgc::SETTINGS_OPTIONS_VERBOSE, false).toBool();

  atools::gui::WidgetState(lfgc::SETTINGS_MAINWINDOW_WIDGET).restore(this);
}

void MainWindow::writeSettings()
{
  qDebug() << Q_FUNC_INFO;

  atools::gui::WidgetState widgetState(lfgc::SETTINGS_MAINWINDOW_WIDGET);
  widgetState.save(this);
  widgetState.syncSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event);
  // Catch all close events like Ctrl-Q or Menu/Exit or clicking on the
  // close button on the window frame
  qDebug() << Q_FUNC_INFO;

  writeSettings();
}

void MainWindow::startStopConnection()
{
    Settings& settings = Settings::instance();
    int port = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_DEFAULT_PORT, 7755).toInt();
    bool fetchAiAircraft = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool();

    // set main flag
    this->fetchAi = fetchAiAircraft;

    if(udpSocket == nullptr) {
      udpSocket = new QUdpSocket(this);
      if (!udpSocket->bind(port)) {
        qWarning() << Q_FUNC_INFO << "Cannot open UDP port";
        udpSocket = nullptr;
      } else {
        QObject::connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
        qDebug() << Q_FUNC_INFO << "Attached to the UDP port";

        thread = new SharedMemoryWriter();
        thread->start();

        if (fetchAiAircraft) {

            onlineTcpSocket = new QTcpSocket(this);
            connect(onlineTcpSocket, SIGNAL(readyRead()),this, SLOT(tcpSocketReadyRead()));
            connect(onlineTcpSocket, SIGNAL(connected()),this, SLOT(tcpSocketConnected()));
            connect(onlineTcpSocket, SIGNAL(disconnected()),this, SLOT(tcpSocketDisconnected()));

            initOnlineTcpConnection();
        }

        qInfo(atools::fs::ns::gui).noquote().nospace() << "Started FlightGear connection slot. Waiting for FlightGear data.";
      }
    } else {
      if (udpSocket->state() == udpSocket->BoundState) {

        qDebug() << Q_FUNC_INFO << "Closing connection thread";
        thread->terminateThread();
        delete thread;
        thread = nullptr;

        qDebug() << Q_FUNC_INFO << "Closing UDP connection";
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;

        if (fetchAiAircraft) {
            qDebug() << Q_FUNC_INFO << "Closing Online Presence TCP Connection";
            onlineTcpSocket->abort();
            onlineTcpSocket->close();
            delete onlineTcpSocket;
            onlineTcpSocket = nullptr;
        }

        qInfo(atools::fs::ns::gui).noquote().nospace() << "Closed FlightGear connection slot.";
      }
    }
}

void MainWindow::readPendingDatagrams()
{
    QByteArray rxData;
    QHostAddress sender;
    quint16 senderPort;

    while (udpSocket->hasPendingDatagrams())
    {
        qDebug() << Q_FUNC_INFO << "Read pending datagrams";

        // Resize and zero byte buffer so we can make way for the new data.
        rxData.fill(0, udpSocket->pendingDatagramSize());

        // Read data from the UDP buffer.
        udpSocket->readDatagram(rxData.data(), rxData.size(), &sender, &senderPort);

        qDebug() << Q_FUNC_INFO << "Received: " << rxData.size();

        // convert to ascii
        QString dataAsString = QString(rxData);

        // Copy data from datarefs and pass it over to the thread for writing into the shared memory
        thread->fetchAndWriteData(dataAsString, this->fetchAi);
    }
}

void MainWindow::tcpSocketReadyRead()
{
    qDebug() << "reading...";
    onlineStatus += onlineTcpSocket->readAll();
}

void MainWindow::tcpSocketConnected()
{
    qDebug() << "connected...";
    onlineStatus = QString("");
}

void MainWindow::initOnlineTcpConnection()
{
    Settings& settings = Settings::instance();
    QString multiplayerServerHost = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_HOST, "").toString();
    int multiplayerServerPort = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_MULTIPLAYER_SERVER_PORT, 5001).toInt();

    onlineTcpSocket->connectToHost(multiplayerServerHost, multiplayerServerPort);
    // we need to wait...
    if(!onlineTcpSocket->waitForConnected(5000))
    {
        qDebug() << "Error: " << onlineTcpSocket->errorString();
    }
}

void MainWindow::tcpSocketDisconnected()
{
    qDebug() << "Disconnected...";
    qDebug() << "Online status: " << onlineStatus;

    thread->writeOnlinePresenceData(onlineStatus);

    // re-run connection after 5 seconds
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MainWindow::initOnlineTcpConnection);
    timer->start(5000);
}

void MainWindow::mainWindowShown()
{
  qDebug() << Q_FUNC_INFO;

  atools::settings::Settings& settings = Settings::instance();

  qInfo(atools::fs::ns::gui).noquote().nospace() << QApplication::applicationName();
  qInfo(atools::fs::ns::gui).noquote().nospace() << tr("Version %1 (revision %2).").
    arg(QApplication::applicationVersion()).arg(GIT_REVISION);

  qInfo(atools::fs::ns::gui).noquote().nospace()
    << tr("Data Version %1. Reply Version %2.").arg(SimConnectData::getDataVersion()).arg(
    SimConnectReply::getReplyVersion());

  // Build the handler classes which are an abstraction to SimConnect and the Little Xpconnect shared memory
  xpConnectHandler = new atools::fs::sc::XpConnectHandler();

  ui->menuTools->insertAction(ui->actionOptions, ui->toolBar->toggleViewAction());

  // Build the thread which will read the data from the interfaces
  dataReader = new atools::fs::sc::DataReaderThread(this, verbose);
  dataReader->setHandler(xpConnectHandler);
  dataReader->setReconnectRateSec(settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_RECONNECT_RATE, 10).toInt());
  dataReader->setUpdateRate(settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_UPDATE_RATE, 500).toUInt());

  atools::fs::sc::Options options = atools::fs::sc::NO_OPTION;
  if(settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool())
    options |= atools::fs::sc::FETCH_AI_AIRCRAFT;
  dataReader->setSimconnectOptions(options);

  connect(dataReader, &atools::fs::sc::DataReaderThread::postLogMessage, this, &MainWindow::postLogMessage);

  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  QGuiApplication::setOverrideCursor(Qt::WaitCursor);

  dataReader->start();

  QGuiApplication::restoreOverrideCursor();

  qInfo(atools::fs::ns::gui).noquote().nospace() << tr("Server running.");
}

void MainWindow::showEvent(QShowEvent *event)
{
  if(firstStart)
  {
    emit windowShown();
    firstStart = false;
  }

  event->ignore();
}
