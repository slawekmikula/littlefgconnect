/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*           2020 Slawomir Mikula slawek.mikula@gmail.com
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

#ifndef LITTLEFGCONNECT_MAINWINDOW_H
#define LITTLEFGCONNECT_MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>

#include "sharedmemorywriter.h"

namespace Ui {
class MainWindow;
}

namespace atools {
namespace gui {
class HelpHandler;
}
namespace fs {
namespace sc {
class DataReaderThread;
class SimConnectHandler;
class XpConnectHandler;
class ConnectHandler;
}
namespace ns {
class NavServer;

}
}
}

class QActionGroup;

class MainWindow :
  public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  virtual ~MainWindow();

  void postLogMessage(QString message, bool warning);

signals:
  /* Append a log message to the gui log. */
  void appendLogMessage(const QString& message);

  /* Emitted when window is shown the first time */
  void windowShown();

private slots:
  void readPendingDatagrams();

private:
  /* Loggin handler will send log messages of category gui to this method which will emit
   * appendLogMessage to ensure that the message is appended using the main thread context. */
  void logGuiMessage(QtMsgType type, const QMessageLogContext& context, const QString& message);
  virtual void showEvent(QShowEvent *event) override;
  virtual void closeEvent(QCloseEvent *event) override;

  void readSettings();
  void writeSettings();

  /* Window visible for the first time after startup */
  void mainWindowShown();

  /* Reset all "do not show again" messages */
  void resetMessages();

  /* Options dialog */
  void options();

  void showOnlineHelp();
  void showOfflineHelp();

  void startStopConnection();

  Ui::MainWindow *ui = nullptr;

  // Runs in background and fetches data from simulator - signals are sent to NavServerWorker threads
  atools::fs::sc::DataReaderThread *dataReader = nullptr;
  atools::fs::sc::XpConnectHandler *xpConnectHandler = nullptr;
  QActionGroup *simulatorActionGroup = nullptr;

  // FlightGear communication
  QUdpSocket* udpSocket = nullptr;
  SharedMemoryWriter *thread = nullptr;

  atools::gui::HelpHandler *helpHandler = nullptr;
  bool firstStart = true; // Used to emit the first windowShown signal
  bool verbose = false;

  QString supportedLanguageOnlineHelp;
  QString aboutMessage;
};

#endif // LITTLEFGCONNECT_MAINWINDOW_H
