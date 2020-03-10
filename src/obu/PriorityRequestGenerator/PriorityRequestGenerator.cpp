/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGenerator.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This is the initial revision developed for receiving bsm, map, ssm data from the transceiver. 
  2. This script use mapengine library to obtain vehicle status based on active map. 
  3. This script has an API to calculate ETA. 
  4. This script has an API to generate srm json string which is compatible to asn1 j2735 standard.
  5. The generated srm json string will send to Transceiver over a UDP socket.
  6. This script has an API to create Active Request Table on the vehicle side based on the received ssm.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <netinet/in.h>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctime>
#include <cmath>
#include <math.h>

#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"
#include "msgEnum.h"
#include "PriorityRequestGenerator.h"
#include "json/json.h"

using namespace GeoUtils;
using namespace MsgEnum;

const double DISTANCEUNITCONVERSION = 100; //cm to meter
const double VEHICLEMINSPEED = 4;
const double VEHICLE_SPEED_DEVIATION_LIMIT = 5.0;
const double ETA_DURATION_SECOND = 2;
const int HOURSINADAY = 24;
const int MINUTESINAHOUR = 60;
const double SECONDSINAMINUTE = 60.0;
const int SECONDTOMILISECOND = 1000;
const double ALLOWED_ETA_DIFFERENCE = 4;
const int MAXMSGCOUNT = 127;
const int MINMSGCOUNT = 1;
const double MIN_ETA = 0.0;
const double SRM_GAPOUT_TIME = 2.0;

PriorityRequestGenerator::PriorityRequestGenerator()
{
}

/*
	- create Active request Table in the vehicle side based on the received ssm
*/

std::vector<ActiveRequest> PriorityRequestGenerator::creatingSignalRequestTable(SignalStatus signalStatus)
{
	//storing the information of ssm
	std::vector<int>vehicleID = signalStatus.getTemporaryVehicleID();
	std::vector<int>requestID = signalStatus.getRequestID();
	std::vector<int>msgCount_ssm = signalStatus.getMsgCount(); //insted of msgCount, msgCount_ssm since it shadowed the declaration
	std::vector<int>inBoundLaneID = signalStatus.getInBoundLaneID();
	std::vector<int>basicVehicleRole_ssm = signalStatus.getBasicVehicleRole(); //insted of basicVehicleRole, basicVehicleRole_ssm since it shadowed the declaration
	std::vector<int>expectedTimeOfArrival_Minute = signalStatus.getETA_Minute();
	std::vector<double>expectedTimeOfArrival_Second = signalStatus.getETA_Second();
	std::vector<int>priorityRequestStatus = signalStatus.getPriorityRequestStatus();
	ActiveRequest activeRequest;

	//creating the active request table based on the stored information
	if (addToActiveRequestTable(signalStatus) == true)
	{
		//Vehicle will clar the whole table when it received new ssm and again create new table.
		//Updating existing ART is time consuming since each time it has to be checked if any fields has been changed in table or
		// if two vehicle information is removed from the table by PRS it may cause problem.

		ActiveRequestTable.clear();
		activeRequest.reset();
		
		for (int i = 0; i < signalStatus.getNoOfRequest(); i++)
		{
			activeRequest.vehicleID = vehicleID[i];
			activeRequest.requestID = requestID[i];
			activeRequest.msgCount = msgCount_ssm[i];
			activeRequest.basicVehicleRole = basicVehicleRole_ssm[i];
			activeRequest.vehicleLaneID = inBoundLaneID[i];
			activeRequest.vehicleETA = expectedTimeOfArrival_Minute[i] * SECONDSINAMINUTE + expectedTimeOfArrival_Second[i];
			activeRequest.prsStatus = priorityRequestStatus[i];
			activeRequest.minuteOfYear = getMinuteOfYear();
			ActiveRequestTable.push_back(activeRequest);
		}
	}

	return ActiveRequestTable;
}

