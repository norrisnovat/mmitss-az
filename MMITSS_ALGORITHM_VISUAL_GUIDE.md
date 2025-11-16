# 🚦 MMITSS Algorithm - Visual Reference Card

## The Complete Flow in One Picture

```
╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║              🚗 VEHICLE SENDS PRIORITY REQUEST (GPS/V2I)                  ║
║                      ↓                                                     ║
║       ┌─────────────────────────────────────────┐                        ║
║       │  Priority Request Server (PRS)          │                        ║
║       │                                         │                        ║
║       │  Input:                                 │                        ║
║       │  • Vehicle position                     │                        ║
║       │  • ETA (Est. arrival time)              │                        ║
║       │  • Vehicle type                         │                        ║
║       │  • Requested phase                      │                        ║
║       │                                         │                        ║
║       │  Output:                                │                        ║
║       │  • Priority request list (max 15)       │                        ║
║       │  • Duplicate filtered                   │                        ║
║       └─────────────────────────────────────────┘                        ║
║                      ↓                                                     ║
║    ┌────────────────────────────────────────────────────────┐            ║
║    │  Signal Coordination Request Generator (SCRG)          │            ║
║    │                                                        │            ║
║    │  Checks: Is there active coordination plan?           │            ║
║    │  ├─ If YES: Get split data                            │            ║
║    │  │  • Cycle length                                    │            ║
║    │  │  • Offset                                          │            ║
║    │  │  • Coordinated phases                              │            ║
║    │  │  • Split times for each phase                      │            ║
║    │  │                                                    │            ║
║    │  └─ Create Virtual Coordination Requests              │            ║
║    │     • 4 virtual vehicles (2 phases × 2 cycles)        │            ║
║    │     • Maintain "green wave" pattern                   │            ║
║    │     • Sent to solver every 8 seconds                  │            ║
║    └────────────────────────────────────────────────────────┘            ║
║                      ↓                                                     ║
║  ┌──────────────────────────────────────────────────────────────┐        ║
║  │  Priority Request Solver (Optimization Engine)              │        ║
║  │                                                              │        ║
║  │  INPUTS:                                                    │        ║
║  │  ├─ Priority request list (PRS)                            │        ║
║  │  ├─ Virtual coordination requests (SCRG)                   │        ║
║  │  ├─ Current signal status (TCI)                            │        ║
║  │  ├─ Static timing plan (Yellow, Red, MinGreen, MaxGreen)   │        ║
║  │  └─ Configuration weights (Emergency, Transit, etc.)       │        ║
║  │                                                              │        ║
║  │  PROCESS:                                                   │        ║
║  │  1. SolverDataManager builds optimization data              │        ║
║  │     OptimizationModelData.dat                              │        ║
║  │                                                              │        ║
║  │  2. GLPK Solver solves Mixed Integer Program               │        ║
║  │     • Minimize: Weighted vehicle delays                    │        ║
║  │     • Subject to: Safety & coordination constraints        │        ║
║  │     • Solution: Optimal green times (3 cycles lookahead)   │        ║
║  │                                                              │        ║
║  │  3. ScheduleManager converts to commands                   │        ║
║  │     OptimizationResults.txt → Schedule JSON                │        ║
║  │                                                              │        ║
║  │  OUTPUTS:                                                   │        ║
║  │  ├─ Hold times per phase                                   │        ║
║  │  ├─ Force-off times per phase                              │        ║
║  │  ├─ Optimal green durations                                │        ║
║  │  └─ Vehicle call extensions                                │        ║
║  │                                                              │        ║
║  │  COMMAND TYPES:                                            │        ║
║  │  • HOLD - Keep phase green                                 │        ║
║  │  • FORCE-OFF - End phase, start yellow                     │        ║
║  │  • CALL_VEH - Extend green for vehicle                     │        ║
║  │  • OMIT - Skip phase this cycle                            │        ║
║  └──────────────────────────────────────────────────────────────┘        ║
║                      ↓                                                     ║
║       ┌──────────────────────────────┐                                    ║
║       │  Traffic Controller Interface│                                    ║
║       │  (TCI)                       │                                    ║
║       │                              │                                    ║
║       │  Sends: Optimized schedule   │                                    ║
║       │  Receives: Signal status     │                                    ║
║       │  Cycle: Every 4 seconds      │                                    ║
║       └──────────────────────────────┘                                    ║
║                      ↓                                                     ║
║       ┌──────────────────────────────┐                                    ║
║       │  Physical Traffic Signal     │                                    ║
║       │  Controller (Hardware)       │                                    ║
║       │                              │                                    ║
║       │  Executes commands:          │                                    ║
║       │  • Changes signal phases     │                                    ║
║       │  • Extends greens            │                                    ║
║       │  • Maintains safety          │                                    ║
║       └──────────────────────────────┘                                    ║
║                      ↓                                                     ║
║            🚗 VEHICLE PASSES EFFICIENTLY                                  ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝
```

---

## Coordination Timeline Example

