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

#ifndef LITTLEFGCONNECT_CONSTANTS_H
#define LITTLEFGCONNECT_CONSTANTS_H

#include <QLatin1String>

namespace lfgc {
/* key names for atools::settings */
const QLatin1String SETTINGS_OPTIONS_DEFAULT_PORT("Options/DefaultPort");
const QLatin1String SETTINGS_OPTIONS_UPDATE_RATE("Options/UpdateRate");
const QLatin1String SETTINGS_OPTIONS_RECONNECT_RATE("Options/ReconnectRate");
const QLatin1String SETTINGS_OPTIONS_FETCH_AI_AIRCRAFT("Options/FetchAiAircraft");
const QLatin1String SETTINGS_OPTIONS_MULTIPLAYER_SERVER_HOST("Options/MultiplayerServerHost");
const QLatin1String SETTINGS_OPTIONS_MULTIPLAYER_SERVER_PORT("Options/MultiplayerServerPort");
const QLatin1String SETTINGS_OPTIONS_VERBOSE("Options/Verbose");
const QLatin1String SETTINGS_OPTIONS_LANGUAGE("Options/Language");

const QLatin1String SETTINGS_MAINWINDOW_WIDGET("MainWindow/Widget");

const QLatin1String SETTINGS_ACTIONS_SHOW_PORT_CHANGE("Actions/ShowPortChange");
const QLatin1String SETTINGS_ACTIONS_SHOW_QUIT("Actions/ShowQuit");

} // namespace lfgc

#endif // LITTLEFGCONNECT_CONSTANTS_H
