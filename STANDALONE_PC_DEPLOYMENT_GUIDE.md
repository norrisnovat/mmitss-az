# 🖥️ MMITSS Standalone PC Deployment Guide - Simulation Setup

## Overview

This guide walks through deploying the MMITSS system on a standalone PC (not connected to real traffic hardware) for simulation, testing, and development purposes.

---

## Table of Contents

1. [System Requirements](#system-requirements)
2. [Pre-Deployment Checklist](#pre-deployment-checklist)
3. [Step-by-Step Deployment](#step-by-step-deployment)
4. [Docker Setup on Standalone PC](#docker-setup-on-standalone-pc)
5. [Environment Configuration](#environment-configuration)
6. [Starting the System](#starting-the-system)
7. [Accessing Interfaces](#accessing-accessing-interfaces)
8. [Simulation Features](#simulation-features)
9. [Troubleshooting](#troubleshooting)
10. [Quick Reference](#quick-reference)

---

## System Requirements

### Hardware Minimum
```
CPU:        Dual-core 2.0+ GHz (4 cores recommended)
RAM:        8 GB (16 GB recommended for smooth operation)
Storage:    50 GB free space
Network:    Ethernet or WiFi (for localhost access)
OS:         Linux, macOS, or Windows with Docker
```

### Recommended Setup
```
CPU:        Quad-core or higher (i5/Ryzen 5+)
RAM:        16 GB or more
Storage:    100 GB SSD (faster than HDD)
OS:         Linux (Ubuntu 20.04+ or similar)
Docker:     Latest Docker + Docker Compose
```

### Why Simulation on Standalone is Great
- ✅ No external hardware needed
- ✅ Complete control over test scenarios
- ✅ Development and testing flexibility
- ✅ Lower cost than real-world deployment
- ✅ Safe experimentation environment
- ✅ Repeatable scenarios

---

## Pre-Deployment Checklist

### On Your Standalone PC

```
[ ] 1. Docker installed and running
[ ] 2. Docker Compose installed
[ ] 3. At least 50 GB free disk space
[ ] 4. Git installed (for cloning)
[ ] 5. Terminal/bash available
[ ] 6. Text editor (nano, vim, or VS Code)
[ ] 7. 8+ GB RAM available
```

### On Original Machine (VM)

```
[ ] 1. Clone MMITSS repository
[ ] 2. Verify all configuration files present
[ ] 3. Test system running correctly
[ ] 4. Create tar/zip backup of workspace
[ ] 5. Transfer to new PC via USB/Network/Cloud
```

---

## Step-by-Step Deployment

### PHASE 1: Transfer Repository to Standalone PC

#### Option A: Git Clone (Recommended)

```bash
# 1. On new PC, open terminal
cd ~/Documents    # or desired location

# 2. Clone the repository
git clone https://github.com/norrisnovat/mmitss-az.git

# Verify clone successful
cd mmitss-az
ls -la
# Should show: README.md, DOCKER_*, build/, src/, config/, docs/

# 3. Check out correct branch (if needed)
git branch -a
git checkout master
```

#### Option B: Manual Transfer (USB/Network Drive)

```bash
# On original VM:
# 1. Create compressed backup
cd ~/Documents
tar -czf mmitss-az-backup.tar.gz mmitss-az/

# 2. Move to USB drive or network share
# (Depends on your system - use file manager or scp)

# On new PC:
# 1. Extract backup
tar -xzf mmitss-az-backup.tar.gz -C ~/Documents/

# 2. Verify extraction
cd ~/Documents/mmitss-az
ls -la
```

#### Option C: Cloud Transfer (Google Drive, Dropbox, GitHub)

```bash
# Create private GitHub repository
# Or use: 
#   - Google Drive (gsutil)
#   - Dropbox (dropbox CLI)
#   - AWS S3 (aws s3)

# Download to new PC
# Extract and verify as above
```

### PHASE 2: Install Docker on Standalone PC

#### Linux (Ubuntu/Debian)

```bash
# 1. Update package manager
sudo apt-get update
sudo apt-get upgrade -y

# 2. Install Docker
sudo apt-get install -y docker.io docker-compose

# 3. Add current user to docker group
sudo usermod -aG docker $USER

# 4. Apply group changes (or log out/in)
newgrp docker

# 5. Verify installation
docker --version
docker-compose --version

# 6. Test Docker
docker run hello-world
```

#### macOS (Intel/Apple Silicon)

```bash
# 1. Install Docker Desktop from:
# https://www.docker.com/products/docker-desktop

# 2. Or use Homebrew
brew install docker docker-compose

# 3. Start Docker
open /Applications/Docker.app

# 4. Wait for Docker to start (check menu bar)

# 5. Verify
docker --version
docker-compose --version
```

#### Windows 10/11

```
# 1. Install Docker Desktop:
# https://www.docker.com/products/docker-desktop

# 2. Requirements:
#    - WSL 2 (Windows Subsystem for Linux)
#    - Virtualization enabled in BIOS
#    - 4 GB RAM allocated

# 3. Run installer, follow prompts

# 4. Restart computer

# 5. Open PowerShell or CMD
docker --version
docker-compose --version
```

### PHASE 3: Set Up Environment Variables

#### Step 1: Create Environment File

```bash
cd ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample

# Create .env file
cat > .env << 'ENVEOF'
# MMITSS Environment Configuration
MMITSS_ROOT=/home/username/Documents/mmitss-az
PROCESSOR=x86
MMITSS_NETWORK_ADAPTER=eth0
COMPOSE_PROJECT_NAME=mmitss-simulation
ENVEOF
```

#### Step 2: Update Paths in .env

```bash
# Edit the .env file
nano .env

# Update MMITSS_ROOT to match your system:
# Windows:      MMITSS_ROOT=C:\Users\YourName\Documents\mmitss-az
# macOS:        MMITSS_ROOT=/Users/YourName/Documents/mmitss-az
# Linux:        MMITSS_ROOT=/home/YourName/Documents/mmitss-az

# Update MMITSS_NETWORK_ADAPTER if needed:
# Windows:      MMITSS_NETWORK_ADAPTER=Ethernet
# macOS:        MMITSS_NETWORK_ADAPTER=en0
# Linux:        MMITSS_NETWORK_ADAPTER=eth0 or enp0s3

# Save and exit (Ctrl+X, then Y, then Enter for nano)
```

#### Step 3: Verify .env

```bash
cat .env
# Should show your configured paths and settings
```

### PHASE 4: Verify Configuration Files

```bash
# Check required config directories
ls -la ~/Documents/mmitss-az/config/speedway-sample/field/

# Should contain:
# ├── speedway-campbell/bin/
# ├── speedway-cherry/bin/
# ├── speedway-mountain/bin/
# └── speedway-park/bin/

# Verify each has required files
for dir in speedway-campbell speedway-cherry speedway-mountain speedway-park; do
  echo "=== $dir ==="
  ls -la ~/Documents/mmitss-az/config/speedway-sample/field/$dir/bin/ | grep -E "supervisord|mmitss-phase3|json"
done
```

### PHASE 5: Pull Docker Image

```bash
# Pull the MMITSS docker image
docker pull mmitssuarizona/mmitss-mrp-x86:3.0

# This may take 5-15 minutes (image is ~860 MB)

# Verify image downloaded
docker images | grep mmitss

# Should show:
# mmitssuarizona/mmitss-mrp-x86   3.0   <sha>   <date>   860 MB
```

### PHASE 6: Start Containers

```bash
cd ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample

# Set environment variables
export MMITSS_ROOT=~/Documents/mmitss-az
export PROCESSOR=x86

# Or on Windows (PowerShell):
# $env:MMITSS_ROOT = "C:\Users\YourName\Documents\mmitss-az"
# $env:PROCESSOR = "x86"

# Start all containers
docker-compose up -d

# Watch startup process
docker-compose logs -f
# Press Ctrl+C to stop watching logs
```

### PHASE 7: Verify All Containers Running

```bash
# Check container status
docker ps

# Expected output:
# speedway-campbell    Up 2 minutes
# speedway-cherry      Up 2 minutes
# speedway-mountain    Up 2 minutes
# speedway-park        Up 2 minutes

# Check individual container logs
docker logs speedway-campbell
docker logs speedway-cherry
docker logs speedway-mountain
docker logs speedway-park

# Watch real-time logs
docker logs -f speedway-campbell
```

---

## Docker Setup on Standalone PC

### Network Architecture for Simulation

```
┌──────────────────────────────────────────┐
│        Standalone PC                      │
│                                           │
│  ┌────────────────────────────────────┐   │
│  │   Docker Bridge Network             │   │
│  │   (mmitss_vlan)                     │   │
│  │                                    │   │
│  │  Intersection 1    Intersection 2  │   │
│  │  speedway-campbell speedway-cherry │   │
│  │  10.12.6.4:9001   10.12.6.5:9001  │   │
│  │                                    │   │
│  │  Intersection 3    Intersection 4  │   │
│  │  speedway-mountain speedway-park   │   │
│  │  10.12.6.3:9001   10.12.6.2:9001  │   │
│  │                                    │   │
│  └────────────────────────────────────┘   │
│                    ↓                       │
│            Localhost Access                │
│    • http://localhost:9001 (Campbell)     │
│    • http://localhost:9002 (Cherry)       │
│    • http://localhost:9003 (Mountain)     │
│    • http://localhost:9004 (Park)         │
└──────────────────────────────────────────┘
```

### Docker Compose Details

**File:** `build/docker-compose/server/speedway-sample/docker-compose.yml`

Key sections for standalone:
```yaml
services:
  speedway-campbell:
    container_name: speedway-campbell
    image: mmitssuarizona/mmitss-mrp-${PROCESSOR}:3.0
    ports:
      - "9001:9001"      # Port mapping (localhost:9001 → container:9001)
    networks:
      mmitss_vlan:
        ipv4_address: 10.12.6.4
    volumes:
      - type: bind
        source: ${MMITSS_ROOT}/config/speedway-sample/field/speedway-campbell/bin
        target: /nojournal/bin

  # (Similar for cherry, mountain, park with ports 9002, 9003, 9004)

networks:
  mmitss_vlan:
    driver: bridge
    ipam:
      config:
        - subnet: 10.12.6.0/24
```

### Port Mapping for Localhost Access

On standalone PC, port mappings work automatically:

```
localhost:9001 → speedway-campbell Supervisord
localhost:9002 → speedway-cherry Supervisord
localhost:9003 → speedway-mountain Supervisord
localhost:9004 → speedway-park Supervisord
```

---

## Environment Configuration

### System Paths to Update

Create configuration for your specific machine:

```bash
# 1. Determine your paths
echo "Home directory: $HOME"
echo "MMITSS root should be: $HOME/Documents/mmitss-az"

# 2. Update .env file
nano ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample/.env

# 3. Update this line:
MMITSS_ROOT=/home/norris/Documents/mmitss-az
# To:
MMITSS_ROOT=$HOME/Documents/mmitss-az
# Or full path: /home/yourusername/Documents/mmitss-az
```

### Configuration Files

Verify these exist:

```bash
# Required config structure
$MMITSS_ROOT/config/speedway-sample/field/
├── speedway-campbell/bin/
│   ├── supervisord.conf
│   ├── mmitss-phase3-master-config.json
│   ├── signal_timing_plan.json
│   └── coordination_plan.json
├── speedway-cherry/bin/
│   └── (same files)
├── speedway-mountain/bin/
│   └── (same files)
└── speedway-park/bin/
    └── (same files)
```

Check they're present:
```bash
find ~/Documents/mmitss-az/config -name "supervisord.conf" | wc -l
# Should output: 4 (one per intersection)
```

---

## Starting the System

### Full Startup Sequence

```bash
# 1. Navigate to docker-compose directory
cd ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample

# 2. Set environment variables
export MMITSS_ROOT=~/Documents/mmitss-az
export PROCESSOR=x86

# 3. Start containers (detached mode)
docker-compose up -d

# Output should show:
# Creating network "speedway-sample_mmitss_vlan" with driver "bridge"
# Creating speedway-campbell ... done
# Creating speedway-cherry ... done
# Creating speedway-mountain ... done
# Creating speedway-park ... done

# 4. Wait 10-15 seconds for services to start
sleep 15

# 5. Verify all containers running
docker-compose ps

# Expected output:
# STATUS: Up X seconds (all 4 should be running)

# 6. Check service status
docker exec speedway-campbell supervisorctl status

# Expected output:
# M_MsgEncoder                    RUNNING
# M_PriorityRequestServer         RUNNING
# M_PriorityRequestSolver         RUNNING
# (+ 9 other services)
```

### Startup Logs

```bash
# View startup progress
docker-compose logs --tail=50 -f

# View specific container
docker logs -f speedway-campbell

# View with timestamps
docker logs -f --timestamps speedway-campbell
```

### Troubleshooting Startup

```bash
# If containers don't start, check:

# 1. Docker daemon running
docker ps
# If error: Docker daemon not running

# 2. Image available
docker images | grep mmitss

# 3. Ports not in use
netstat -tlnp | grep 900
# Should show no conflicts on ports 9001-9004

# 4. Disk space
df -h
# Ensure >10 GB free

# 5. Memory available
free -h
# Should have at least 2 GB free

# 6. Check container error
docker logs speedway-campbell
# Look for specific error messages
```

---

## Accessing Interfaces

### Web Interface Access

Open your browser to any of these URLs:

```
Supervisord Dashboards (Web UI for process management):

http://localhost:9001     (speedway-campbell)
http://localhost:9002     (speedway-cherry)
http://localhost:9003     (speedway-mountain)
http://localhost:9004     (speedway-park)

Or use container IP (also works):

http://10.12.6.4:9001     (speedway-campbell)
http://10.12.6.5:9001     (speedway-cherry)
http://10.12.6.3:9001     (speedway-mountain)
http://10.12.6.2:9001     (speedway-park)
```

### What You'll See

Each dashboard shows:

```
┌─────────────────────────────────────────┐
│  Supervisord Web Interface               │
├─────────────────────────────────────────┤
│                                         │
│  Server Status:  RUNNING                │
│  Available Log:  Main log file           │
│                                         │
│  Process Control:                       │
│  ✓ M_MsgEncoder              RUNNING    │
│  ✓ M_PriorityRequestServer   RUNNING    │
│  ✓ M_PriorityRequestSolver   RUNNING    │
│  ✓ M_SystemInterface         RUNNING    │
│  ✓ M_TrajectoryAware         RUNNING    │
│  ✓ M_WirelessMsgDecoder      RUNNING    │
│  ✓ M_TimePhaseDiagramTool    RUNNING    │
│  ✗ M_MapSpatBroadcaster      FATAL      │
│  ✗ M_SignalCoordinationRequestGenerator FATAL
│  ✗ M_TrafficControllerInterface FATAL   │
│  ✗ M_V2XDataCollector        FATAL      │
│  ○ M_SnmpEngine              STOPPED    │
│                                         │
└─────────────────────────────────────────┘
```

### Remote Access (Optional)

To access from another computer on your network:

```bash
# 1. Find your PC's IP address
# Linux/Mac:
ifconfig | grep "inet " | grep -v 127.0.0.1

# Windows:
ipconfig

# 2. From remote computer, open browser:
http://<your-pc-ip>:9001
# Example: http://192.168.1.100:9001
```

---

## Simulation Features

### What Works in Standalone Mode

✅ **Core Coordination Algorithm**
- Priority request handling
- Signal optimization
- Virtual coordination requests
- Green wave timing
- Multi-intersection coordination

✅ **Testing & Development**
- Algorithm modifications
- Configuration changes
- Performance testing
- Log analysis
- Parameter tuning

✅ **Monitoring**
- Service status
- Process logs
- Performance metrics
- Resource usage (CPU, memory)

✅ **Data Collection**
- Optimization results
- Timing decisions
- Coordination data
- Performance statistics

### What Doesn't Work (Expected)

❌ **External Hardware Connections**
- Real traffic signal controllers
- RSU (Roadside Unit) hardware
- V2X network communication
- GPS-enabled vehicles

❌ **Services (Shows FATAL)**
- M_MapSpatBroadcaster - Needs RSU hardware
- M_SignalCoordinationRequestGenerator - Needs coordination server
- M_TrafficControllerInterface - Needs physical signal controller
- M_V2XDataCollector - Needs V2X network

**This is NORMAL and EXPECTED for simulation mode!**

### Simulating Vehicle Requests

To test the system, you can create mock vehicle requests:

```python
# Example: priority-request-simulator.py

import json
import socket
import time
from datetime import datetime, timedelta

def send_priority_request(intersection_ip, port, priority_type="TRANSIT"):
    """Send simulated priority request to MMITSS"""
    
    request = {
        "vehicleID": f"SIM_{int(time.time())}",
        "latitude": 33.4236,
        "longitude": -111.9286,
        "heading": 180,
        "speed": 25,
        "eta": (datetime.now() + timedelta(seconds=10)).isoformat(),
        "priority": priority_type,
        "equipmentType": 4  # 1=Emergency, 2=Transit, 3=Truck, 4=None
    }
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.sendto(json.dumps(request).encode(), (intersection_ip, port))
        print(f"Sent {priority_type} request to {intersection_ip}:{port}")
    except Exception as e:
        print(f"Error: {e}")

# Usage:
# send_priority_request("localhost", 5000, "TRANSIT")
# send_priority_request("localhost", 5000, "EMERGENCY")
```

---

## Troubleshooting

### Issue 1: Docker Won't Start

**Symptom:** `docker ps` shows error

**Solution:**
```bash
# Check if Docker daemon is running
sudo systemctl status docker

# Start Docker
sudo systemctl start docker

# Or on Mac, open Docker app from Applications
```

### Issue 2: Port Already in Use

**Symptom:** Error creating container on port 9001

**Solution:**
```bash
# Check what's using port 9001
netstat -tlnp | grep 9001
lsof -i :9001

# Kill process using that port
kill -9 <PID>

# Or change port in docker-compose.yml
# Change "9001:9001" to "9005:9001" (etc)
```

### Issue 3: Container Crashes on Startup

**Symptom:** `docker ps` shows `Exited (1)`

**Solution:**
```bash
# Check logs for error
docker logs speedway-campbell

# Common issues:
# 1. Config file missing
ls ~/Documents/mmitss-az/config/speedway-sample/field/speedway-campbell/bin/

# 2. Wrong path in .env
cat ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample/.env

# 3. Insufficient disk space
df -h

# 4. Restart containers
docker-compose restart
```

### Issue 4: Can't Access Web Interface

**Symptom:** Browser shows "Cannot connect to localhost:9001"

**Solution:**
```bash
# Check container is running
docker ps | grep speedway-campbell

# Check port is mapped
docker port speedway-campbell

# Test connectivity
curl -s http://localhost:9001 | head -20

# If fails, try container IP
curl -s http://10.12.6.4:9001 | head -20

# Check firewall
sudo ufw status
# If enabled, allow port
sudo ufw allow 9001/tcp
```

### Issue 5: Services Show FATAL

**Symptom:** Multiple services in FATAL state

**Solution:**
```bash
# This is EXPECTED for simulation mode!
# Services that need real hardware will be FATAL:
# - M_TrafficControllerInterface (needs signal controller)
# - M_MapSpatBroadcaster (needs RSU hardware)
# - M_SignalCoordinationRequestGenerator (needs coordination server)
# - M_V2XDataCollector (needs V2X network)

# To verify this is normal:
docker exec speedway-campbell supervisorctl tail M_TrafficControllerInterface

# Should show error like:
# "[Errno 99] Cannot assign requested address"
# This is normal - service can't reach hardware that doesn't exist

# Core services should still be RUNNING:
docker exec speedway-campbell supervisorctl status | grep RUNNING
```

### Issue 6: Performance Issues (Slow, Laggy)

**Symptom:** System runs but feels slow

**Solution:**
```bash
# Check resource usage
docker stats

# Monitor CPU/Memory
# Press Ctrl+C to stop monitoring

# If using too much:
# 1. Close unnecessary applications
# 2. Increase Docker memory allocation
# 3. Use faster storage (SSD vs HDD)

# Check disk I/O
iostat -xm 1 5

# Reduce logging (improves performance)
# Edit supervisord.conf: set silent=true
```

---

## Quick Reference

### Essential Commands

```bash
# Navigate to project
cd ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample

# Set environment variables
export MMITSS_ROOT=~/Documents/mmitss-az
export PROCESSOR=x86

# Start all containers
docker-compose up -d

# Check status
docker-compose ps

# View logs (all)
docker-compose logs

# View logs (real-time)
docker-compose logs -f

# View specific container logs
docker logs -f speedway-campbell

# Stop all containers
docker-compose stop

# Remove all containers
docker-compose down

# Remove with volumes
docker-compose down -v

# Restart a service
docker-compose restart speedway-campbell

# Check service status inside container
docker exec speedway-campbell supervisorctl status

# View service logs inside container
docker exec speedway-campbell tail -f /nojournal/log/supervisord.log
```

### Browser Access

```
http://localhost:9001    speedway-campbell
http://localhost:9002    speedway-cherry
http://localhost:9003    speedway-mountain
http://localhost:9004    speedway-park
```

### File Locations

```
Project Root:        ~/Documents/mmitss-az/
Config:              ~/Documents/mmitss-az/config/speedway-sample/field/
Docker Compose:      ~/Documents/mmitss-az/build/docker-compose/server/speedway-sample/
Source Code:         ~/Documents/mmitss-az/src/
Documentation:       ~/Documents/mmitss-az/*.md
```

### Environment Variables

```bash
export MMITSS_ROOT=/path/to/mmitss-az
export PROCESSOR=x86
export MMITSS_NETWORK_ADAPTER=eth0
```

---

## Performance Tuning for Simulation

### For Better Performance

```yaml
# Edit docker-compose.yml

services:
  speedway-campbell:
    cpus: "1.0"        # Limit to 1 CPU core
    mem_limit: "2g"    # Limit to 2 GB RAM
    memswap_limit: "2g"
```

### For Development (No Limit)

```yaml
# Don't set limits - allows full system resources
services:
  speedway-campbell:
    # cpus and mem_limit removed
```

### Monitor Performance

```bash
# Real-time monitoring
docker stats

# Per-process
docker exec speedway-campbell ps aux

# Memory usage
docker exec speedway-campbell free -h

# Disk usage
docker exec speedway-campbell df -h /nojournal
```

---

## Summary: From Clone to Running

```
1. Clone repo:
   git clone <repo-url>

2. Install Docker:
   (Varies by OS - see instructions above)

3. Create .env file:
   MMITSS_ROOT=/path/to/mmitss-az
   PROCESSOR=x86

4. Pull image:
   docker pull mmitssuarizona/mmitss-mrp-x86:3.0

5. Start system:
   cd build/docker-compose/server/speedway-sample
   export MMITSS_ROOT=...
   docker-compose up -d

6. Verify running:
   docker ps
   http://localhost:9001

✅ System ready for simulation!
```

---

## Next Steps

1. **Access Supervisord Dashboards**
   - Open http://localhost:9001 in browser
   - Repeat for 9002, 9003, 9004

2. **Monitor Service Status**
   - View all 4 intersections
   - Note: 4 services will be FATAL (normal for simulation)
   - 6-7 services should be RUNNING

3. **Review Documentation**
   - Read MMITSS_COORDINATION_ALGORITHM_GUIDE.md
   - Understand how algorithm works

4. **Test Simulation**
   - Create mock priority requests
   - Observe signal timing changes
   - Monitor optimization cycles

5. **Customize Configuration**
   - Modify timing plans
   - Adjust priority weights
   - Test different scenarios

6. **Scale to Multiple Intersections**
   - Add more intersections (refer to MMITSS_DEPLOYMENT_NEW_INTERSECTIONS.md)
   - Create corridor coordination
   - Test green wave timing