/*
	- Create srm json string based on te bsm and associated information obtained based on the active map (laneID, approachID, ETA)
*/
std::string PriorityRequestGenerator::createSRMJsonObject(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager)
{
	std::string srmJsonString;
	int vehExpectedTimeOfArrival_Minute;
	double vehExpectedTimeOfArrival_Second = remquo((getTime2Go() / SECONDSINAMINUTE), 1.0, &vehExpectedTimeOfArrival_Minute);
	double vehDuration = ETA_DURATION_SECOND;

	tempVehicleSpeed = basicVehicle.getSpeed_MeterPerSecond(); //storing vehicle speed while sending srm. It will be use to compare if there is any speed change or not
	tempVehicleSignalGroup = getSignalGroup();

	signalRequest.setMinuteOfYear(getMinuteOfYear());
	signalRequest.setMsOfMinute(getMsOfMinute());
	signalRequest.setMsgCount(getMsgCount());
	signalRequest.setRegionalID(getRegionalID());
	signalRequest.setIntersectionID(getIntersectionID());
	signalRequest.setVehicleType(getVehicleType()); //getVehicleType() function has to be executed before the getBasicVehicleRole()
	signalRequest.setPriorityRequestType(getPriorityRequestType(basicVehicle, mapManager));
	signalRequest.setInBoundLaneIntersectionAccessPoint(getLaneID(), getApproachID());
	signalRequest.setETA(vehExpectedTimeOfArrival_Minute, vehExpectedTimeOfArrival_Second * SECONDSINAMINUTE, vehDuration);
	signalRequest.setTemporaryVechileID(getVehicleID(basicVehicle));
	signalRequest.setBasicVehicleRole(getBasicVehicleRole());
	signalRequest.setPosition(basicVehicle.getLatitude_DecimalDegree(), basicVehicle.getLongitude_DecimalDegree(), basicVehicle.getElevation_Meter());
	signalRequest.setHeading_Degree(basicVehicle.getHeading_Degree());
	signalRequest.setSpeed_MeterPerSecond(basicVehicle.getSpeed_MeterPerSecond());
	srmJsonString = signalRequest.signalRequest2Json();

	return srmJsonString;
}

/*
	- Check whether vehicle should accept the ssm and populate active request table or not.
		-- If current intersectionID and regionalID of the vehicle and intersectionID and regionalID of ssm match accept the ssm 
*/
bool PriorityRequestGenerator::addToActiveRequestTable(SignalStatus signalStatus)
{
	bool matchIntersection = false;

	if (getIntersectionID() == signalStatus.getIntersectionID() && getRegionalID() == signalStatus.getRegionalID())
		matchIntersection = true;

	return matchIntersection;
}

/*
	- Check whether vehicle needs to send srm or not based on the active map list, vehicle status.
*/
bool PriorityRequestGenerator::shouldSendOutRequest(BasicVehicle basicVehicle)
{
	bool bSendRequest = false;
	double vehicleSpeed = basicVehicle.getSpeed_MeterPerSecond();
	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == temporaryVehicleID; });

	if (bgetActiveMap == true && getVehicleIntersectionStatus() == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		bSendRequest = true;
		std::cout << "SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection" << std::endl;
	}

	else if (bgetActiveMap == true && findVehicleIDOnTable != ActiveRequestTable.end() && getVehicleIntersectionStatus() == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox)|| static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		bSendRequest = true;
		std::cout << "SRM is sent since vehicle is leaving the Intersection" << std::endl;
	}

	else if (bgetActiveMap == true && getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && abs(tempSRMTimeStamp - getMsOfMinute() / SECONDTOMILISECOND) >= SRM_GAPOUT_TIME) //check if there is active or not
	{

		if (findVehicleIDOnTable == ActiveRequestTable.end()) //If vehicleID is not in the ART, vehicle should send srm
		{
			bSendRequest = true;
			std::cout << "SRM is sent since ART is empty" << std::endl;
		}

		else if (findVehicleIDOnTable != ActiveRequestTable.end() && tempVehicleSignalGroup != getSignalGroup()) //If vehicle signal group changed it should send SRM. Vehicle signal group can be messed up when it is inside the intersectionBox, due to which it is required to check whether vehicle is on inBoundlane or not
		{
			bSendRequest = true;
			std::cout << "SRM is sent since vehicle signalGroup has been changed" << std::endl;
		}

		else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(vehicleSpeed - tempVehicleSpeed) >= VEHICLE_SPEED_DEVIATION_LIMIT) //If vehicleID is in ART and vehicle speed changes by 5m/s, vehicle should send srm. tempVehicleSpeed store the vehicle speed of last send out srm.
		{
			bSendRequest = true;
			std::cout << "SRM is sent since vehicle speed has been changed" << std::endl;
		}

		// else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->vehicleLaneID != getLaneID()) //If vehicleID is in ART and vehicle laneID changes, vehicle should send srm
		// {
		// 	bSendRequest = true;
		// 	std::cout << "SRM is sent since vehicle laneID has been changed" << std::endl;
		// }

		else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleETA - getTime2Go()) >= ALLOWED_ETA_DIFFERENCE) //If vehicleID is in ART and vehicle ETA doesn't match the ETA of ART, vehicle should send srm
		{
			bSendRequest = true;
			std::cout << "SRM is sent since vehicle ETAhas been changed from" << findVehicleIDOnTable->vehicleETA << " to " << getTime2Go() << std::endl;
		}

		else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->msgCount != msgCount) //If vehicleID is in ART and message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
		{
			bSendRequest = true;
			std::cout << "SRM is sent since msgCount doesn't match" << std::endl;
		}
	}

	if (bSendRequest == true)
		tempSRMTimeStamp = getMsOfMinute() / SECONDTOMILISECOND;

	return bSendRequest;
}

