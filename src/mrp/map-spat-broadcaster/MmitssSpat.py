import time
from Ntcip1202v2Blob import Ntcip1202v2Blob
from Spat import Spat


UNKNOWN = 36001

class MmitssSpat(Spat):
    def __init__(self):
        super().__init__()
        self.reset()

    def reset(self):
        
        # Reset all internal variable
        super().__init__()
        self.isActive = False
        
        self.greenPhaseIndices = []
        self.yellowPhaseIndices = []
        self.redPhaseIndices = []

        self.omittedPhases = []

        self.gMaxEndTimes_cycle1 = []
        self.gMaxEndTimes_cycle2 = []
        
        self.gMinEndTimes_cycle1 = []
        self.gMinEndTimes_cycle2 = []
        
        self.rMaxEndTimes_cycle1 = []
        self.rMaxEndTimes_cycle2 = []
        
        self.rMinEndTimes_cycle1 = []
        self.rMinEndTimes_cycle2 = []

        self.servedAtleastOnce = []

        self.prevTimestamp = 0
        self.timestep = 0

        self.yellowStartPhaseIndices = []

    def initialize(self, scheduleSpatTranslation:dict):

        self.reset()
        self.isActive = True

        self.omittedPhases = scheduleSpatTranslation["OmittedPhases"]

        self.gMaxEndTimes_cycle1 = scheduleSpatTranslation["GreenStates"]["Cycle1"]["MaxEndTime"]
        self.gMaxEndTimes_cycle2 = scheduleSpatTranslation["GreenStates"]["Cycle2"]["MaxEndTime"]
        
        self.gMinEndTimes_cycle1 = scheduleSpatTranslation["GreenStates"]["Cycle1"]["MinEndTime"]
        self.gMinEndTimes_cycle2 = scheduleSpatTranslation["GreenStates"]["Cycle2"]["MinEndTime"]
        
        self.rMaxEndTimes_cycle1 = scheduleSpatTranslation["RedStates"]["Cycle1"]["MaxEndTime"]
        self.rMaxEndTimes_cycle2 = scheduleSpatTranslation["RedStates"]["Cycle2"]["MaxEndTime"]
        
        self.rMinEndTimes_cycle1 = scheduleSpatTranslation["RedStates"]["Cycle1"]["MinEndTime"]
        self.rMinEndTimes_cycle2 = scheduleSpatTranslation["RedStates"]["Cycle2"]["MinEndTime"]

        self.prevTimestamp = int(time.time() * 10)

        self.firstBlob = True
        

    def update(self, spatBlob:Ntcip1202v2Blob):
        currentTime = int(time.time() * 10)
        self.timestep = currentTime - self.prevTimestamp
        self.prevTimestamp = currentTime
        
        def deduct_timestep(dSecond):
            return max(dSecond-self.timestep, 0)

        if self.firstBlob == True:
            currentStates = spatBlob.getVehCurrState()
            for phaseIndex in range(len(currentStates)):
                if currentStates[phaseIndex]=="yellow":
                    self.yellowStartPhaseIndices += [phaseIndex]
        
            self.firstBlob = False

        else:
            if len(self.yellowStartPhaseIndices) != 0:
                currentStates = spatBlob.getVehCurrState()
                indicesToPop = []
                for phaseIndex in self.yellowStartPhaseIndices:
                    if currentStates[phaseIndex] != "yellow":
                        indicesToPop += [phaseIndex]

                for index in indicesToPop:
                    self.yellowStartPhaseIndices.remove(index)
                    

        
        


        # Update internal variables
        self.gMaxEndTimes_cycle1 = list(map(deduct_timestep, self.gMaxEndTimes_cycle1))
        self.gMaxEndTimes_cycle2 = list(map(deduct_timestep, self.gMaxEndTimes_cycle2))
        
        self.gMinEndTimes_cycle1 = list(map(deduct_timestep, self.gMinEndTimes_cycle1))
        self.gMinEndTimes_cycle2 = list(map(deduct_timestep, self.gMinEndTimes_cycle2))
        
        self.rMaxEndTimes_cycle1 = list(map(deduct_timestep, self.rMaxEndTimes_cycle1))
        self.rMaxEndTimes_cycle2 = list(map(deduct_timestep, self.rMaxEndTimes_cycle2))
        
        self.rMinEndTimes_cycle1 = list(map(deduct_timestep, self.rMinEndTimes_cycle1))
        self.rMinEndTimes_cycle2 = list(map(deduct_timestep, self.rMinEndTimes_cycle2))

        # Update current states of vehicle phases
        vehCurrStateList = super().getVehCurrStateList(spatBlob)
        self.greenPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "green"]
        self.redPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "red"]
        self.yellowPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "yellow"]
        for phaseIndex in self.yellowPhaseIndices:
            if phaseIndex not in self.yellowStartPhaseIndices:
                if phaseIndex+1 not in self.servedAtleastOnce:
                    self.servedAtleastOnce += [phaseIndex+1]

    def getVehMinTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMinEndTimeList = [UNKNOWN for phase in range(8)]
        for phaseIndex in range(len(8)):
            if phaseIndex+1 not in self.omittedPhases: # Do the following only for non-omitted phases
                if phaseIndex in self.greenPhaseIndices:
                    # Substitute with the value from gMinEndTimes_cycle1 if it is not yet served
                    if phaseIndex+1 in self.servedAtleastOnce:
                        vehMinEndTimeList[phaseIndex] = self.gMinEndTimes_cycle1[phaseIndex]
                    
                    # Substitute with the value from gMinEndTimes_cycle2 if it is already served
                    else: vehMinEndTimeList[phaseIndex] = self.gMinEndTimes_cycle2[phaseIndex]

                elif phaseIndex in self.yellowPhaseIndices:
                    # Substitute with the value from the original MinEndTime
                    vehMinEndTimeList[phaseIndex] = super().getVehMinEndTimeList(spatBlob)[phaseIndex]

                elif phaseIndex in self.redPhaseIndices:
                    # Substitute with the value from rMinEndTimes_cycle1 if it is not yet served
                    if phaseIndex+1 in self.servedAtleastOnce:
                        vehMinEndTimeList[phaseIndex] = self.rMinEndTimes_cycle1[phaseIndex]
                    
                    # Substitute with the value from rMinEndTimes_cycle2 if it is already served
                    else: vehMinEndTimeList[phaseIndex] = self.rMinEndTimes_cycle2[phaseIndex]
            else: vehMinEndTimeList[phaseIndex] = UNKNOWN

        return vehMinEndTimeList

    def getVehMaxTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMaxEndTimeList = [UNKNOWN for phase in range(8)]
        for phaseIndex in range(len(8)):
            if phaseIndex+1 not in self.omittedPhases: # Do the following only for non-omitted phases
                if phaseIndex in self.greenPhaseIndices:
                    # Substitute with the value from gMaxEndTimes_cycle1 if it is not yet served
                    if phaseIndex+1 in self.servedAtleastOnce:
                        vehMaxEndTimeList[phaseIndex] = self.gMaxEndTimes_cycle1[phaseIndex]
                    
                    # Substitute with the value from gMaxEndTimes_cycle2 if it is already served
                    else: vehMaxEndTimeList[phaseIndex] = self.gMaxEndTimes_cycle2[phaseIndex]

                elif phaseIndex in self.yellowPhaseIndices:
                    # Substitute with the value from the original MinEndTime
                    vehMaxEndTimeList[phaseIndex] = super().getVehMaxEndTimeList(spatBlob)[phaseIndex]

                elif phaseIndex in self.redPhaseIndices:
                    # Substitute with the value from rMaxEndTimes_cycle1 if it is not yet served
                    if phaseIndex+1 in self.servedAtleastOnce:
                        vehMaxEndTimeList[phaseIndex] = self.rMaxEndTimes_cycle1[phaseIndex]
                    
                    # Substitute with the value from rMaxEndTimes_cycle2 if it is already served
                    else: vehMaxEndTimeList[phaseIndex] = self.rMaxEndTimes_cycle2[phaseIndex]
            else: vehMaxEndTimeList[phaseIndex] = UNKNOWN

        return vehMaxEndTimeList

    def getPedMinEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        return [UNKNOWN for phase in range(8)]

    def getPedMaxEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        return [UNKNOWN for phase in range(8)]

if __name__=="__main__":
    import json

    filename = "test/scheduleSpatTranslation_nonev.json"

    with open(filename, 'r') as fp:
        scheduleSpatTranslation = json.load(fp)

    mmitssSpat = MmitssSpat()

    mmitssSpat.initialize(scheduleSpatTranslation)
    mmitssSpat.update()
    mmitssSpat.deactivate()
    pass


