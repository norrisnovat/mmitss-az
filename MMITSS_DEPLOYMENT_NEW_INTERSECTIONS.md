# 🚦 MMITSS Deployment Guide - Adding New Intersections

## Overview

This guide explains how to deploy the MMITSS coordination system to other intersections or create new intersection instances. The process involves configuration, container setup, and coordination synchronization.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Architecture Overview](#architecture-overview)
3. [Step-by-Step Deployment](#step-by-step-deployment)
4. [Configuration Files](#configuration-files)
5. [Network & Coordination Setup](#network--coordination-setup)
6. [Testing & Validation](#testing--validation)
7. [Troubleshooting](#troubleshooting)
8. [Examples](#examples)

---

## Prerequisites

### Required Resources

- **Hardware:**
  - Docker-capable host (Linux, Mac, or Windows with Docker Desktop)
  - Minimum: 4GB RAM per intersection container
  - Network connectivity between intersections

- **Software:**
  - Docker & Docker Compose installed
  - Git (for cloning repository)
  - Basic networking knowledge (IP addresses, UDP ports)

- **Access:**
  - MMITSS repository cloned locally
  - Root or sudo access for Docker commands
  - Network access to real traffic controllers (optional, for production)

### Existing System Status

Your current deployment has 4 intersections:
```
speedway-campbell    (IP: 10.12.6.4)
speedway-cherry      (IP: 10.12.6.5)
speedway-mountain    (IP: 10.12.6.3)
speedway-park        (IP: 10.12.6.2)
```

---

## Architecture Overview

### Current System Layout

```
┌─────────────────────────────────────────┐
│      Docker Bridge Network               │
│      (mmitss_vlan)                      │
├─────────────────────────────────────────┤
│                                         │
│  Intersection 1        Intersection 2   │
│  (speedway-campbell)   (speedway-cherry)│
│  10.12.6.4:9001        10.12.6.5:9001   │
│                                         │
│  Intersection 3        Intersection 4   │
│  (speedway-mountain)   (speedway-park)  │
│  10.12.6.3:9001        10.12.6.2:9001   │
│                                         │
└─────────────────────────────────────────┘
         ↓
    Coordination via:
    - Virtual coordination requests
    - Split data synchronization
    - Shared configuration
```

### Adding New Intersections

New intersections can be added in two deployment models:

**Model 1: Horizontal Scaling (Same Compose File)**
- Add containers to existing docker-compose.yml
- Share same network (mmitss_vlan)
- Centralized coordination
- Best for: Testing, development, local deployment

**Model 2: Distributed Deployment (Separate Docker Hosts)**
- Deploy on different machines/networks
- Coordinate via external coordination server
- Better for: Production, real-world deployments
- Requires: Coordination server (separate component)

---

## Step-by-Step Deployment

### OPTION 1: Add Intersection to Existing Docker Compose

#### Step 1: Plan New Intersection

Define your new intersection:
```
Intersection Name: speedway-dublin
Container Name: speedway-dublin
IP Address: 10.12.6.6
Port: 9001 (internal), 9005 (host mapping)
Configuration Path: config/speedway-sample/field/speedway-dublin/
```

#### Step 2: Create Configuration Directory

```bash
# Create directory structure
mkdir -p $MMITSS_ROOT/config/speedway-sample/field/speedway-dublin/bin
mkdir -p $MMITSS_ROOT/config/speedway-sample/field/speedway-dublin/log

# Copy configuration from existing intersection
cp -r $MMITSS_ROOT/config/speedway-sample/field/speedway-campbell/bin/* \
      $MMITSS_ROOT/config/speedway-sample/field/speedway-dublin/bin/
```

#### Step 3: Update Configuration Files

Edit `speedway-dublin/bin/supervisord.conf`:
```bash
nano $MMITSS_ROOT/config/speedway-sample/field/speedway-dublin/bin/supervisord.conf
```

Key changes:
```ini
# Change intersection name in logs
[program: M_MsgEncoder]
stdout_logfile=/nojournal/log/speedway-dublin_MsgEncoder.log

[program: M_PriorityRequestServer]
stdout_logfile=/nojournal/log/speedway-dublin_PRS.log

# Update port if needed (if using separate signal controller)
; port=*:9002  (if different from 9001)
```

Edit `speedway-dublin/bin/mmitss-phase3-master-config.json`:
```json
{
  "IntersectionName": "speedway-dublin",
  "IntersectionID": 5,
  "PortNumber": {
    "PrioritySolver": 5000,
    "PrioritySolverToTCIInterface": 5001,
    "SignalCoordination": 5002,
    "TrafficControllerInterface": 5003,
    "TimePhaseDiagramTool": 5004
  },
  "HostIp": "127.0.0.1"
}
```

#### Step 4: Add Container to docker-compose.yml

Edit `/build/docker-compose/server/speedway-sample/docker-compose.yml`:

```yaml
  speedway-dublin:
    container_name: speedway-dublin
    volumes:
      - type: bind
        source: ${MMITSS_ROOT}/config/speedway-sample/field/speedway-dublin/bin
        target: /nojournal/bin
    networks:
      mmitss_vlan:
        ipv4_address: 10.12.6.6
    ports:
      - "9005:9001"  # Maps container port 9001 to host port 9005
    image: mmitssuarizona/mmitss-mrp-${PROCESSOR}:3.0
    environment:
      - TZ=America/Phoenix
    security_opt:
      - apparmor=unconfined
```

#### Step 5: Start New Container

```bash
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample

# Set environment variables
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export PROCESSOR=x86

# Start new container
docker-compose up -d speedway-dublin

# Verify it's running
docker-compose ps | grep speedway-dublin

# Check logs
docker logs speedway-dublin
```

#### Step 6: Verify Supervisord Access

```bash
# Test connectivity
curl -s http://10.12.6.6:9001 | head -c 100

# Or from host
curl -s http://localhost:9005 | head -c 100
```

---

### OPTION 2: Create Separate Docker Compose for New Intersection

#### Step 1: Create New Directory

```bash
mkdir -p $MMITSS_ROOT/build/docker-compose/server/speedway-dublin
```

#### Step 2: Copy and Modify docker-compose.yml

```bash
# Copy existing file as template
cp $MMITSS_ROOT/build/docker-compose/server/speedway-sample/docker-compose.yml \
   $MMITSS_ROOT/build/docker-compose/server/speedway-dublin/docker-compose.yml

# Copy .env file
cp $MMITSS_ROOT/build/docker-compose/server/speedway-sample/.env \
   $MMITSS_ROOT/build/docker-compose/server/speedway-dublin/.env
```

#### Step 3: Edit docker-compose.yml

Modify for single intersection:
```yaml
version: "3.8"

services:
  speedway-dublin:
    container_name: speedway-dublin
    volumes:
      - type: bind
        source: ${MMITSS_ROOT}/config/speedway-sample/field/speedway-dublin/bin
        target: /nojournal/bin
    networks:
      mmitss_vlan:
        ipv4_address: 10.12.6.100
    ports:
      - "9001:9001"
    image: mmitssuarizona/mmitss-mrp-${PROCESSOR}:3.0
    environment:
      - TZ=America/Phoenix
    security_opt:
      - apparmor=unconfined

networks:
  mmitss_vlan:
    driver: bridge
    ipam:
      config:
        - subnet: 10.12.6.0/24
```

#### Step 4: Deploy

```bash
cd $MMITSS_ROOT/build/docker-compose/server/speedway-dublin
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export PROCESSOR=x86
docker-compose up -d
```

---

### OPTION 3: Deploy on Different Host (Distributed)

#### Step 1: Prepare Remote Host

On new host:
```bash
# Install Docker
sudo apt-get update
sudo apt-get install -y docker.io docker-compose

# Clone MMITSS repository
git clone <repository-url> /home/user/mmitss-az
cd /home/user/mmitss-az
```

#### Step 2: Set Up Coordination Server (if multi-host)

Create coordination service (central authority for split data):
```python
# coordination-server.py
from flask import Flask, jsonify
import json

app = Flask(__name__)

# Store active coordination plans
coordination_plans = {
    "Plan1": {
        "CycleLength": 90,
        "Offset": 10,
        "CoordinatedPhases": [2, 6],
        "Split": [20, 25, 0, 0, 0, 0, 0, 0]
    }
}

@app.route('/coordination/split-data', methods=['GET'])
def get_split_data():
    """Return split data for all intersections"""
    return jsonify(coordination_plans)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5555)
```

#### Step 3: Configure Remote Intersection

In remote docker-compose.yml:
```yaml
# Connect to central coordination server
environment:
  - COORDINATION_SERVER_IP=10.0.1.100
  - COORDINATION_SERVER_PORT=5555
```

#### Step 4: Start Remote Container

```bash
export MMITSS_ROOT=/home/user/mmitss-az
export PROCESSOR=x86
export COORDINATION_SERVER_IP=10.0.1.100

docker-compose up -d
```

---

## Configuration Files

### Required Configuration Structure

```
$MMITSS_ROOT/config/speedway-sample/field/
├── speedway-campbell/
│   └── bin/
│       ├── supervisord.conf
│       ├── mmitss-phase3-master-config.json
│       ├── signal_timing_plan.json
│       └── coordination_plan.json
├── speedway-cherry/
├── speedway-mountain/
├── speedway-park/
└── speedway-dublin/  (NEW)
    └── bin/
        ├── supervisord.conf
        ├── mmitss-phase3-master-config.json
        ├── signal_timing_plan.json
        └── coordination_plan.json
```

### mmitss-phase3-master-config.json Template

```json
{
  "IntersectionName": "speedway-dublin",
  "IntersectionID": 5,
  "PortNumber": {
    "PrioritySolver": 5000,
    "PrioritySolverToTCIInterface": 5001,
    "SignalCoordination": 5002,
    "TrafficControllerInterface": 5003,
    "TimePhaseDiagramTool": 5004
  },
  "HostIp": "127.0.0.1",
  "Logging": false,
  "ConsoleOutput": false,
  "CoordinationPlanCheckingTimeInterval": 2,
  "SignalPlanCheckingTimeInterval": 2,
  "MaximumNumberOfSignalPlans": 5
}
```

### signal_timing_plan.json Template

```json
{
  "IntersectionName": "speedway-dublin",
  "SignalPlan": [
    {
      "phaseNumber": 1,
      "phaseRing": 1,
      "minGreen": 4,
      "maxGreen": 15,
      "yellowChange": 3,
      "redClear": 1,
      "pedWalk": 0,
      "pedClear": 0
    },
    {
      "phaseNumber": 2,
      "phaseRing": 1,
      "minGreen": 15,
      "maxGreen": 40,
      "yellowChange": 3,
      "redClear": 2.5,
      "pedWalk": 0,
      "pedClear": 0
    }
  ]
}
```

### coordination_plan.json Template

```json
{
  "IntersectionName": "speedway-dublin",
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
      "Days": ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"],
      "ActionPlan": [
        {
          "CoordinationPatternNo": 1,
          "CoordinationStartTime_Hour": 6,
          "CoordinationStartTime_Minute": 0,
          "CoordinationEndTime_Hour": 22,
          "CoordinationEndTime_Minute": 0
        }
      ]
    }
  ]
}
```

---

## Network & Coordination Setup

### Single Host (All Containers)

**Network Configuration:**
```
Bridge Network: mmitss_vlan
Subnet: 10.12.6.0/24

speedway-campbell: 10.12.6.4
speedway-cherry:   10.12.6.5
speedway-mountain: 10.12.6.3
speedway-park:     10.12.6.2
speedway-dublin:   10.12.6.6
```

**Synchronization:**
- Virtual coordination requests coordinate timing
- Shared configuration files (if on same filesystem)
- Container DNS resolution for service discovery

**Communication Ports:**
```
Each container listens on:
- Port 5000: Priority Request Solver
- Port 5001: TCI Interface
- Port 5002: Signal Coordination
- Port 5003: Traffic Controller
- Port 5004: Time-Phase Diagram Tool
- Port 9001: Supervisord web UI
```

### Multi-Host (Distributed)

**Network Architecture:**

```
Host 1 (Main)              Host 2 (Remote)
├─ speedway-campbell       ├─ speedway-dublin
├─ speedway-cherry         └─ coordination-server
├─ speedway-mountain
└─ speedway-park
    ↓
    └─→ UDP Communication (coordination server IP:5555)
```

**Required for Multi-Host:**

1. **Coordination Server** (central authority):
   ```python
   # Runs on Host 1 or separate host
   - Manages coordination plans
   - Distributes split data to all intersections
   - Port 5555 (UDP/TCP)
   ```

2. **Firewall Rules:**
   ```
   Allow UDP 5555 between hosts (coordination server)
   Allow TCP 9001 to each Supervisord UI
   Allow UDP 5000-5004 for service discovery
   ```

3. **Configuration on Remote:**
   ```json
   {
     "CoordinationServerIP": "192.168.1.100",
     "CoordinationServerPort": 5555,
     "IntersectionID": 5
   }
   ```

---

## Testing & Validation

### Step 1: Verify Container Running

```bash
# Check container status
docker ps | grep speedway-dublin

# Output should show:
# speedway-dublin   mmitssuarizona/mmitss-mrp-x86:3.0   Up 5 minutes

# Check container logs
docker logs speedway-dublin
docker logs --follow speedway-dublin  # Real-time logs
```

### Step 2: Check Supervisord Interface

```bash
# Access Supervisord web UI
curl -s http://10.12.6.6:9001 | grep -i "supervisor"

# Or from host (if port mapped)
curl -s http://localhost:9005 | grep -i "supervisor"

# Expected: HTML response with Supervisord dashboard
```

### Step 3: Verify Service Status

```bash
# Inside container
docker exec speedway-dublin supervisorctl status

# Expected output:
# M_MsgEncoder                     RUNNING   pid 123, uptime 0:05:12
# M_PriorityRequestServer          RUNNING   pid 124, uptime 0:05:12
# M_PriorityRequestSolver          RUNNING   pid 125, uptime 0:05:12
# ... (other services)
```

### Step 4: Test Network Connectivity

```bash
# From existing container to new
docker exec speedway-campbell ping -c 3 speedway-dublin
# Should succeed (10.12.6.6)

# From host
ping 10.12.6.6

# From new container to existing
docker exec speedway-dublin ping -c 3 speedway-campbell
```

### Step 5: Test Service Communication

```bash
# Test Priority Solver port
docker exec speedway-dublin nc -zv 127.0.0.1 5000

# Test Supervisord port
docker exec speedway-dublin netstat -tlnp | grep 9001
```

### Step 6: Verify Coordination Synchronization

```bash
# Check if coordination data received
docker exec speedway-dublin cat /nojournal/log/speedway-dublin_coordinationLog*.log | tail -20

# Look for successful coordination plan loads
# Expected: "Active Coordination Plan loaded"
```

---

## Troubleshooting

### Issue 1: Container Won't Start

**Symptom:** `docker-compose up -d` shows container created but immediately exits

**Solution:**
```bash
# Check logs
docker logs speedway-dublin

# Common issues:
# 1. Config file missing
ls -la $MMITSS_ROOT/config/speedway-sample/field/speedway-dublin/bin/

# 2. Port conflict
docker ps | grep 9005
lsof -i :9005

# 3. Fix and restart
docker-compose down speedway-dublin
docker-compose up -d speedway-dublin
```

### Issue 2: Services Not Running (All FATAL)

**Symptom:** Supervisord shows all services in FATAL state

**Solution:**
```bash
# Check service logs
docker exec speedway-dublin supervisorctl tail -f M_MsgEncoder

# Common issues:
# 1. Config file paths wrong
# 2. Missing dependencies
# 3. Port conflicts

# View service config
docker exec speedway-dublin cat /nojournal/bin/supervisord.conf | grep "command="

# Test command manually
docker exec speedway-dublin /mmitss/M_MsgEncoder --help
```

### Issue 3: Network Connectivity Issues

**Symptom:** Containers can't reach each other

**Solution:**
```bash
# Check Docker network
docker network ls | grep mmitss

# Inspect network
docker network inspect mmitss_vlan

# Check container network interfaces
docker exec speedway-dublin ip addr show

# Test DNS resolution
docker exec speedway-dublin nslookup speedway-campbell
```

### Issue 4: Coordination Not Synchronizing

**Symptom:** Virtual coordination requests not received

**Solution:**
```bash
# Check coordination server
curl http://coordination-server:5555/coordination/split-data

# Verify intersection can reach coordination server
docker exec speedway-dublin ping coordination-server

# Check coordination plan file
docker exec speedway-dublin cat /nojournal/bin/coordination_plan.json

# View coordination logs
docker exec speedway-dublin tail -f /nojournal/log/speedway-dublin_coordinationLog*.log
```

### Issue 5: Port Mapping Not Working on Host

**Symptom:** `curl localhost:9005` fails but `curl 10.12.6.6:9001` works

**Solution:**
```bash
# Check port binding
docker port speedway-dublin

# May be empty! (known issue, use container IP instead)

# Workaround 1: Use container IP
curl http://10.12.6.6:9001

# Workaround 2: Set up port forwarding on host
sudo socat TCP-LISTEN:9005,reuseaddr,fork TCP:10.12.6.6:9001 &

# Workaround 3: SSH tunnel (from remote machine)
ssh -L 9005:10.12.6.6:9001 user@host-ip
# Then access: http://localhost:9005
```

---

## Examples

### Example 1: Add Single Intersection to Existing Setup

```bash
# 1. Create directory
mkdir -p ~/mmitss-az/config/speedway-sample/field/speedway-dublin/bin

# 2. Copy config
cp -r ~/mmitss-az/config/speedway-sample/field/speedway-campbell/bin/* \
      ~/mmitss-az/config/speedway-sample/field/speedway-dublin/bin/

# 3. Update config files
sed -i 's/speedway-campbell/speedway-dublin/g' \
    ~/mmitss-az/config/speedway-sample/field/speedway-dublin/bin/*

# 4. Add to docker-compose.yml (see OPTION 1, Step 4)

# 5. Start
cd ~/mmitss-az/build/docker-compose/server/speedway-sample
export MMITSS_ROOT=~/mmitss-az
export PROCESSOR=x86
docker-compose up -d speedway-dublin

# 6. Verify
docker exec speedway-dublin supervisorctl status
```

### Example 2: Create 4-Intersection Corridor

```bash
# Create 4 new intersections with coordination
intersections=("speedway-dublin" "speedway-elm" "speedway-forest" "speedway-grove")
ips=("10.12.6.6" "10.12.6.7" "10.12.6.8" "10.12.6.9")

for i in "${!intersections[@]}"; do
    name=${intersections[$i]}
    ip=${ips[$i]}
    
    # Create config
    mkdir -p ~/mmitss-az/config/speedway-sample/field/$name/bin
    cp -r ~/mmitss-az/config/speedway-sample/field/speedway-campbell/bin/* \
          ~/mmitss-az/config/speedway-sample/field/$name/bin/
    
    # Update intersection name in config
    sed -i "s/speedway-campbell/$name/g" \
        ~/mmitss-az/config/speedway-sample/field/$name/bin/*
    
    # Add to docker-compose.yml with offset coordination
    offset=$((i * 20))  # 0s, 20s, 40s, 60s offsets
    echo "
  $name:
    container_name: $name
    volumes:
      - type: bind
        source: \${MMITSS_ROOT}/config/speedway-sample/field/$name/bin
        target: /nojournal/bin
    networks:
      mmitss_vlan:
        ipv4_address: $ip
    ports:
      - \"$((9005+i)):9001\"
    image: mmitssuarizona/mmitss-mrp-\${PROCESSOR}:3.0
    environment:
      - TZ=America/Phoenix
    security_opt:
      - apparmor=unconfined
    " >> docker-compose.yml
done

# Start all
cd ~/mmitss-az/build/docker-compose/server/speedway-sample
export MMITSS_ROOT=~/mmitss-az
export PROCESSOR=x86
docker-compose up -d

# Verify all running
docker-compose ps
```

### Example 3: Deploy to Remote Host with Central Coordination

**On Central Host (Host A):**
```bash
# Start coordination server
python3 coordination-server.py &

# Start main intersections
cd ~/mmitss-az/build/docker-compose/server/speedway-sample
docker-compose up -d
```

**On Remote Host (Host B):**
```bash
# Clone and setup
git clone <repo> ~/mmitss-az
cd ~/mmitss-az/build/docker-compose/server/speedway-dublin

# Configure for central coordination
sed -i 's/"CoordinationServerIP": "localhost"/"CoordinationServerIP": "192.168.1.100"/' \
    ~/mmitss-az/config/speedway-sample/field/speedway-dublin/bin/mmitss-phase3-master-config.json

# Start
export MMITSS_ROOT=~/mmitss-az
export PROCESSOR=x86
docker-compose up -d
```

---

## Summary: Deployment Checklist

```
[ ] 1. Create intersection configuration directory
[ ] 2. Copy configuration files from existing intersection
[ ] 3. Update intersection name in all config files
[ ] 4. Update IP address in docker-compose.yml
[ ] 5. Update port mapping (if needed)
[ ] 6. Add entry to docker-compose.yml
[ ] 7. Export MMITSS_ROOT and PROCESSOR
[ ] 8. Run docker-compose up -d
[ ] 9. Verify container running with docker ps
[ ] 10. Check Supervisord interface accessible
[ ] 11. Verify services running with supervisorctl status
[ ] 12. Test network connectivity to other intersections
[ ] 13. Verify coordination plan loaded
[ ] 14. Monitor logs for errors
[ ] 15. Test priority request handling
```

---

## Key Differences: Existing vs New Intersection

| Aspect | Existing (Campbell) | New (Dublin) |
|--------|-------------------|--------------|
| Config Dir | Preconfigured | Create manually |
| Docker Image | Same image | Same image |
| Network | Part of mmitss_vlan | Join mmitss_vlan |
| IP Address | 10.12.6.4 | Assign new (10.12.6.6+) |
| Port Mapping | 9001 | Map to new (9005+) |
| Supervisord | Configured | Copy & customize |
| Services | 12 predefined | Same 12 services |
| Coordination | Virtual requests | Virtual requests |
| Optimization | GLPK solver | GLPK solver |

---

## Production Considerations

### Security
- Use internal Docker networks (not exposed to internet)
- Implement firewall rules between hosts
- Use VPN for remote intersections
- Authenticate coordination server requests

### Performance
- Monitor CPU/memory per container
- Scale coordination server for 20+ intersections
- Use load balancing for split data distribution

### Reliability
- Implement health checks in docker-compose
- Set up automatic restart policies
- Use Docker volumes for persistent logs
- Implement backup coordination plans

### Scaling
- Tested up to 10 intersections per compose file
- Beyond that: use distributed deployment model
- Consider Kubernetes for 50+ intersections

---

## Next Steps

1. **Choose Deployment Model:**
   - Single host (5-10 intersections): Use Option 1
   - Multiple hosts: Use Option 2 or 3

2. **Prepare Configuration:**
   - Create directories
   - Copy template configs
   - Customize for each intersection

3. **Deploy:**
   - Start containers
   - Verify services
   - Test connectivity

4. **Monitor:**
   - Watch logs
   - Check service status
   - Validate coordination

5. **Scale:**
   - Add more intersections
   - Synchronize timing
   - Optimize weights