void PriorityRequestGenerator::setIntersectionID(int vehicleNearByIntersectionId)
{
	intersectionID = vehicleNearByIntersectionId;
}

void PriorityRequestGenerator::setRegionalID(int vehicleNearByRegionalId)
{
	regionalID = vehicleNearByRegionalId;
}

void PriorityRequestGenerator::setLaneID(int laneId)
{
	vehicleLaneID = laneId;
}

void PriorityRequestGenerator::setApproachID(int approachID)
{
	vehicleAprroachID = approachID;
}

void PriorityRequestGenerator::setSignalGroup(int phaseNo)
{
	signalGroup = phaseNo;
}

/*
	-calculation for ETA. Units will be second
*/
bool PriorityRequestGenerator::setTime2Go(double distance2go, double vehicleSpeed)
{
	bool bETAValue = false;
	double vehicleTime2Go;
	if (vehicleSpeed > VEHICLEMINSPEED)
	{
		vehicleTime2Go = (distance2go / DISTANCEUNITCONVERSION) / vehicleSpeed; //distance2go is cm. DISTANCEUNITCONVERSION is used converst distance2go into meter
		time2go = vehicleTime2Go;
		bETAValue = true;
	}
	else
	{
		bETAValue = false;
		time2go = MIN_ETA;
	}

	return bETAValue;
}

/*
	-obtain vehicle location in the map-  whether it is in inBound or in intersectionBox or in outBound
*/
void PriorityRequestGenerator::setVehicleIntersectionStatus(int vehIntersectionStatus)
{
	vehicleIntersectionStatus = vehIntersectionStatus;
}

/*
	-Get the message type based on the received json string from Transceiver
*/
int PriorityRequestGenerator::getMessageType(std::string jsonString)
{
	Json::Value jsonObject;
	Json::Reader reader;
	reader.parse(jsonString.c_str(), jsonObject);

	if ((jsonObject["MsgType"]).asString() == "MAP")
	{
		messageType = MsgEnum::DSRCmsgID_map;
	}

	else if ((jsonObject["MsgType"]).asString() == "BSM")
	{
		messageType = MsgEnum::DSRCmsgID_bsm;
	}

	else if ((jsonObject["MsgType"]).asString() == "SSM")
	{
		messageType = MsgEnum::DSRCmsgID_ssm;
	}

	return messageType;
}

std::vector<Map::ActiveMap> PriorityRequestGenerator::getActiveMapList(MapManager mapManager)
{
	activeMapList = mapManager.getActiveMapList();

	return activeMapList;
}

