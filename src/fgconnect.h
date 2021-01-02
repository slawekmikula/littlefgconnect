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

#ifndef LITTLEFGCONNECT_FGCONNECT_H
#define LITTLEFGCONNECT_FGCONNECT_H

#include <QCache>

namespace atools {
namespace fs {
namespace sc {
class SimConnectData;
class SimConnectAircraft;
}
}
}

namespace lfgc {

/*
 * Class that has full access to SimConnectData.
 */
class FgConnect
{
public:
  FgConnect();
  ~FgConnect();

  /* Fill SimConnectData from FlightGear datarefs. Returns true if data was found */
  bool fillSimConnectData(atools::fs::sc::SimConnectData& data, bool fetchAi);

};

} // namespace lfgc

#endif // LITTLEFGCONNECT_FGCONNECT_H
