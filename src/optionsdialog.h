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

#ifndef LITTLEFGCONNECT_OPTIONSDIALOG_H
#define LITTLEFGCONNECT_OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog :
  public QDialog
{
  Q_OBJECT

public:
  explicit OptionsDialog(QWidget *parent = 0);
  ~OptionsDialog();

  int getPort() const;
  unsigned int getUpdateRate() const;
  bool isHideHostname() const;
  bool isFetchAiAircraft() const;

  void setPort(int port);
  void setUpdateRate(unsigned int ms);
  void setHideHostname(bool hide);
  void setFetchAiAircraft(bool value);

private:
  Ui::OptionsDialog *ui;
};

#endif // LITTLEFGCONNECT_OPTIONSDIALOG_H