/*
	-If there is active map, based on the bsm data this function will locate vehicle on the map and obtain inBoundLaneID, inBoundApproachID, distance from the stop-bar and time requires to reach the stop-bar
*/
void PriorityRequestGenerator::getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle)
{

	std::string fmap{};
	std::string intersectionName{};

	//If active map list is empty, look for active map
	if (activeMapList.empty())
	{
		mapManager.createActiveMapList(basicVehicle);
		getActiveMapList(mapManager);
	}

	//If active map List is not empty, locate vehicle on the map and obtain inBoundLaneID, inBoundApproachID, distance from the stop-bar and time requires to reach the stop-bar
	if (!activeMapList.empty())
	{
		bgetActiveMap = true; //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true.
		// std::cout << "Active Map List is not Empty" << std::endl;
		fmap = activeMapList.front().activeMapFileDirectory;
		intersectionName = activeMapList.front().activeMapFileName;
		bool singleFrame = false; /// TRUE to encode speed limit in lane, FALSE to encode in approach
		//initialize mapengine library
		LocAware *plocAwareLib = new LocAware(fmap, singleFrame);

		uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
		uint16_t regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
		uint16_t intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);
		double distance2go{};
		//get the vehicle data from bsm
		double vehicle_Latitude = basicVehicle.getLatitude_DecimalDegree();
		double vehicle_Longitude = basicVehicle.getLongitude_DecimalDegree();
		double vehicle_Elevation = basicVehicle.getLongitude_DecimalDegree();
		double vehicle_Speed = basicVehicle.getSpeed_MeterPerSecond();
		double vehicle_Heading = basicVehicle.getHeading_Degree();
		//initialize all the struct require to locate vehicle in Map.
		struct geoPoint_t geoPoint_t_1 = {vehicle_Latitude, vehicle_Longitude, vehicle_Elevation};
		struct motion_t motion_t_1 = {vehicle_Speed, vehicle_Heading};
		struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
		struct point2D_t point2D_t_1 = {0, 0};
		struct point2D_t point2D_t_2 = {0, 0};
		struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
		struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
		struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
		std::bitset<4> maneuvers;
		struct dist2go_t dist2go_t_1 = {0.0, 0.0};
		struct connectTo_t connectTo_t_1 = {0, 0, 0, maneuverType::straightAhead};
		std::vector<connectTo_t> connect2go1;
		connect2go1.push_back(connectTo_t_1);
		struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go1};
		struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
		struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};

		//counter_VehicleInMap will ensure after being inside the map vehicle doesn't go out of inBoundLane(stopped in the parking lot)
		if (counter_VehicleInMap > 10)
		{
			if (plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1) == true && unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onInbound))
			{
				setVehicleIntersectionStatus(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus));
				setIntersectionID(intersectionId);
				setRegionalID(regionalId);
				setLaneID(plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex)));
				setApproachID(plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)getLaneID())));
				setSignalGroup(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(vehicleAprroachID), static_cast<uint8_t>(vehicleLaneID))); //Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
				plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
				distance2go = unsigned(point2D_t_1.distance2pt(point2D_t_2)); //unit of centimeters
				setTime2Go(distance2go, vehicle_Speed);
				getVehicleID(basicVehicle); //Vehicle change its ID on a regular basis. Need to check the vehicle id.
				bRequestSendStatus = true;
			}

			else
			{
				mapManager.deleteActiveMapfromList();
				activeMapList.clear();
				ActiveRequestTable.clear();
				setIntersectionID(0);
				setSignalGroup(0);
				bgetActiveMap = false;
				bRequestSendStatus = false;
			}
			counter_VehicleInMap = 0;
		}

		else
		{
			plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);
			setVehicleIntersectionStatus(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus));
			setIntersectionID(intersectionId);
			setRegionalID(regionalId);
			setLaneID(plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex)));
			setApproachID(plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)getLaneID())));
			setSignalGroup(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(vehicleAprroachID), static_cast<uint8_t>(vehicleLaneID))); //Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
			plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
			distance2go = unsigned(point2D_t_1.distance2pt(point2D_t_2)); //unit of centimeters
			setTime2Go(distance2go, vehicle_Speed);
			getVehicleID(basicVehicle); //Vehicle change its ID on a regular basis. Need to check the vehicle id.
			counter_VehicleInMap++;
		}

		delete plocAwareLib;
	}
	else
	{
		// std::cout << "Active Map List is  Empty" << std::endl;
		bgetActiveMap = false;
	}
}

int PriorityRequestGenerator::getIntersectionID()
{
	return intersectionID;
}

int PriorityRequestGenerator::getRegionalID()
{
	return regionalID;
}

int PriorityRequestGenerator::getVehicleID(BasicVehicle basicVehicle)
{
	temporaryVehicleID = basicVehicle.getTemporaryID();

	return temporaryVehicleID;
}

int PriorityRequestGenerator::getLaneID()
{
	return vehicleLaneID;
}

int PriorityRequestGenerator::getApproachID()
{
	return vehicleAprroachID;
}

int PriorityRequestGenerator::getSignalGroup()
{
	return signalGroup;
}

double PriorityRequestGenerator::getTime2Go()
{
	return time2go;
}

int PriorityRequestGenerator::getVehicleIntersectionStatus()
{
	return vehicleIntersectionStatus;
}