```
Coordination Pattern:
  Cycle Length: 90 seconds
  Offset: 10 seconds
  Phase 2: 20 second split
  Phase 6: 25 second split

CYCLE 1 (T=0 to T=90)
├─ T=0-10    Phase 4 (through)
├─ T=10-30   Phase 2 GREEN (COORDINATED) ← Virtual vehicles request this
├─ T=30-40   Phase 1 (through)
├─ T=40-65   Phase 6 GREEN (COORDINATED) ← Virtual vehicles request this
├─ T=65-75   Phase 5 (through)
└─ T=75-90   Phase 8 (through)

CYCLE 2 (T=90 to T=180)
├─ T=100-120 Phase 2 GREEN (COORDINATED) ← Same as before
├─ T=135-160 Phase 6 GREEN (COORDINATED) ← Same as before
└─ ...

GREEN WAVE ACROSS INTERSECTIONS:
Intersection A: Phase 2 green T=0-20    ✅ Vehicle at right speed
Intersection B: Phase 2 green T=30-50   ✅ Vehicle continues
Intersection C: Phase 2 green T=60-80   ✅ Vehicle continues
Intersection D: Phase 2 green T=90-110  ✅ Vehicle continues
                                        → NO STOPS! 🎉
```

---

## Priority Request Processing

```
Vehicle Priority Levels:

EMERGENCY (Weight: 10)
├─ Ambulance
├─ Fire Truck
└─ Police
    → Gets green in < 10 seconds
    → Can override coordination

TRANSIT (Weight: 5)
├─ Bus
└─ Light Rail
    → Gets green in 5-15 seconds
    → Respects coordination splits

TRUCK (Weight: 3)
└─ Heavy commercial
    → Gets green in 10-30 seconds
    → Lower priority than transit

COORDINATION (Weight: 1)
└─ Virtual vehicles
    → Maintains signal sync
    → Lowest priority

NORMAL (Weight: 0)
└─ Regular traffic
    → Uses remaining green time
```

---

## Real-Time Decision Making

```
SCENARIO: Bus arriving while Truck in transit

T=0:
  Current: Phase 1 (green) - 15s elapsed, 20s remaining
  Request: Transit bus needs Phase 2 (ETA = 12s)
  
Decision Analysis:
  Option A: Let Phase 1 finish (20s more)
    Bus delay: ~20 seconds
    Cost: 20 × 5 (transit weight) = 100
    
  Option B: Force Phase 1 off at T=8s, start Phase 2 early
    Bus delay: ~0 seconds  
    Phase 1 vehicles delay: ~8 seconds
    Cost: 0 × 5 + 8 × 0 = 0
    
SOLVER CHOOSES: Option B ✓
    → Better overall system performance
```

---

## 4-Second Optimization Cycle

```
TimelineActual (approximate):

0ms   |----------→ Receive priority requests from PRS
      |
50ms  |-------→ Request current signal status
      |
100ms |---→ Build optimization data file (OptimizationModelData.dat)
      |
150ms |---→ Call GLPK solver
      |
150-300ms | GLPK Computing...
      |   (Solving MIP problem)
      |
300ms |---→ GLPK returns solution (OptimizationResults.txt)
      |
400ms |---→ Parse results, build schedule
      |
450ms |---→ Send JSON schedule to TCI
      |
500ms |---→ TCI executes commands on signal
      |
      | Monitor & Log
      |
1000ms|---→ Repeat cycle (all cycles run ~every 4 seconds)
```

---

## Data Structure: Priority Request

```json
{
  "requestId": 1,
  "vehicleType": 5,              // 2=Emergency, 5=Transit, 8=Truck, 20=Coordination
  "vehicleId": 42,
  "requestedPhase": 2,           // Which signal phase needed
  "ETA": 12.5,                   // Seconds until arrival at intersection
  "ETA_Lower": 8.0,              // Earliest acceptable arrival
  "ETA_Upper": 25.0,             // Latest acceptable arrival
  "priorityWeight": 0.9,         // 10=Emergency, 5=Transit, 3=Truck, 1=Coordination
  "timestamp": 1731700000.5,     // When request sent
  "basicVehicleRole": 11,        // Vehicle classification
  "coordinationSplit": 0.0       // For coordination: how long phase should run
}
```

---

## Data Structure: Optimization Result

```json
{
  "Schedule": [
    {
      "commandPhase": 2,
      "commandType": "hold",
      "commandStartTime": 0.0,
      "commandEndTime": 5.0,
      "reason": "Hold current phase for transit bus"
    },
    {
      "commandPhase": 2,
      "commandType": "call_veh",
      "commandStartTime": 0.0,
      "commandEndTime": 35.07,
      "reason": "Extend green for priority vehicle"
    },
    {
      "commandPhase": 2,
      "commandType": "forceoff",
      "commandStartTime": 35.07,
      "commandEndTime": 39.07,
      "reason": "Yellow change + red clearance"
    }
  ],
  "objectiveValue": 45.23,       // Total weighted delay minimized
  "solverStatus": "optimal",
  "computationTime": 0.15        // seconds
}
```

---

## Green Wave Visualization

