"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

Scheduler.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class is responsible for processing the schedule received from the Priority Solver 
or any other MMITSS component. For processing following steps are followed:
(1) All commands in the schedule are sorted based on the command start time.
(2) Formulate the groups of commands having same start time and same action.
(3) Create the bitstring based on the phases included in the group, and formulate 
corresponding integer representation. Formulate the corresponding group command.
(4) Run an instance of background scheduler and based on the start time and the end time 
of the group command and the NTCIP backup time, schedule a SignalController::phaseControl 
function in the background scheduler.
(5) If the schedule is new, make sure not to stop the currently active actions in the NTCIP
mode of the signal controller, before executing the new schedule.
(6) At exit, clear the NTCIP commands in the signal controller, and in the background scheduler.

This class uses the APScheduler library, which needs to be installed separately.
For installation and general help: https://apscheduler.readthedocs.io/en/v1.3.1/
***************************************************************************************
"""

import json
import datetime
import atexit
from apscheduler.schedulers.background import BackgroundScheduler
from Command import Command
from SignalController import SignalController

class Scheduler:
    """
    Scheduler class is responsible for processing the schedule received from MMITSS components,
    and accordingly managing the BackgroundScheduler from the APScheduler library. The API for 
    managing the BackgroundScheduler is provided in the member functions. 
    Arguments: an object of the SignalController class.
    For example: scheduler = Scheduler(signalController)
    """
    def __init__(self, signalController:SignalController):
        
        self.commandId = 0        
        
        # Create an object of the SignalCOntroller class and extract the ntcipBackupTime.
        self.signalController = signalController
        self.ntcipBackupTime_Sec = signalController.ntcipBackupTime_sec

        # Scheduler parameters
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()
        self.scheduleTimingPlanUpdate(self.signalController.timingPlanUpdateInterval_sec)
        
        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.stopBackgroundScheduler())

      

    def processReceivedSchedule(self, scheduleJson:json):
        """
        Scheduler::processReceivedSchedule function is responsible for processing the schedule (json) passed in the arguments.
        After processing, command groups are created based on start times of individual commands, phases in the command, and the action required in command.
        The group commands are scheduled in the BackgroundScheduler. In addition to group commands, additional commands are created for:
        (1) Ending the scheudled commands after their EndTime is reached
        (2) If two (or more) commands have same startTime, same actions, but different endTimes.
        Arguments: (1) A JSON string containing the schedule

        *Scheduler::processReceivedSchedulefunction has three subfunctions:
        (1) clearOldSchedule(scheduleDataStructure)
        (2) formulateGroupCommand(currentGroup)
        (3) clearScheduleDataStructure(scheduleJson)

        
        """
        def clearOldSchedule(scheduleDataStructure:list):   
            """
            clearOldSchedule function checks the received schedule for actions starting at time 0.0.
            If a hold, vehOmit, or pedOmit starts at time zero, this function checks if the corresponding action is already active in 
            the signal controller. Only if such action is NOT active in the signal controller, then it is cleared from the signal controller.
            On the other hand, vehCall, pedCall, and forceoffs are cleared from the signal controller unconditionally.
            """     
            self.clearBackgroundScheduler(True)
            
            # Initialize flags to clear Holds, PedOmits, and VehOmits
            clearHolds = True
            clearPedOmit = True
            clearVehOmit = True

            # Update the flags based on the commands in the schedule
            for command in scheduleDataStructure:
                if command.startTime == 0 or command.startTime == 0.0:
                    if command.action == command.HOLD_VEH_PHASES:
                        if self.signalController.getPhaseControl(command.HOLD_VEH_PHASES) > 0:
                            clearHolds = False
                    elif command.action == command.OMIT_VEH_PHASES:    
                        if self.signalController.getPhaseControl(command.OMIT_VEH_PHASES) > 0:
                            clearVehOmit = False
                    elif command.action == command.OMIT_PED_PHASES:
                        if self.signalController.getPhaseControl(command.OMIT_PED_PHASES) > 0:
                            clearPedOmit = False

            # Clear Holds if its flag is True    
            if clearHolds == True:
                self.signalController.setPhaseControl(command.HOLD_VEH_PHASES,0)    
            
            # Clear VehOmits if its flag is True    
            if clearVehOmit == True:
                self.signalController.setPhaseControl(command.OMIT_VEH_PHASES,0)    

            # Clear PedOmits if its flag is True            
            if clearPedOmit == True:
                self.signalController.setPhaseControl(command.OMIT_PED_PHASES,0)    

            # Clear VehCalls
            self.signalController.setPhaseControl(command.CALL_VEH_PHASES,0)
            # Clear PedCalls
            self.signalController.setPhaseControl(command.CALL_PED_PHASES,0)
            # Clear Forceoffs
            self.signalController.setPhaseControl(command.FORCEOFF_PHASES,0)  
        
        
        # Formulate group command:
        def formulateGroupCommand(currentGroup:list):
            """
            formulateGroupCommand function takes a list of commands as an argument and returns a single object of Command class,
            that represents all commands in the group supplied in the arguments.
            arguments: (1) list of commands in current group
            *This function has 1 subfunction: formulateBinaryIntegerRepresentation.
            """

            # Formulate the binary integer representation of the commandPhase:
            def formulateBinaryIntegerRepresentation(groupPhases:list):
                """
                formulateBinaryIntegerRepresentation takes a list of phases in the argument, 
                creates a bitstring and returns the integer corresponding to the bitstring.
                arguments: (1) list of phases in the group
                """
                groupPhaseStr = list("00000000")
                
                if groupPhases[0] != 0:
                    for phase in groupPhases:
                        groupPhaseStr[phase-1]="1"
                groupPhaseStr = groupPhaseStr[::-1]
                groupPhaseStr = "".join(groupPhaseStr)
                groupPhaseInt = int(groupPhaseStr,2)
                
                return groupPhaseInt    

            # End the group at time which is minimum of all commands' end time. Additional commands will be added later in the code
            # for controls that end later.
            groupCommand = currentGroup[0].action
            groupStartTime = currentGroup[0].startTime
            groupEndTime = min(command.endTime for command in currentGroup )
            groupPhases = []
            for command in currentGroup:
                groupPhases = groupPhases + [command.phases]

            groupPhaseInt = formulateBinaryIntegerRepresentation(groupPhases)

            groupCommand = Command(groupPhaseInt, groupCommand, groupStartTime, groupEndTime)
            return groupCommand
            
        # Create Schedule data structure
        def createScheduleDataStructure(scheduleJson:json):
            """
            createScheduleDataStructure function takes a json string containing the schedule, and returns a list of CommandObjects.
            """
            scheduleDataStructure = []
            for command in scheduleJson:
                commandObject = Command(command["commandPhase"], command["commandType"],  command["commandStartTime"], command["commandEndTime"])
                scheduleDataStructure = scheduleDataStructure + [commandObject]

            return scheduleDataStructure
    
    ######################################### SUB-FUNCTIONS DEFINITION END ######################################### 

        scheduleJson = scheduleJson["Schedule"]

        # Delete the commands with invalid combination of startTime and EndTime
        scheduleJson = [i for i in scheduleJson if not ((i["commandStartTime"] == i["commandEndTime"]) or (i["commandStartTime"] > i["commandEndTime"]))] 
        
        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandStartTime"]))

        # Read the json into a data structure
        scheduleDataStructure = createScheduleDataStructure(scheduleJson)

        # Clear the old schedule from the Background Scheduler, and clear the require NTCIP commands.       
        clearOldSchedule(scheduleDataStructure)    

        # Form action group
        index = 0
        while index < len(scheduleDataStructure):
            
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type, and gather them together
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.action == currentGroup[0].action)):
                    currentGroup = currentGroup + [command]
                    index = index+1
            groupCommand =  formulateGroupCommand(currentGroup)

            # Check the end times of all items in the current group and add additional commands for the phases that end later.
            for command in currentGroup:
                if command.endTime > groupCommand.endTime:
                    scheduleDataStructure.append(Command(command.phases,command.action,groupCommand.endTime,command.endTime))

            # Find out the maximum end time when entire group command ends, and formulate a groupEndCommand (clear) at the maximum end time.
            groupMaxEndTime = max(command.endTime for command in currentGroup)
            groupEndCommand = Command(0,command.action,groupMaxEndTime,groupMaxEndTime)

            # Add the group end command at the endTime of the group only if it does not already exist in the scheduleDataStructure
            addEndCommandFlag = any((elem.phases==groupEndCommand.phases and elem.action==groupEndCommand.action and elem.startTime==groupEndCommand.startTime) for elem in scheduleDataStructure)
            if addEndCommandFlag == False:
                scheduleDataStructure.append(groupEndCommand)

            # Resort the schedule data structure based on command start time
            scheduleDataStructure.sort(key=lambda x: x.startTime, reverse=False)

            # Add the group command to the BackgroundScheduler
            self.addCommandToSchedule(groupCommand)
                
            # Clear the current group before the formulation of the next group.
            currentGroup = []
            index = index + 1
    
    '''##############################################
                    Scheduler Methods
    ##############################################'''
    
    def addCommandToSchedule(self, commandObject:Command):
        """
        addCommandToScgedule function takes a commandObject as an argument, and based on whether it is a clear command or setPhaseControl command,
        it schedules in the BackgroundScheduler. The function returns the ID of the scheduled command.
        arguments: (1) an object of the Command class.
        """
        # Assign Command ID:       
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        # The BackgroundScheduler does not support the tasks that need to be done NOW, as for such tasks, there is no need of using a scheduler.
        # For such tasks, that is for commands with startTime = 0.0, a small delay needs to be added.
        if commandObject.startTime == 0.0:
            commandObject.startTime = 0.001 

        # If the phase control needs to be cleared (command.phases == 0), 
        # then add a single instance of a function call that clears the phase control.
        if commandObject.phases == 0: 
            self.backgroundScheduler.add_job(self.signalController.setPhaseControl, args = [commandObject.action, commandObject.phases], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId
        
        else: # Then add a series of function calls starting at startTime, ending at endTime and separated by ntcipBackupTime.           
            
            self.backgroundScheduler.add_job(self.signalController.setPhaseControl, args = [commandObject.action, commandObject.phases], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1, # ADD EXPLANATION -> NTCIP BACKUPTIME MUST NOT BE 1.
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId

    def scheduleTimingPlanUpdate(self, interval:int):
        """
        scheduleTimingPlanUpdate takes in the interval as an argument, and for that interval, schedules the update of active timing plan.
        """
        self.backgroundScheduler.add_job(self.signalController.updateAndSendActiveTimingPlan,
                    trigger = 'interval',
                    seconds = interval,
                    id = str(self.commandId))
        return self.commandId

    def stopBackgroundScheduler(self):
        """
        stopBackgroundScheduler function first clears all jobs from the backgroundScheduler, clears all NTCIP commands in the signal controller, and then shuts down
        the backgroundScheduler.This function is intended to run at the exit.
        """
        
        # Clear all jobs from the BackgroundScheduler
        self.clearBackgroundScheduler(False)

        # Clear all phase controls from the traffic signal controller
        self.clearAllNtcipCommandsFromSignalController()
        
        # shut down the background scheudler
        self.backgroundScheduler.shutdown(wait=False)

    def clearAllNtcipCommandsFromSignalController(self):
        command = Command(0,0,0,0)
        # Clear VehCalls
        self.signalController.setPhaseControl(command.CALL_VEH_PHASES,0)
        # Clear PedCalls
        self.signalController.setPhaseControl(command.CALL_PED_PHASES,0)
        # Clear Forceoffs
        self.signalController.setPhaseControl(command.FORCEOFF_PHASES,0)
        # Clear Holds
        self.signalController.setPhaseControl(command.HOLD_VEH_PHASES,0)
        # Clear VehOmits
        self.signalController.setPhaseControl(command.OMIT_VEH_PHASES,0)
        # Clear PedOmits
        self.signalController.setPhaseControl(command.OMIT_PED_PHASES,0)
        
    def clearBackgroundScheduler(self, rescheduleTimingPlanUpdate:bool):
        """
        clearBackgroundScheduler clears all jobs from the BackgroundScheduler. 
        If the argument rescheduleTimingPlanUpdate is True, then adds the update of timing plan to the Background scheduler.
        Arguments: (1) Boolean to indicate whether TimingPlanUpdater needs to be scheduled after clearing the schedule.
        """
        self.backgroundScheduler.remove_all_jobs()
        if rescheduleTimingPlanUpdate==True:
            self.scheduleTimingPlanUpdate(self.signalController.timingPlanUpdateInterval_sec)

'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    import time

    asc = SignalController()

    # Create an object of Scheduler class
    scheduler = Scheduler(asc)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("schedule.json", "r")
    scheduleJson = json.loads(scheduleFile.read())

    scheduler.processReceivedSchedule(scheduleJson)

    # Schedule a vehicle call on all phases after 10 seconds
    # scheduler.addCommandToSchedule(Command(255,10,10,6))
    time.sleep(100)