/*
	- obtain vehicle type from VehicleConfiguration.json file
*/
int PriorityRequestGenerator::getVehicleType()
{
	Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
	std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
	vehicleType = (jsonObject_config["VehicleType"]).asInt();

	return vehicleType;
}

/*
	- Define vehile role based on vehicle type
*/
int PriorityRequestGenerator::getBasicVehicleRole()
{
	if (getVehicleType() == static_cast<int>(MsgEnum::vehicleType::bus))
	{
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::transit);
	}

	else if (getVehicleType() == static_cast<int>(MsgEnum::vehicleType::axleCnt4))
	{
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::truck);
	}

	else if (getVehicleType() == static_cast<int>(MsgEnum::vehicleType::special))
	{
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::fire);
	}

	else if (getVehicleType() == static_cast<int>(vehicleType::unavailable))
	{
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::unavailable);
	}
	return basicVehicleRole;
}

/*
	- Define priority status based on vehicle status
		--If vehicle is at inBoundLane priority request type will be priorityRequest
		--If vehicle is at insideIntersectionBox priority request type will be priorityCancellation
		--If vehicle speed changes by 5m/s priority request type will be requestUpdate
*/
int PriorityRequestGenerator::getPriorityRequestType(BasicVehicle basicVehicle, MapManager mapManager)
{
	double vehicleSpeed = basicVehicle.getSpeed_MeterPerSecond();
	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == temporaryVehicleID; });

	if (getVehicleIntersectionStatus() == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		
		priorityRequestType = static_cast<int>(MsgEnum::requestType::priorityCancellation); //Setting priority requestType
		mapManager.deleteActiveMapfromList();
		activeMapList.clear();
		ActiveRequestTable.clear();
		setIntersectionID(0);
		//setSignalGroup(0);
		bgetActiveMap = false; //Required for HMI json
		bRequestSendStatus = false;
	}

	else if (getVehicleIntersectionStatus() == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound)) && findVehicleIDOnTable != ActiveRequestTable.end())
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::priorityCancellation);
		mapManager.deleteActiveMapfromList();
		activeMapList.clear();
		ActiveRequestTable.clear();
		setIntersectionID(0);
		//setSignalGroup(0);
		bgetActiveMap = false; //Required for HMI json
		bRequestSendStatus = false;
	}
	
	else if (getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && findVehicleIDOnTable == ActiveRequestTable.end())
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::priorityRequest);
		bRequestSendStatus = true; //Required for HMI json
	}

	// else if (getVehicleIntersectionStatus() != static_cast<int>(MsgEnum::mapLocType::onInbound))
	// {
	// 	priorityRequestType = static_cast<int>(MsgEnum::requestType::priorityCancellation);
	// 	mapManager.deleteActiveMapfromList();
	// 	activeMapList.clear();
	// 	ActiveRequestTable.clear();
	// 	setIntersectionID(0);
	// 	//setSignalGroup(0);
	// 	bgetActiveMap = false; //Required for HMI json
	// 	bRequestSendStatus = false;
	// }
	

	else if (getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && findVehicleIDOnTable != ActiveRequestTable.end() && tempVehicleSignalGroup != signalGroup)
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::requestUpdate);
		bRequestSendStatus = true;
	}

	else if (getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && findVehicleIDOnTable != ActiveRequestTable.end() && abs(vehicleSpeed - tempVehicleSpeed) <= VEHICLE_SPEED_DEVIATION_LIMIT)
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::requestUpdate);
		bRequestSendStatus = true;
	}

	else if (getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleETA - getTime2Go()) >= ALLOWED_ETA_DIFFERENCE)
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::requestUpdate);
		bRequestSendStatus = true;
	}

	else if (getVehicleIntersectionStatus() == static_cast<int>(MsgEnum::mapLocType::onInbound) && findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->msgCount != msgCount)
	{
		priorityRequestType = static_cast<int>(MsgEnum::requestType::requestUpdate);
		bRequestSendStatus = true;
	}
	return priorityRequestType;
}

/*
	- Obtain system time and calculte current Minute of the Year
		--get the number of years passed (today's number of day in the year -1)
		--get number of hours and minutes elapsed today
*/
int PriorityRequestGenerator::getMinuteOfYear()
{
	int minuteOfYear{};
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int dayOfYear = timePtr->tm_yday;
	int currentHour = timePtr->tm_hour;
	int currentMinute = timePtr->tm_min;

	minuteOfYear = (dayOfYear - 1) * HOURSINADAY * MINUTESINAHOUR + currentHour * MINUTESINAHOUR + currentMinute;

	return minuteOfYear;
}

