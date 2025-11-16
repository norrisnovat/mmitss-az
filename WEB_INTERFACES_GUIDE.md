# MMITSS Web Interfaces Launch Guide

## Overview

MMITSS provides multiple web interfaces for monitoring and control:

1. **Supervisord Web Interface** - Process management and monitoring
2. **System Interface Flask App** - Configuration and process control

---

## Quick Access

### Supervisord Web Interfaces

After running `docker-compose up -d`, access the Supervisord web interfaces at:

| Intersection | URL | Port |
|---|---|---|
| **Speedway Campbell** | `http://localhost:9001` | 9001 |
| **Speedway Cherry** | `http://localhost:9002` | 9002 |
| **Speedway Mountain** | `http://localhost:9003` | 9003 |
| **Speedway Park** | `http://localhost:9004` | 9004 |

**Example**:
```bash
# In your browser, open:
http://localhost:9001
```

### System Interface Flask App

The System Interface application runs **inside** the containers and provides:
- Process control console
- Configuration viewer/editor
- Performance monitoring

**Status**: Check if running by looking at the Supervisord interface.

---

## Prerequisites

### 1. Set Environment Variables

```bash
# Add to ~/.bashrc or ~/.zshrc
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export MMITSS_NETWORK_ADAPTER=enp0s3
export PROCESSOR=amd64
```

### 2. Navigate to Docker Compose Directory

```bash
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
```

### 3. Start Services

```bash
docker-compose up -d
```

### 4. Verify Containers Running

```bash
docker-compose ps
```

Expected output:
```
CONTAINER ID   IMAGE                                    STATUS
abc123...      mmitssuarizona/mmitss-mrp-amd64:3.0    Up 2 minutes  speedway-campbell
def456...      mmitssuarizona/mmitss-mrp-amd64:3.0    Up 2 minutes  speedway-cherry
ghi789...      mmitssuarizona/mmitss-mrp-amd64:3.0    Up 2 minutes  speedway-mountain
jkl012...      mmitssuarizona/mmitss-mrp-amd64:3.0    Up 2 minutes  speedway-park
```

---

## Supervisord Web Interface

### What It Does

Supervisord is a process control system that manages MMITSS services. The web interface allows you to:

- ✅ View process status (running/stopped)
- ✅ Start/stop individual processes
- ✅ View process logs
- ✅ Monitor memory and CPU usage
- ✅ Check service health

### How to Access

#### Method 1: Direct Browser Access

Simply open these URLs in your browser:

- Campbell: `http://localhost:9001`
- Cherry: `http://localhost:9002`
- Mountain: `http://localhost:9003`
- Park: `http://localhost:9004`

#### Method 2: Using curl

```bash
# Check if Supervisord is responding
curl -v http://localhost:9001

# Should return HTML with supervisor dashboard
```

#### Method 3: Using supervisorctl (Command Line)

Inside the container:

```bash
# Access Campbell container
docker exec -it speedway-campbell supervisorctl

# Then in supervisorctl shell:
status                    # Show status of all processes
start MsgEncoder          # Start a specific process
stop all                  # Stop all processes
tail -f ProcessName       # View process logs
```

### Understanding the Dashboard

The Supervisord web interface shows:

1. **Process Status Table**:
   - Process Name
   - Status (RUNNING, STOPPED, ERROR, FATAL)
   - Uptime
   - CPU %
   - Memory (MB)

2. **Action Buttons**:
   - `Start` - Start a stopped process
   - `Stop` - Stop a running process
   - `Restart` - Restart a process
   - `Clear Log` - Clear process log

3. **Log Viewer**:
   - Click process name to view logs
   - Shows stdout/stderr output
   - Useful for debugging errors

### Expected Processes

In a healthy MMITSS deployment, you should see 230+ processes in RUNNING state:

```
✅ RUNNING Services Examples:
- MsgEncoder
- MessageDistributor
- PriorityRequestServer
- TrafficControllerInterface
- SnmpEngine
- MapSpatBroadcaster
- V2XDataTransfer
- (and many more...)
```

### Common Issues & Fixes

#### Issue: "Cannot connect to localhost:9001"

**Causes**:
- Containers not running
- Port not exposed
- Firewall blocking

**Fix**:
```bash
# 1. Check containers
docker-compose ps

# 2. Check if port is listening
netstat -tlnp | grep 9001
# or
lsof -i :9001

# 3. Verify docker-compose has port mapping
docker-compose config | grep ports -A 2

# 4. Restart containers
docker-compose restart
```

#### Issue: "Port already in use"

**Fix**:
```bash
# Find what's using the port
lsof -i :9001

# Kill the process or change port mapping in docker-compose.yml
# Then restart
docker-compose down
docker-compose up -d
```

#### Issue: "Page loads but shows no processes"

**Cause**: Supervisord is running but processes failed to start

**Fix**:
```bash
# Check Supervisord logs
docker logs speedway-campbell

# Check process logs in UI or via:
docker exec speedway-campbell supervisorctl tail MsgEncoder

# Verify config files exist
docker exec speedway-campbell ls -la /nojournal/bin/*.json
```

---

## System Interface Flask App

### What It Does

The System Interface is a Flask web application for:

- 🔧 **Process Control Console** - Remote management of services
- ⚙️ **Configuration Viewer/Editor** - View and edit MMITSS configuration
- 📊 **Performance Monitoring** - View system metrics

### How to Access

#### Inside Container

