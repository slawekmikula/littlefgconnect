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
#include "fs/sc/simconnecthandler.h"
#include "fs/sc/xpconnecthandler.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QActionGroup>
#include <QDir>
#include <QRegularExpression>

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
                              "<p><b>Copyright 2015-2020 Alexander Barthel</b></p>");

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

  // Create a group to turn the simulator actions into mutual exclusive ones
  simulatorActionGroup = new QActionGroup(ui->menuTools);
  simulatorActionGroup->addAction(ui->actionConnectFlightgear);

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

  delete simulatorActionGroup;
  qDebug() << Q_FUNC_INFO << "fsActionGroup deleted";

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
  HelpHandler::openFile(this, HelpHandler::getHelpFile(HELP_OFFLINE_FILE, "en" /* override */));
}

void MainWindow::options()
{
  OptionsDialog dialog;

  Settings& settings = Settings::instance();
  unsigned int updateRateMs = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_UPDATE_RATE, 500).toUInt();
  int port = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_DEFAULT_PORT, 7755).toInt();
  bool hideHostname = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_HIDE_HOSTNAME, false).toBool();

  bool fetchAiAircraft = settings.getAndStoreValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, true).toBool();

  dialog.setUpdateRate(updateRateMs);
  dialog.setPort(port);
  dialog.setHideHostname(hideHostname);
  dialog.setFetchAiAircraft(fetchAiAircraft);

  int result = dialog.exec();

  if(result == QDialog::Accepted)
  {
    settings.setValue(lfgc::SETTINGS_OPTIONS_HIDE_HOSTNAME, static_cast<int>(dialog.isHideHostname()));
    settings.setValue(lfgc::SETTINGS_OPTIONS_UPDATE_RATE, static_cast<int>(dialog.getUpdateRate()));
    settings.setValue(lfgc::SETTINGS_OPTIONS_DEFAULT_PORT, dialog.getPort());
    settings.setValue(lfgc::SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT, dialog.isFetchAiAircraft());

    settings.syncSettings();

    atools::fs::sc::Options options = atools::fs::sc::NO_OPTION;
    if(dialog.isFetchAiAircraft())
      options |= atools::fs::sc::FETCH_AI_AIRCRAFT;

    dataReader->setSimconnectOptions(options);

    if(dialog.getUpdateRate() != updateRateMs)
    {
      // Update rate changed - restart data readers
      dataReader->terminateThread();
      dataReader->setUpdateRate(dialog.getUpdateRate());
      dataReader->start();
    }

    if(dialog.getPort() != port)
    {
      // Restart navserver on port change
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
    qDebug() << Q_FUNC_INFO;

    if(udpSocket == nullptr) {
      udpSocket = new QUdpSocket(this);
      if (!udpSocket->bind(7755)) {
        qWarning() << Q_FUNC_INFO << "Cannot open UDP port";
        udpSocket = nullptr;
      } else {
        QObject::connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
        qInfo() << Q_FUNC_INFO << "Attached to the UDP port";

        thread = new SharedMemoryWriter();
        thread->start();
      }
    } else {
      if (udpSocket->state() == udpSocket->BoundState) {

        qDebug() << Q_FUNC_INFO << "Closing connection";
        thread->terminateThread();
        delete thread;
        thread = nullptr;

        qDebug() << Q_FUNC_INFO << "Closing connection";
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
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
        qInfo() << Q_FUNC_INFO << "Read pending datagrams";

        // Resize and zero byte buffer so we can make way for the new data.
        rxData.fill(0, udpSocket->pendingDatagramSize());

        // Read data from the UDP buffer.
        udpSocket->readDatagram(rxData.data(), rxData.size(), &sender, &senderPort);

        qInfo() << Q_FUNC_INFO << "Received: " << rxData.size();

        // Copy data from datarefs and pass it over to the thread for writing into the shared memory
        thread->fetchAndWriteData(false);
    }
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