```
COORDINATED CORRIDOR (4 intersections, main arterial)

Time: 0-30s   Phase timing in all intersections

  Int 1 Phase 2:  ▓▓▓▓▓▓▓▓▓▓ (green, 20s)
  Int 2 Phase 2:  ░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓ (offset 10s, green 20s)
  Int 3 Phase 2:  ░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓ (offset 20s, green 20s)
  Int 4 Phase 2:  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░▓▓▓▓▓▓▓▓▓▓ (offset 30s, green 20s)
  
Northbound Vehicle:
  T=0-10s:   Drives through Int 1 (green) ✅
  T=10-20s:  Drives through Int 2 (green) ✅
  T=20-30s:  Drives through Int 3 (green) ✅
  T=30-40s:  Drives through Int 4 (green) ✅
  
Result: ZERO STOPS! Perfect "green wave" 🌊
```

---

## Constraints Enforced by Solver

```
Safety Constraints:
├─ Yellow interval maintained (typically 3-4 seconds)
├─ Red clearance maintained (typically 1-3 seconds)
├─ No phase can be too short (min green = 4-15 seconds)
└─ No phase can be too long (max green = 15-90 seconds)

Coordination Constraints:
├─ Coordinated phases start at correct offset times
├─ Coordination splits honored (phase duration fixed)
├─ Ring-barrier constraints (phases must follow sequence)
└─ Can't skip required phases

Priority Constraints:
├─ Emergency vehicle receives priority
├─ Transit vehicle gets reasonable service
├─ Coordination maintained even with priority requests
└─ Dilemma zone protected from unsafe phase changes

Practical Constraints:
├─ Maximum 15 simultaneous priority requests
├─ 3-cycle lookahead planning
├─ Solution must be achievable by hardware
└─ Computation must complete in < 1 second
```

---

## Optimization Objective Function

```
Minimize Z = Σ(weight_i × delay_i)

where for each request i:
  weight_i = 10  (if Emergency)
           = 5   (if Transit)
           = 3   (if Truck)
           = 1   (if Coordination)
           = 0   (if Normal)
  
  delay_i = max(0, actual_arrival_i - desired_arrival_i)

Subject to:
  • All constraints above
  • Signal timing plan feasibility
  • Controller hardware capabilities

Result: Signal timing that minimizes weighted vehicle delays
        while maintaining safety and coordination
```

---

## System Status Indicators

```
✅ HEALTHY (6 Services Running)
  • M_MsgEncoder - ✓ Encoding/Decoding
  • M_PriorityRequestServer - ✓ Receiving requests
  • M_PriorityRequestSolver - ✓ Optimizing
  • M_SystemInterface - ✓ Central hub
  • M_TrajectoryAware - ✓ Predicting
  • M_WirelessMsgDecoder - ✓ V2I comms

🔴 EXPECTED TO FAIL (4 Services - No External Hardware)
  • M_MapSpatBroadcaster - Needs RSU hardware
  • M_SignalCoordinationRequestGenerator - Needs coordination server
  • M_TrafficControllerInterface - Needs physical controller
  • M_V2XDataCollector - Needs V2X network

🟡 OPTIONAL (2 Services)
  • M_SnmpEngine - SNMP monitoring (stopped)
  • M_TimePhaseDiagramTool - Visualization (running)

Overall: System is OPERATIONAL with core services active! ✅
```

---

## Quick Debugging Guide

```
Problem: Solver not producing solution
├─ Check: Is PRSolver receiving requests? (supervisord tail -f)
├─ Check: Is TCI sending signal status? (check logs)
├─ Check: OptimizationModelData.dat exists and valid?
└─ Fix: Verify all inputs present, check GLPK version

Problem: Coordination timing off
├─ Check: Is SCRG running? (supervisord status)
├─ Check: Active coordination plan configured?
├─ Check: Split data valid? (check JSON format)
└─ Fix: Review coordination plan in config file

Problem: Vehicle requests not served
├─ Check: PRS running? (supervisord status)
├─ Check: Vehicle sending request properly?
├─ Check: ETA in correct time window?
└─ Fix: Check request weight configuration

Problem: Safety violation (wrong phase on)
├─ Check: Yellow/red clearance values
├─ Check: Min/max green constraints
├─ Check: Ring-barrier model correct
└─ Fix: Verify signal controller type, update constraints
```

---

## Key Takeaways

1. **Every 4 seconds**: Solver optimizes timing for current traffic
2. **3 priorities**: Emergency (weight 10), Transit (weight 5), Truck (weight 3)
3. **Coordination maintained**: Virtual vehicles ensure green waves
4. **Safety first**: Yellow/red constraints, dilemma zone protection
5. **Scalable**: Handles up to 15 simultaneous requests
6. **Open source**: GLPK solver, Apache 2.0 license
7. **Research based**: University of Arizona, USDOT funded

---

## For Further Study

- `src/mrp/signal-coordination-request-generator/` - Coordination logic
- `src/mrp/priority-request-solver/` - Solver implementation
- `src/mrp/priority-request-server/` - Request management
- `/nojournal/bin/supervisord.conf` - Service configuration
- `mmitss-phase3-master-config.json` - System configuration

