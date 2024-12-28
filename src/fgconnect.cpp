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

bool XpConnect::fillSimConnectData(QString simData, QString onlineStatus, atools::fs::sc::SimConnectData& data, bool fetchAi)
{
    Q_UNUSED(fetchAi);

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
    //      <name>fuelFlowGPH
    float fuelFlowGPH = pieces.at(index++).toFloat();
    //      <name>fuelFlowPPH (in PPS)
    float fuelFlowPPS = pieces.at(index++).toFloat();
    //      <name>fuelFlowGPH (yasim)
    float fuelFlowGPH0 = pieces.at(index++).toFloat();
    //      <name>fuelFlowGPH (yasim)
    float fuelFlowGPH1 = pieces.at(index++).toFloat();
    //      <name>fuelFlowGPH (yasim)
    float fuelFlowGPH2 = pieces.at(index++).toFloat();
    //      <name>fuelFlowGPH (yasim)
    float fuelFlowGPH3 = pieces.at(index++).toFloat();
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
    //      flight freeze (pause)
    QString flightFreeze = pieces.at(index++);
    //      flight replay (fgtape)
    int flightReplay = pieces.at(index++).toInt();
    //      multiplayer online
    QString multiplayerOnline = pieces.at(index++);
    //      multiplayer server
    QString multiplayerServer = pieces.at(index++);
    //      ai objects combined
    QString aiObjectsCombined = pieces.at(index++);

    atools::fs::sc::SimConnectUserAircraft& userAircraft = data.userAircraft;

    // Reset user aircraft
    userAircraft = atools::fs::sc::SimConnectUserAircraft();

    userAircraft.position = Pos(longitude, latitude, altitudeAboveGroundFt);

    userAircraft.properties.addProp(atools::util::Prop(atools::fs::sc::PROP_XPCONNECT_VERSION, QCoreApplication::applicationVersion()));

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
    userAircraft.totalAirTemperatureCelsius = ambientTemperatureCelsius; // FIXME - use the same value ?
    userAircraft.seaLevelPressureMbar = seaLevelPressureInhg/0.029530f;

    // Ice
    // userAircraft.pitotIcePercent
    // userAircraft.structuralIcePercent

    // Weight    
    userAircraft.airplaneTotalWeightLbs = flightModel.contains("jsb") ? airplaneTotalWeightLbsJsbsim : airplaneTotalWeightLbsYasim;
    // simplification
    userAircraft.airplaneMaxGrossWeightLbs = userAircraft.airplaneTotalWeightLbs;
    // simplification - does not account people & luggage weight
    userAircraft.airplaneEmptyWeightLbs = userAircraft.airplaneTotalWeightLbs - fuelTotalWeightLbs;

    // Fuel flow in weight
    userAircraft.fuelTotalWeightLbs = fuelTotalWeightLbs;
    userAircraft.fuelTotalQuantityGallons = fuelTotalQuantityGallons;

    if (flightModel.contains("jsb")) {
        userAircraft.fuelFlowGPH = fuelFlowGPH;
        userAircraft.fuelFlowPPH = fuelFlowPPS * 3600;
    } else {
        float temperatureFarenheit = ambientTemperatureCelsius * 1.8 + 32;
        // density relation of Jet A fuel:
        // fuel has 7.275 lbs/gal at -100 °F
        // fuel has 6.950 lbs/gal at 0 °F
        // fuel has 6.625 lbs/gal at 100 °F
        // change 0.00325 lbs/gal per 1 °F
        float fuelDensityJetA = temperatureFarenheit * -0.00325 + 6.95;

        // density relation of LL100 (AVGAS) fuel:
        // fuel has 6.490 lbs/gal at -100 °F
        // fuel has 6.080 lbs/gal at 0 °F
        // fuel has 5.670 lbs/gal at 100 °F
        // change 0.0041 lbs/gal per 1 °F
        float fuelDensityll100 = temperatureFarenheit * -0.0041 + 6.08;

        userAircraft.fuelFlowGPH = fuelFlowGPH0 + fuelFlowGPH1 + fuelFlowGPH2 + fuelFlowGPH3;
        userAircraft.fuelFlowPPH = userAircraft.fuelFlowGPH * fuelDensityll100 ;
    }
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
    userAircraft.flags = atools::fs::sc::IS_USER | atools::fs::sc::SIM_XPLANE11; // FIXME - don't know if this is correct one
    if((int)altitudeAboveGroundFt == 0) {
      userAircraft.flags |= atools::fs::sc::ON_GROUND;
    }

    // userAircraft.flags |= atools::fs::sc::IN_RAIN;
    // IN_CLOUD = 0x0002, - not available
    // IN_SNOW = 0x0008,  - not available


    if (flightFreeze == "true") {
        userAircraft.flags |= atools::fs::sc::SIM_PAUSED;
    }
    if (flightReplay == 1) {
        userAircraft.flags |= atools::fs::sc::SIM_REPLAY;
    }

    userAircraft.category = atools::fs::sc::AIRPLANE;
    // AIRPLANE, HELICOPTER, BOAT, GROUNDVEHICLE, CONTROLTOWER, SIMPLEOBJECT, VIEWER

    userAircraft.engineType = atools::fs::sc::UNSUPPORTED;
    // PISTON = 0, JET = 1, NO_ENGINE = 2, HELO_TURBINE = 3, UNSUPPORTED = 4, TURBOPROP = 5

    qDebug() << Q_FUNC_INFO << "Online: " << multiplayerOnline;
    if (multiplayerOnline == "true") {
        qDebug() << Q_FUNC_INFO << "Server: " << multiplayerServer;
    }

    if (fetchAi) {

        data.aiAircraft.clear();
        quint32 objId = 1;

        // AI objects parser
        qDebug() << Q_FUNC_INFO << "AI Objects: " << aiObjectsCombined;
        if (aiObjectsCombined != "") {
            QStringList aircrafts = aiObjectsCombined.split("|");
            foreach(QString aircraft, aircrafts) {
                if (aircraft == "") {
                    continue;
                }

                QStringList aircraftItem = aircraft.split("^");

                if (aircraftItem.size() != 6) {
                    qDebug() << Q_FUNC_INFO << "ERROR: AI Object size not 6 items: " << aircraft;
                    continue;
                }

                int index = 0;
                QString callsign = aircraftItem.at(index++);
                QString arrivalAirportId = aircraftItem.at(index++);
                QString departureAirportId = aircraftItem.at(index++);
                float altitudeFt = aircraftItem.at(index++).toFloat();
                float latitudeDeg = aircraftItem.at(index++).toFloat();
                float longitudeDeg = aircraftItem.at(index++).toFloat();

                atools::fs::sc::SimConnectAircraft aiAircraft;
                aiAircraft.airplaneFlightnumber = callsign;
                aiAircraft.fromIdent = departureAirportId;
                aiAircraft.toIdent = arrivalAirportId;
                aiAircraft.flags = atools::fs::sc::SIM_XPLANE11;
                aiAircraft.position = Pos(longitudeDeg, latitudeDeg, altitudeFt);

                // Mark fields as unavailable
                aiAircraft.headingTrueDeg = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

                aiAircraft.objectId = objId;
                aiAircraft.category = atools::fs::sc::AIRPLANE;
                aiAircraft.engineType = atools::fs::sc::UNSUPPORTED;

                data.aiAircraft.append(aiAircraft);

                objId++;
            }
        }

        // Online objects parser
        qDebug() << Q_FUNC_INFO << "Online Users: " << onlineStatus;
        if (onlineStatus != "") {
            QStringList strList = onlineStatus.split("\n",Qt::SkipEmptyParts);
            for(int index = 1;index < strList.length();index++)
            {
                qDebug() << strList[index];
                if (strList[index].startsWith("#")) {
                    continue;
                }

                QStringList userData = strList[index].split(" ");
                // "RER@mpserver01: 1034171.664623 -6222033.096334 1007853.793531 9.138567 -80.563069 32170.780096
                // -1.734371 0.059653 0.326972 Aircraft/757-200/Models/757-200.xml"
                qDebug() << "Online user data: " << userData;
                // 0 callsign
                QString callsign = userData[0].mid(0, userData[0].indexOf("@"));
                // 4 - lat
                float latitudeDeg = userData[4].toFloat();
                // 5 - lon
                float longitudeDeg = userData[5].toFloat();
                // 6 - altitude
                float altitudeFt = userData[6].toFloat();
                // 10 - model
                QString model = userData[10].split("/").last();
                model = model.mid(0, model.indexOf(".xml"));

                qDebug() << "callsign: " << callsign
                         << " latitude: " << latitudeDeg
                         << " longitude: " << longitudeDeg
                         << " altitude: " << altitudeFt
                         << " model: " << model;

                atools::fs::sc::SimConnectAircraft aiAircraft;
                aiAircraft.airplaneFlightnumber = callsign;
                aiAircraft.flags = atools::fs::sc::SIM_XPLANE11;
                aiAircraft.position = Pos(longitudeDeg, latitudeDeg, altitudeFt);

                // Mark fields as unavailable
                aiAircraft.headingTrueDeg = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.headingMagDeg = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.groundSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.indicatedAltitudeFt = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.indicatedSpeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.trueAirspeedKts = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.machSpeed = atools::fs::sc::SC_INVALID_FLOAT;
                aiAircraft.verticalSpeedFeetPerMin = atools::fs::sc::SC_INVALID_FLOAT;

                aiAircraft.objectId = objId;
                aiAircraft.category = atools::fs::sc::AIRPLANE;
                aiAircraft.engineType = atools::fs::sc::UNSUPPORTED;

                data.aiAircraft.append(aiAircraft);

                objId++;
            }
        }

    }

    return true;
}

} // namespace xpc