/*
	- Obtain system time for time stamp
*/
int PriorityRequestGenerator::getMsOfMinute()
{
	int msOfMinute{};
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int currentSecond = timePtr->tm_sec;

	msOfMinute = currentSecond * SECONDTOMILISECOND;

	return msOfMinute;
}

/*
	- Get the message count information for srm
*/
int PriorityRequestGenerator::getMsgCount()
{
	if (msgCount < MAXMSGCOUNT)
	{
		msgCount++;
	}
	else
	{
		msgCount = MINMSGCOUNT;
	}

	return msgCount;
}

/*
	- Get current signalgroup info of vehicle from MAP based on LaneID and AprroachID for HMI Controller
*/
// int PriorityRequestGenerator::getVehicleCurrentSignalGroup()
// {
// 	int signalGroup{};
// 	int approachID{};
// 	std::string fmap{};
// 	std::string intersectionName{};
// 	bool singleFrame = false; /// TRUE to encode speed limit in lane, FALSE to encode in approach

// 	if (!activeMapList.empty())
// 	{
// 		fmap = activeMapList.front().activeMapFileDirectory;
// 		intersectionName = activeMapList.front().activeMapFileName;

// 		//initialize mapengine library
// 		LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
// 		approachID = plocAwareLib->getApproachIdByLaneId(static_cast<uint16_t>(getRegionalID()), static_cast<uint8_t>(getIntersectionID()), static_cast<uint8_t>(getLaneID()));
// 		signalGroup = plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(getRegionalID()), static_cast<uint16_t>(getIntersectionID()), approachID, static_cast<uint8_t>(getLaneID()));

// 		delete plocAwareLib;
// 	}

// 	return signalGroup;
// }

/*
	- Get vehicle status on current map (Whether on map or not) for HMI Controller
*/
std::string PriorityRequestGenerator::getVehicleMapStatus()
{
	std::string vehicleMapStatus{};

	if (bgetActiveMap == true)
		vehicleMapStatus = "True";

	else
		vehicleMapStatus = "False";

	return vehicleMapStatus;
}

/*
	- Get information whether SRM is sent or not for HMI Controller
*/
std::string PriorityRequestGenerator::getVehicleRequestSentStatus()
{
	std::string vehicleSRMStatus{"False"};

	if (bRequestSendStatus == true)
		vehicleSRMStatus = "True";
	else
		vehicleSRMStatus = "False";

	return vehicleSRMStatus;
}

// std::vector<Map::ActiveMap> PriorityRequestGenerator::getActiveMapListFORHMI()
// {
// 	return activeMapList;
// }

/*
	- Getters for ART table
*/
std::vector<ActiveRequest> PriorityRequestGenerator::getActiveRequestTable()
{
	return ActiveRequestTable;
}

/*
	-Methods for updating map status for HMI
	-If vehicle in on Map then for the active map, activeMapStatus will be true for the active map
	-If vehicle is leaving the map (either leaving the intersection or going to parking lot) then activeMapStatus will be false for all available map
*/
std::vector<Map::AvailableMap> PriorityRequestGenerator::changeMapStatusInAvailableMapList(MapManager mapManager)
{

	if (!activeMapList.empty())
	{
		std::vector<Map::AvailableMap>::iterator findActiveMap = std::find_if(std::begin(mapManager.availableMapList), std::end(mapManager.availableMapList),
																			  [&](Map::AvailableMap const &p) { return p.availableMapFileName == activeMapList.front().activeMapFileName; });

		if (findActiveMap != availableMapList.end())
			findActiveMap->activeMapStatus = "True";

		availableMapList = mapManager.availableMapList;
	}

	else
	{
		for (size_t i = 0; i < availableMapList.size(); i++)
			availableMapList[i].activeMapStatus = "False";

		availableMapList = mapManager.availableMapList;
	}

	return availableMapList;
}

/*
	- This function is for printing Active request table. Here only few attributes are printed
*/
void PriorityRequestGenerator::printART()
{
	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	{
		std::cout << ActiveRequestTable[i].vehicleID << " " << ActiveRequestTable[i].vehicleLaneID << " " << ActiveRequestTable[i].vehicleETA << std::endl;
	}
}

PriorityRequestGenerator::~PriorityRequestGenerator()
{
}
