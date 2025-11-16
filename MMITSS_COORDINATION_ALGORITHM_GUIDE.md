# 🚦 MMITSS Coordination Algorithm - Complete Technical Guide

## Quick Summary

**MMITSS** = Multi-Modal Intelligent Traffic Signal System

**Coordination Algorithm**: Dynamically optimizes traffic signal timing every 4 seconds based on vehicle priority requests, current signal status, and coordination parameters to minimize system-wide delays while maintaining inter-intersection synchronization.

---

## Core Components & Data Flow

```
Vehicles (GPS/V2I)
    ↓
Priority Request Server
    ↓
Signal Coordination Request Generator (manages coordination plans)
    ↓
Priority Request Solver (optimization engine - GLPK solver)
    ↓
Traffic Controller Interface (TCI)
    ↓
Physical Traffic Signal Controller
```

---

## 6-Phase Algorithm Execution

### Phase 1: Data Collection & Aggregation
- **PRS** receives vehicle priority requests with ETAs
- Filters duplicate requests (same phase, same vehicle type)
- Current signal status captured (which phase active, elapsed time)
- Maximum 15 simultaneous requests supported

**Vehicle Types:**
- Emergency (Weight: 10) - Ambulance, Fire, Police
- Transit (Weight: 5) - Bus, light rail
- Truck (Weight: 3) - Heavy vehicles
- Coordination (Weight: 1) - Virtual vehicles for signal sync
- Normal (Weight: 0) - Regular traffic

### Phase 2: Check for Coordination Plans
- **SCRG** (Signal Coordination Request Generator) provides:
  - Cycle length (e.g., 90 seconds)
  - Offset (e.g., 10 seconds from cycle start)
  - Coordinated phases (e.g., Phase 2 and Phase 6)
  - Split data (duration for each coordinated phase)

**Green Wave Concept:**
```
Int 1: Phase 2 green, T=0-20s    → Vehicle gets green
Int 2: Phase 2 green, T=20-40s   → Vehicle continues green
Int 3: Phase 2 green, T=40-60s   → Vehicle continues green
Int 4: Phase 2 green, T=60-80s   → Vehicle continues green
```

### Phase 3: Build Optimization Model
**SolverDataManager** creates data file with:
- Starting phases
- Current green time elapsed
- Yellow and red clearance intervals (safety)
- Min/max green times per phase (constraints)
- Priority weights for each request
- ETA time windows (when vehicle can/must arrive)
- Coordination split constraints

**Example OptimizationModelData.dat:**
```
param SP1 := 4;              # Starting phase
param Grn1 := 15;            # Current green time
param y  := 1 3 2 4 3 3 ...  # Yellow intervals
param red:= 1 1 2 2.5 3 1 .. # Red clearance
param gmin:= 1 4 2 15 ...    # Minimum green (safety)
param gmax:= 1 13 2 35 ...   # Maximum green (coordination)
param priorityType:= 1 2 2 5 ... # Vehicle types
param PrioWeight:= 1 0 2 0.9 ... # Priority weights
```

### Phase 4: Solve Optimization Problem
**Uses GNU GLPK** (Linear Programming solver)

Two Models:
1. **OptimizationModel.mod** - Standard (Transit/Truck/Coordination)
   - Objective: Minimize total weighted delay
   
2. **OptimizationModel_EV.mod** - Emergency Special
   - Objective: Minimize emergency vehicle delay
   - Includes dilemma zone constraints

**Variables Solved:**
- Hold times (keep phase green)
- Force-off times (end phase, start yellow)
- Green durations for each phase (3 cycles lookahead)

**Output: OptimizationResults.txt**
- Hold/Force-off points for 3 future cycles
- Optimized green times
- Vehicle arrival times and delays
- Objective value (total delay minimized)

### Phase 5: Create Optimal Schedule
**ScheduleManager** converts optimization output to commands:

```json
{
  "Schedule": [
    {
      "commandPhase": 2,
      "commandType": "hold",
      "commandStartTime": 0.0,
      "commandEndTime": 5.0
    },
    {
      "commandPhase": 2,
      "commandType": "call_veh",
      "commandStartTime": 0.0,
      "commandEndTime": 35.07
    },
    {
      "commandPhase": 2,
      "commandType": "forceoff",
      "commandStartTime": 35.07,
      "commandEndTime": 39.07
    }
  ]
}
```

**Command Types:**
- **HOLD** - Keep current phase green
- **FORCE-OFF** - End phase (activate yellow)
- **CALL_VEH** - Extend green for vehicle request
- **OMIT** - Skip phase in this cycle

