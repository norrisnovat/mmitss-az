from Phase import Phase

class MmitssPhase(Phase):
    def __init__(self, phaseNo:int):
        super().__init__()
        self.phaseNo = phaseNo
        self.currentCycle = 0
        self.previousPhaseNo = False
        self.previousPhaseClearanceTime = False
        self.initialGMinTimeToEnd = [False, False]
        self.initialGMaxTimeToEnd = [False, False]
        self.initialRMinTimeToEnd = [False, False]
        self.initialRMaxTimeToEnd = [False, False]
        self.omit:bool = False

if __name__=="__main__":
    mmitssPhase = MmitssPhase(1)
    pass