The Flask app runs **inside** each container. Access it via:

1. Get the container IP:
```bash
docker inspect speedway-campbell | grep IPAddress
# Output: "172.17.0.2"
```

2. Access the app (replace with actual IP):
```bash
# In browser or curl:
http://172.17.0.2:5000

# Or via container name (if DNS resolution works):
http://speedway-campbell:5000
```

#### From Host Machine

To access from your host, you need to expose the Flask port. Add to `docker-compose.yml`:

```yaml
services:
    speedway-campbell:
        ...
        ports:
            - "9001:9001"    # Supervisord
            - "5001:5000"    # Flask System Interface
```

Then access:
```bash
http://localhost:5001
```

### Features

#### Homepage (`/`)
- Overview of system status
- Link to configuration

#### Process Control Console (`/console`)
- Remote process control
- View running services
- Start/stop/restart services
- Monitor logs

#### Local Console (`/local_console`)
- Full supervisord integration
- Embedded iframe to supervisord web interface
- Local process management

#### Configuration Editor (`/config`)
- View current configuration
- Edit settings
- Save changes back to JSON

### Configuration File Location

The Flask app reads/writes:
```
/nojournal/bin/mmitss-phase3-master-config.json
```

This file contains all MMITSS system configuration parameters.

---

## Recommended Setup Workflow

### Step 1: Start Services

```bash
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
docker-compose up -d
```

### Step 2: Verify Services Running

```bash
docker-compose ps
```

### Step 3: Access Supervisord Interfaces

Open in separate browser tabs:
- http://localhost:9001 (Campbell)
- http://localhost:9002 (Cherry)
- http://localhost:9003 (Mountain)
- http://localhost:9004 (Park)

### Step 4: Verify Process Health

In each Supervisord interface:
1. Look for all processes in RUNNING state
2. Should see 200+ services running
3. If any are in ERROR/FATAL, click to view logs

### Step 5: Monitor Logs (Optional)

```bash
# View real-time logs from all containers
docker-compose logs -f

# View specific container
docker logs -f speedway-campbell

# Search for errors
docker-compose logs | grep -i error
```

---

## Advanced: Multiple Interfaces at Once

### Using a Script to Open All

Create `open-interfaces.sh`:

```bash
#!/bin/bash

# Open all Supervisord interfaces in browser
if command -v xdg-open &> /dev/null; then
    # Linux
    xdg-open http://localhost:9001 &
    xdg-open http://localhost:9002 &
    xdg-open http://localhost:9003 &
    xdg-open http://localhost:9004 &
elif command -v open &> /dev/null; then
    # macOS
    open http://localhost:9001
    open http://localhost:9002
    open http://localhost:9003
    open http://localhost:9004
else
    echo "Please open these URLs manually:"
    echo "http://localhost:9001"
    echo "http://localhost:9002"
    echo "http://localhost:9003"
    echo "http://localhost:9004"
fi
```

Make executable:
```bash
chmod +x open-interfaces.sh
```

Use:
```bash
./open-interfaces.sh
```

### Using Tab Manager

For quick access, consider using browser tools:
- **Firefox**: Bookmark all 4 URLs, open in new window
- **Chrome**: Create a group with 4 tabs
- **Command**: `for i in {1..4}; do xdg-open http://localhost:$((9000+i)); done`

---

## Port Mapping Reference

### Current Configuration

```yaml
# Port Mappings in docker-compose.yml
Speedway Campbell:  Host 9001 → Container 9001 (Supervisord)
Speedway Cherry:    Host 9002 → Container 9001 (Supervisord)
Speedway Mountain:  Host 9003 → Container 9001 (Supervisord)
Speedway Park:      Host 9004 → Container 9001 (Supervisord)
```

### Custom Ports

To use different host ports, edit docker-compose.yml:

```yaml
speedway-campbell:
    ports:
        - "8001:9001"  # Use 8001 instead of 9001
```

---

## Troubleshooting Checklist

- [ ] Docker is running: `docker ps`
- [ ] Containers are running: `docker-compose ps`
- [ ] Port not in use: `lsof -i :9001`
- [ ] Firewall allows connections: Check firewall settings
- [ ] Environment variables set: `echo $MMITSS_ROOT`
- [ ] Config files exist: `ls $MMITSS_ROOT/config/speedway-sample/field/*/bin/supervisord.conf`
- [ ] Container logs clean: `docker logs speedway-campbell | grep -i error`

---

## Performance Monitoring

### Memory & CPU Usage

View in Supervisord web interface:
1. Each process shows CPU % and Memory (MB)
2. Watch for processes using excessive resources
3. Look for memory leaks (steadily increasing memory)

Command line alternative:

```bash
# Real-time stats
docker stats speedway-campbell

# Memory usage per process
docker exec speedway-campbell ps aux --sort=-%mem | head -10
```

### Disk Space

```bash
# Container disk usage
docker system df

# Logs size
du -sh /var/lib/docker/containers/*
```

---

## Related Documentation

- [DOCKER_QUICK_REFERENCE.md](../../DOCKER_QUICK_REFERENCE.md)
- [DEPLOYMENT_GUIDE.md](../../DEPLOYMENT_GUIDE.md)
- [ENV_CONFIGURATION_GUIDE.md](../../ENV_CONFIGURATION_GUIDE.md)

---

**Last Updated**: November 15, 2025  
**Status**: Current  
**Web Interfaces**: 4 Supervisord + 4 Flask Apps (optional)