### Phase 6: Execute Commands
**TCI** sends commands to physical signal controller via UDP socket
- Receives back current signal status
- Cycle time: ~100-200ms updates

---

## Virtual Coordination Requests

**Problem:** How to maintain "green waves" without real vehicles?

**Solution:** Generate virtual priority requests for coordinated phases

**How it works:**
```
Cycle Start T=0
├─ Virtual Vehicle #1
│  ├─ Phase: 2 (coordinated phase)
│  ├─ ETA: 10 seconds (offset)
│  ├─ Type: Coordination (vehicleType=20)
│  └─ Split: 20 seconds (how long phase should run)
├─ Virtual Vehicle #2
│  ├─ Phase: 6 (coordinated phase)
│  ├─ ETA: 10 seconds
│  ├─ Type: Coordination
│  └─ Split: 25 seconds
├─ Virtual Vehicle #3 (next cycle)
│  ├─ Phase: 2
│  ├─ ETA: 100 seconds (10 + 90 cycle length)
│  └─ Split: 20 seconds
└─ Virtual Vehicle #4 (next cycle)
   ├─ Phase: 6
   ├─ ETA: 100 seconds
   └─ Split: 25 seconds
```

**Update Strategy:**
- ETA updated every cycle to reflect current time
- Sent to solver every 8 seconds (to avoid PRS timeout)
- Deleted when coordination split becomes 0

---

## Real Operation Example

### Scenario: Transit Bus Approaching

**WITHOUT MMITSS (Fixed Timing):**
```
T=0:  Bus 12 blocks away, current phase: 1 (green)
T=30: Phase 1 ends
T=31: Phase 2 starts (bus's needed phase)
T=35: Bus arrives (delayed)
Total delay: 35+ seconds
```

**WITH MMITSS COORDINATION:**
```
T=0:    Bus sends priority request (Phase 2, ETA=10s)
        Current: Phase 1 (25s elapsed, 20s remaining)
        
T=0.05: Solver receives request
        Decision: Extend Phase 1 by 5s, then start Phase 2 immediately
        
T=1:    Commands sent to TCI:
        - Hold Phase 1 for 5 more seconds
        - Vehicle call on Phase 2
        
T=10:   Phase 2 goes green, bus enters green light
        Total delay: ~0 seconds (OPTIMAL!)
        
Impact: Phase 4 delayed ~5s, overall system better by 10+ seconds
```

---

## Priority Weighting Scheme

How does the solver decide which requests to serve?

**Objective Function:**
```
Minimize: Σ (priority_weight × vehicle_delay)

Where:
  Emergency vehicle weight = 10 (HIGHEST)
  Transit weight = 5 (HIGH)
  Truck weight = 3 (MEDIUM)
  Coordination weight = 1 (BASELINE)
  Normal vehicles = 0 (LOWEST)
```

**Result:**
- Emergency vehicles get fastest green (< 10 seconds)
- Transit gets next priority (5-10 second response)
- Trucks get moderate extension
- Coordination maintained at baseline
- Normal traffic serves remaining time

---

## Dilemma Zone Management

**Definition:** Region where driver cannot safely stop or proceed

**Safety Handling:**
```
IF emergency vehicle request WHILE trucks in dilemma zone:
  1. Detect trucks in dilemma zone
  2. Hold red signal to let trucks clear
  3. Prevent dangerous sudden red
  4. Then serve emergency request
  5. Minimize emergency delay while ensuring safety
```

---

## Coordination Parameters (Configuration)

### Port Numbers (UDP Communication)
```
PrioritySolver:              5000  (receives priority requests)
TrafficControllerInterface:  5001  (sends/receives TCI commands)
SignalCoordination:          5002  (receives split data)
```

### Timing Parameters
```
CoordinationPlanCheckingInterval:  Check for new plans periodically
ETA_Delete_Time:                   1 second (remove old requests)
SRM_TIME_GAP_VALUE:                8 seconds (update sending interval)
```

### Priority Weights (Configurable)
```
EV_Weight:                 10.0  (emergency vehicle)
EV_SplitPhase_Weight:       0.5  (coordination during emergency)
Transit_Weight:             0.9  (transit vehicle)
Truck_Weight:               0.2  (truck vehicle)
Coordination_Weight:        1.0  (base coordination)
DZ_Request_Weight:          2.0  (dilemma zone protection)
```

---

## Timing: 4-Second Optimization Cycle

