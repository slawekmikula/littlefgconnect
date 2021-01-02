/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
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

#include "fgconnect.h"

#include "fs/sc/simconnectuseraircraft.h"
#include "fs/sc/simconnectdata.h"
#include "fs/sc/simconnecttypes.h"
#include "geo/calculations.h"

using atools::geo::kgToLbs;
using atools::geo::meterToFeet;
using atools::geo::meterToNm;
using atools::roundToInt;
using atools::geo::Pos;

namespace xpc {

XpConnect::XpConnect()
{
  qDebug() << Q_FUNC_INFO;
}

XpConnect::~XpConnect()
{
  qDebug() << Q_FUNC_INFO;
}

bool XpConnect::fillSimConnectData(QString simData, atools::fs::sc::SimConnectData& data, bool fetchAi)
{
    Q_UNUSED(fetchAi);

    //QString retval = "2019-12-26T09:27:49;7200;3.564416;763.517456;2.513103;150.000000;26.924566;30.476448;39.534885;238.000000;5.453534;28949.210938;354.546478;347.994141;Piper PA28-161 Warrior II (160hp);PA28-161-160;50.0742416382;19.8020534515;347.994141;342.540619;0.000000;229.615570;0.004633;0.004734;0.000007;-12.254924";
    QString retval = simData;
    QStringList pieces = retval.split(";");

    qDebug() << Q_FUNC_INFO << retval;

    if (pieces.size() == 1) {
        return false;
    }

    int index = 0;
    //      <name>2019-12-25T09:43:49</name>
    QString timegmt = pieces.at(index++);
    //      <name>7200</name>
    int timeLocalOffset = pieces.at(index++).toInt();
    //      <name>altitudeAboveGroundFt = 0.f</name>
    float altitudeAboveGroundFt = pieces.at(index++).toFloat();
    //      <name>groundAltitudeFt = 0.f</name>
    float groundAltitudeFt = pieces.at(index++).toFloat();
    //      <name>windSpeedKts = 0.f</name>
    float windSpeedKts = pieces.at(index++).toFloat();
    //      <name>windDirectionDegT = 0.f</name>
    float windDirectionDegT = pieces.at(index++).toFloat();
    //      <name>ambientTemperatureCelsius = 0.f</name>
    float ambientTemperatureCelsius = pieces.at(index++).toFloat();
    //      <name>seaLevelPressureMbar = 0.f - inhg</name>
    float seaLevelPressureInhg = pieces.at(index++).toFloat();
    //      <name>airplaneTotalWeightLbs
    float airplaneTotalWeightLbsYasim = pieces.at(index++).toFloat();
    //      <name>airplaneTotalWeightLbs
    float airplaneTotalWeightLbsJsbsim = pieces.at(index++).toFloat();
    //      <name>fuelTotalQuantityGallons = 0.f</name>
    float fuelTotalQuantityGallons = pieces.at(index++).toFloat();
    //      <name>fuelTotalWeightLbs = 0.f</name>
    float fuelTotalWeightLbs = pieces.at(index++).toFloat();
    //      <name>fuelFlowPPH (in PPS)
    float fuelFlowPPS = pieces.at(index++).toFloat();
    //      <name>magVarDeg = 0.f</name>
    float magVarDeg = pieces.at(index++).toFloat();
    //      <name>ambientVisibilityMeter = 0.f;f</name>
    float ambientVisibilityMeter = pieces.at(index++).toFloat();
    //      <name>trackMagDeg = 0.f;</name>
    float trackMagDeg = pieces.at(index++).toFloat();
    //      <name>trackTrueDeg = 0.f;</name>
    float trackTrueDeg = pieces.at(index++).toFloat();
    //      <name>airplaneTitle = ""</name>
    QString airplaneTitle = pieces.at(index++);
    //      <name>airplaneModel = ""</name>
    QString airplaneModel = pieces.at(index++);
    //      <name>airplaneCallsign = ""</name>
    QString airplaneCallsign = pieces.at(index++);
    //      <name>atools::geo::Pos position - latitude</name>
    float latitude = pieces.at(index++).toFloat();
    //      <name>atools::geo::Pos position - longitude</name>
    float longitude = pieces.at(index++).toFloat();
    //      <name>headingTrueDeg = 0.f</name>
    float headingTrueDeg = pieces.at(index++).toFloat();
    //      <name>headingMagDeg = 0.f</name>
    float headingMagDeg = pieces.at(index++).toFloat();
    //      <name>groundSpeedKts = 0.f</name>
    float groundSpeedKts = pieces.at(index++).toFloat();
    //      <name>indicatedAltitudeFt = 0.f</name>
    float indicatedAltitudeFt = pieces.at(index++).toFloat();
    //      <name>indicatedSpeedKts = 0.f</name>
    float indicatedSpeedKts = pieces.at(index++).toFloat();
    //      <name>trueAirspeedKts = 0.f</name>
    float trueAirspeedKts = pieces.at(index++).toFloat();
    //      <name>machSpeed = 0.f</name>
    float machSpeed = pieces.at(index++).toFloat();
    //      <name>verticalSpeedFeetPerMin = 0.f</name>
    float verticalSpeedFeetPerMin = pieces.at(index++).toFloat();
    //      flightModel Type (jsb or yasim)
    QString flightModel = pieces.at(index++);

    atools::fs::sc::SimConnectUserAircraft& userAircraft = data.userAircraft;

    qDebug() << Q_FUNC_INFO << "flightModel ";

    userAircraft.position = Pos(longitude, latitude, altitudeAboveGroundFt);
    if(!userAircraft.position.isValid() || userAircraft.position.isNull()) {
      return false;
    }

    // Build local time
    QDateTime zuluDateTime = QDateTime::fromString(timegmt, "yyyy-MM-ddTHH:mm:ss");
    userAircraft.zuluDateTime = zuluDateTime;
    QDateTime localDateTime(zuluDateTime.date(), zuluDateTime.time(), Qt::OffsetFromUTC, timeLocalOffset);
    userAircraft.localDateTime = localDateTime;

    userAircraft.magVarDeg = magVarDeg;

    // Wind and ambient parameters
    userAircraft.windSpeedKts = windSpeedKts;
    userAircraft.windDirectionDegT = windDirectionDegT;
    userAircraft.ambientTemperatureCelsius = ambientTemperatureCelsius;
    userAircraft.totalAirTemperatureCelsius = ambientTemperatureCelsius; // FIXME
    userAircraft.seaLevelPressureMbar = seaLevelPressureInhg/0.029530f;

    // Ice
    // userAircraft.pitotIcePercent
    // userAircraft.structuralIcePercent

    // Weight
    // userAircraft.airplaneMaxGrossWeightLbs
    if (flightModel == "jsb") {
        userAircraft.airplaneTotalWeightLbs = airplaneTotalWeightLbsJsbsim;
        // simplification - does not account people & luggage weight
        userAircraft.airplaneEmptyWeightLbs = airplaneTotalWeightLbsJsbsim - fuelTotalWeightLbs;
    } else {
        userAircraft.airplaneTotalWeightLbs = airplaneTotalWeightLbsYasim;
        // simplification - does not account people & luggage weight
        userAircraft.airplaneEmptyWeightLbs = airplaneTotalWeightLbsYasim - fuelTotalWeightLbs;
    }

    // Fuel flow in weight
    userAircraft.fuelTotalWeightLbs = fuelTotalWeightLbs;
    userAircraft.fuelTotalQuantityGallons = fuelTotalQuantityGallons;
    userAircraft.fuelFlowPPH = fuelFlowPPS * 3600;
    userAircraft.fuelFlowGPH = fuelFlowPPS * 3600 * 8.35; // for water ??
    // userAircraft.numberOfEngines

    userAircraft.ambientVisibilityMeter = ambientVisibilityMeter;

    // SimConnectAircraft
    userAircraft.airplaneTitle = airplaneTitle;
    userAircraft.airplaneModel = airplaneModel;
    userAircraft.airplaneReg = airplaneCallsign;
    // userAircraft.airplaneType;
    // userAircraft.airplaneAirline;
    // userAircraft.airplaneFlightnumber;
    // userAircraft.fromIdent;
    // userAircraft.toIdent;

    userAircraft.altitudeAboveGroundFt = altitudeAboveGroundFt;
    userAircraft.groundAltitudeFt = groundAltitudeFt;
    userAircraft.indicatedAltitudeFt = indicatedAltitudeFt;

    // Heading and track
    userAircraft.headingMagDeg = headingMagDeg;
    userAircraft.headingTrueDeg = headingTrueDeg;
    userAircraft.trackMagDeg = trackMagDeg;
    userAircraft.trackTrueDeg = trackTrueDeg;

    // Speed
    userAircraft.indicatedSpeedKts = indicatedSpeedKts;
    userAircraft.trueAirspeedKts = trueAirspeedKts;
    userAircraft.machSpeed = machSpeed;
    userAircraft.verticalSpeedFeetPerMin = verticalSpeedFeetPerMin;
    userAircraft.groundSpeedKts = groundSpeedKts;

    // Model
    // userAircraft.modelRadiusFt
    // userAircraft.wingSpanFt

    // Set misc flags
    userAircraft.flags = atools::fs::sc::IS_USER | atools::fs::sc::SIM_XPLANE; // FIXME
    if((int)altitudeAboveGroundFt > 0) {
      userAircraft.flags |= atools::fs::sc::ON_GROUND;
    }

    // userAircraft.flags |= atools::fs::sc::IN_RAIN;
    // IN_CLOUD = 0x0002, - not available
    // IN_SNOW = 0x0008,  - not available

    // userAircraft.flags |= atools::fs::sc::SIM_PAUSED;
    // userAircraft.flags |= atools::fs::sc::SIM_REPLAY;

    userAircraft.category = atools::fs::sc::UNKNOWN;
    // AIRPLANE, HELICOPTER, BOAT, GROUNDVEHICLE, CONTROLTOWER, SIMPLEOBJECT, VIEWER

    userAircraft.engineType = atools::fs::sc::UNSUPPORTED;
    // PISTON = 0, JET = 1, NO_ENGINE = 2, HELO_TURBINE = 3, UNSUPPORTED = 4, TURBOPROP = 5

    return true;
}

} // namespace xpc