```
T=0ms    | Receive priority requests
T=50ms   | Get current signal status from TCI
T=100ms  | Build optimization data file
T=150ms  | Call GLPK solver
T=300ms  | GLPK returns optimal solution
T=400ms  | Parse results, create schedule
T=450ms  | Send schedule to TCI
T=500ms  | TCI executes commands
T=1000ms | (cycle repeats ~ every 4 seconds)
```

**Why 4 seconds?**
- Fast enough for responsive vehicle service
- Slow enough for GLPK solver computation
- Gives TCI time to execute commands safely
- Balances responsiveness vs stability

---

## Constraints Enforced by Solver

✓ Green time within min/max bounds (safety)
✓ Yellow and red clearance intervals maintained
✓ Cannot skip required phases (ring-barrier model)
✓ Coordination splits honored when active
✓ All 15 max requests evaluated
✓ 3-cycle lookahead planning

---

## Output Data Files

### 1. OptimizationModelData.dat
Input parameters to GLPK solver
- Current signal status
- Static timing plan
- Priority request info
- ETA time windows

### 2. OptimizationResults.txt
Solution from GLPK
- Hold/force-off points (3 cycles)
- Green times per phase
- Vehicle delays
- Objective value (total delay)

### 3. Optimal Schedule (JSON)
Final commands sent to physical signal
- Command type (hold/forceoff/call_veh/omit)
- Phase number
- Start/end times

---

## Key Advantages

1. **Dynamic Optimization**: Real-time response to traffic changes
2. **Priority Service**: Emergency vehicles served < 10 seconds
3. **Coordination Maintained**: Green waves preserved across intersections
4. **Safety First**: Dilemma zone protection, yellow/red constraints
5. **Scalable**: Handles 15+ simultaneous requests
6. **Resilient**: Graceful degradation without external systems
7. **Research-Based**: Built on proven optimization algorithms

---

## Research & Development

- **Institution**: University of Arizona
- **Department**: Systems and Industrial Engineering
- **Lead**: Professor Larry Head
- **Funding**: USDOT (US Department of Transportation)
- **License**: Apache 2.0 Open Source
- **Technology**: Mixed Integer Programming (MIP) with GNU GLPK

---

## Common Configuration Pattern

```json
{
  "CoordinationPattern": [
    {
      "CoordinationPlanName": "Plan1",
      "CoordinationPatternNo": 1,
      "CycleLength": 90,
      "Offset": 10,
      "CoordinatedPhase1": 2,
      "CoordinatedPhase2": 6,
      "CoordinationSplit": 20.0,
      "Split": [25, 37, 0, 38, 24, 38, 0, 38]
    }
  ],
  "CoordinationDayPlan": [
    {
      "Days": ["Monday", "Tuesday", "Wednesday"],
      "ActionPlan": [
        {
          "CoordinationPatternNo": 1,
          "CoordinationStartTime_Hour": 7,
          "CoordinationStartTime_Minute": 30,
          "CoordinationEndTime_Hour": 10,
          "CoordinationEndTime_Minute": 30
        }
      ]
    }
  ]
}
```

---

## System Status (Your Environment)

✅ **Core Services Running:**
- M_MsgEncoder - Message handling
- M_PriorityRequestServer - Request management
- M_PriorityRequestSolver - Optimization
- M_SystemInterface - Core hub
- M_TrajectoryAware - Vehicle prediction
- M_WirelessMsgDecoder - V2I communication

🔴 **External Services (Need Hardware):**
- M_MapSpatBroadcaster - Needs RSU hardware
- M_SignalCoordinationRequestGenerator - Needs coordination server
- M_TrafficControllerInterface - Needs physical controller
- M_V2XDataCollector - Needs V2X network

⏸️ **Optional:**
- M_SnmpEngine - SNMP monitoring
- M_TimePhaseDiagramTool - Signal visualization

---

## Next Steps

1. **Monitor Dashboards**: Open all 4 intersection Supervisord interfaces
2. **Understand Flows**: Review Priority Request Server and Solver docs
3. **Test Scenarios**: Simulate different priority request combinations
4. **Configure Coordination**: Set up coordination plans in JSON files
5. **Optimize Weights**: Tune priority weights for your traffic patterns

---

## Reference Documentation

- `/src/mrp/signal-coordination-request-generator/README.md` - Coordination details
- `/src/mrp/priority-request-solver/README.md` - Solver details
- `/src/mrp/priority-request-server/README.md` - Request management
- Configuration: `mmitss-phase3-master-config.json`

