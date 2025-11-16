# MMITSS Deployment Guide

**Last Updated:** November 15, 2025  
**Version:** 2.0  
**Status:** Current with November 2025 fixes

---

## Table of Contents
1. [Pre-Deployment Checklist](#pre-deployment-checklist)
2. [Quick Start](#quick-start)
3. [Common Issues & Solutions](#common-issues--solutions)
4. [Configuration Guide](#configuration-guide)
5. [Advanced Troubleshooting](#advanced-troubleshooting)
6. [Performance Tuning](#performance-tuning)

---

## Pre-Deployment Checklist

### System Requirements
- [ ] Ubuntu 18.04 LTS (or compatible Linux)
- [ ] Docker installed and running
- [ ] Docker-compose installed (v1.26+)
- [ ] Minimum 4GB RAM available
- [ ] At least 50GB free disk space

### Environment Setup
```bash
# Add to ~/.bashrc and source it
export MMITSS_ROOT=/path/to/mmitss-az
export MMITSS_NETWORK_ADAPTER=eth0          # Replace with your adapter
export PROCESSOR=x86                         # or arm

# Verify setup
source ~/.bashrc
echo "MMITSS_ROOT: $MMITSS_ROOT"
echo "MMITSS_NETWORK_ADAPTER: $MMITSS_NETWORK_ADAPTER"
echo "PROCESSOR: $PROCESSOR"
```

### Network Adapter Identification
```bash
# List all network adapters
ifconfig

# Typical output shows adapters like: eth0, enp0s31f6, wlp2s0, docker0
# Choose the one you want MMITSS to use (usually connected to network)
```

### Pre-Deployment Validation
```bash
# Check Docker
docker ps                  # Should show running containers (may be empty)
docker --version          # Should show Docker version

# Check Docker-compose
docker-compose --version  # Should show version 1.26+

# Verify network adapter
ifconfig $MMITSS_NETWORK_ADAPTER
# Should show IP address and be UP

# Validate docker-compose file
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
docker-compose config     # Should complete without errors
```

---

## Quick Start

### Simple Deployment (Server Mode)

```bash
# 1. Navigate to deployment directory
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample

# 2. Verify configuration
docker-compose config

# 3. Start services
docker-compose up -d

# 4. Monitor startup (Ctrl+C to exit)
docker-compose logs -f

# 5. Verify services running
docker-compose ps

# 6. Stop services
docker-compose down
```

### Simulation Deployment

```bash
cd $MMITSS_ROOT/build/docker-compose/simulation/speedway-sample
docker-compose up -d
docker-compose logs -f
```

---

## Common Issues & Solutions

### Issue 1: YAML Validation Errors
**Error Message:**
```
Error reading docker-compose.yml: yaml: error: line X: Implicit keys need to be on a single line
```

**Cause:** Malformed YAML syntax in docker-compose file

**Solution:**
```bash
# 1. Update from repository (files fixed in November 2025)
cd $MMITSS_ROOT
git pull origin master

# 2. Verify file is correct
grep "x-var:" build/docker-compose/server/speedway-sample/docker-compose.yml
# Should show: x-var: &IMAGE_MRP mmitssuarizona/mmitss-mrp-$PROCESSOR:2.3

# 3. Revalidate
docker-compose -f build/docker-compose/server/speedway-sample/docker-compose.yml config
```

### Issue 2: JSON Configuration Parse Error
**Error Message:**
```
JSON parsing error in [component_name]: [error details]
```

**Cause:** Invalid JSON in configuration file

**Solution:**
```bash
# 1. Find the config file (usually in container mount)
find /nojournal -name "*.json" -type f 2>/dev/null

# 2. Validate JSON syntax
python -m json.tool < /path/to/config.json > /dev/null
# Should complete without output (if valid)

# 3. Pretty-print to find issues
python -m json.tool < /path/to/config.json | head -20

# 4. Fix any syntax errors
# Common issues:
# - Missing commas between fields
# - Trailing commas in arrays/objects
# - Unquoted string values
# - Wrong data types

# 5. Restart container
docker-compose restart [service_name]
```

### Issue 3: Container Won't Start
**Error Message:**
```
Container exited with code 1
```

**Diagnosis:**
```bash
# Check logs for error details
docker logs [container_name]

# Look for:
# - JSON parsing errors (now visible!)
# - Network configuration issues
# - Missing mounted volumes
# - Permission denied errors
```

**Solution by Error Type:**

**If "File not found":**
```bash
# Verify mount paths exist on host
ls -la /path/to/mounted/directory

# Check docker-compose volume definitions
grep -A5 "volumes:" docker-compose.yml
```

**If "Permission denied":**
```bash
# Fix permissions
sudo chmod 644 /path/to/config/files
sudo chown $USER:$USER /path/to/config/files

# Restart container
docker-compose restart [service_name]
```

**If "JSON parsing error":**
```bash
# See "Issue 2: JSON Configuration Parse Error" above
```

### Issue 4: Network Connectivity Problems
**Error Message:**
```
Cannot connect to [service_name]:[port]
Connection refused / No route to host
```

**Diagnosis:**
```bash
# Check running containers
docker-compose ps

# Check network configuration
docker network inspect $(docker network ls | grep mmitss | awk '{print $1}')

# Test connectivity from host
ping [container_ip]

# Test connectivity from another container
docker exec [container1] ping [container2_ip]
```

**Solution:**
```bash
# 1. Verify network adapter is active
ifconfig $MMITSS_NETWORK_ADAPTER
# Should show status UP with IP address

# 2. Check firewall
sudo ufw status
# Allow container traffic if needed:
# sudo ufw allow from [container_subnet]

# 3. Verify IP addresses in config
cat $MMITSS_ROOT/lib/mmitss.conf | grep -i "ip"

# 4. Restart networking in docker-compose
docker-compose down
docker network prune
docker-compose up -d
```

### Issue 5: Memory Allocation Failures
**Error Message:**
```
std::bad_alloc
malloc: Cannot allocate memory
Segmentation fault
```

**Cause:** Tool trying to allocate zero-sized array (fixed Nov 2025) OR insufficient memory

**Solution:**
```bash
# 1. Check system memory
free -h
# Look for: Available memory should be > 1GB

# 2. Check if issue is fixed
cd $MMITSS_ROOT
git log --oneline | head -5
# Should show recent commits including memory fixes

# 3. If not fixed, rebuild
cd build/
make clean
./scripts/build-mmitss.sh

# 4. Monitor memory usage during deployment
watch -n 1 free -h
```

### Issue 6: Silent Failures (Pre-November 2025)
**Symptom:** Component processes but produces no output or errors

**Solution (Nov 2025+):** Errors are now logged!
```bash
# Check stderr for logged errors
docker logs [container_name] 2>&1 | grep -i error

# For BSM processor specifically
docker logs simulated-bsm-blob-processor 2>&1 | tail -50

# For Python components
python /path/to/script.py 2>&1 | tail -50
```

---

## Configuration Guide

### Environment Variables Required

```bash
# MMITSS_ROOT - Path to repository
export MMITSS_ROOT=/home/user/mmitss-az

# MMITSS_NETWORK_ADAPTER - Network interface
export MMITSS_NETWORK_ADAPTER=eth0

# PROCESSOR - Hardware architecture
export PROCESSOR=x86    # x86 or arm
```

### Docker-Compose Configuration

**Key Variables in docker-compose.yml:**
```yaml
x-var: &IMAGE_MRP mmitssuarizona/mmitss-mrp-$PROCESSOR:2.3
# Image name and version (uses $PROCESSOR env var)

services:
  speedway-campbell:
    container_name: speedway-campbell
    # Container name for reference
    
    volumes:
      - type: bind
        source: $MMITSS_ROOT/config/...   # Host path
        target: /nojournal                  # Container path
    # Mounts configuration from host
    
    networks:
      mmitss_vlan:
        ipv4_address: 10.12.6.3
    # Fixed IP for inter-container communication
```

### Main Configuration File

**Location:** `/nojournal/bin/mmitss-phase3-master-config.json`

**Key Sections:**
```json
{
  "HostIp": "10.12.6.1",                    // RSU host IP
  "ApplicationPlatform": "roadside",        // or "vehicle"
  "PortNumber": {
    "MessageTransceiver": {
      "MessageEncoder": 18001,
      "MessageDecoder": 18002
    },
    "PriorityRequestServer": 18003,
    "TrajectoryAware": 18004
    // ... more ports
  },
  "SystemPerformanceTimeInterval": 30.0,    // Logging interval
  "PeerDataDecoding": true,                 // Enable V2X
  // ... more configuration
}
```

**Validation:**
```bash
python -m json.tool < mmitss-phase3-master-config.json > /dev/null && echo "Valid JSON"
```

---

## Advanced Troubleshooting

### Enable Debug Logging

```bash
# 1. Modify environment in docker-compose.yml
environment:
  - DEBUG=1
  - LOG_LEVEL=DEBUG
  - VERBOSE=1

# 2. Rebuild container
docker-compose down
docker-compose build --no-cache

# 3. Restart with debug output
docker-compose up
```

### Inspect Running Container

```bash
# Access container shell
docker exec -it [container_name] /bin/bash

# Inside container:
ps aux                    # Show running processes
netstat -tlnp            # Show listening ports
cat /nojournal/bin/mmitss-phase3-master-config.json  # View config
tail -100 /nojournal/logs/*.log                       # View logs
```

### Network Packet Capture

```bash
# Capture traffic on network adapter
sudo tcpdump -i $MMITSS_NETWORK_ADAPTER -w capture.pcap

# Later, analyze with wireshark
wireshark capture.pcap
```

### Memory Usage Analysis

```bash
# Monitor container memory in real-time
docker stats [container_name]

# Check for memory leaks
docker exec [container_name] valgrind /path/to/binary
```

### Performance Profiling

```bash
# Profile CPU usage
docker exec [container_name] perf record -F 99 -p [PID]
docker exec [container_name] perf report

# Check thread counts
docker exec [container_name] ps -eLf | wc -l
```

---

## Performance Tuning

### Container Resource Limits

```yaml
# Add to docker-compose.yml
services:
  speedway-campbell:
    deploy:
      resources:
        limits:
          cpus: '2'              # 2 CPU cores
          memory: 2G             # 2GB RAM
        reservations:
          cpus: '1'              # Reserve 1 core
          memory: 1G             # Reserve 1GB
```

### Network Optimization

```bash
# Increase network buffer sizes
sudo sysctl -w net.core.rmem_max=134217728
sudo sysctl -w net.core.wmem_max=134217728

# Persistent changes in /etc/sysctl.conf
net.core.rmem_max = 134217728
net.core.wmem_max = 134217728
```

### Docker Daemon Optimization

```bash
# Edit /etc/docker/daemon.json
{
  "debug": false,
  "log-driver": "json-file",
  "log-opts": {
    "max-size": "10m",
    "max-file": "3"
  },
  "storage-driver": "overlay2"
}

# Restart docker
sudo systemctl restart docker
```

---

## Health Checks

### Container Health

```bash
# Check all containers
docker-compose ps
# Status should be "Up X seconds"

# Check specific container logs for errors
docker-compose logs [service_name] | grep -i error

# Verify port bindings
docker port [container_name]
```

### Service Connectivity

```bash
# From host, verify RSU connectivity
telnet [rsu_ip] [port]

# From container, verify connections
docker exec [container] telnet [service_ip] [port]
```

### Data Flow Verification

```bash
# Monitor traffic
docker logs -f [container_name] | grep -i "decoded\|encoded\|received"

# Count messages
docker logs [container_name] | grep "Decoded BSM" | wc -l
```

---

## Maintenance

### Backup Configuration

```bash
# Backup before deployment
cp -r /nojournal /nojournal.backup.$(date +%Y%m%d)

# Version control config changes
git add config/
git commit -m "Update MMITSS configuration"
```

### Clean Up Old Containers/Images

```bash
# Remove stopped containers
docker-compose down -v

# Remove unused images
docker image prune

# Remove all stopped containers
docker container prune
```

### Update MMITSS

```bash
# Pull latest code
cd $MMITSS_ROOT
git pull origin master

# Rebuild containers
cd build/docker-compose/server/speedway-sample
docker-compose down
docker-compose build --no-cache
docker-compose up -d
```

---

## Support

For issues not covered here:

1. **Check logs first:**
   ```bash
   docker-compose logs -f
   docker logs [container_name] 2>&1 | tail -100
   ```

2. **See FIXES_SUMMARY.md** for technical details of November 2025 updates

3. **Review component READMEs** in `src/*/README.md`

4. **Check GitHub Issues:** https://github.com/mmitss/mmitss-az/issues

5. **Contact:** See main README.md for contact information

---

**Document Version:** 2.0 (Updated November 15, 2025)  
**Last Revision:** Comprehensive deployment guide with Nov 2025 fixes  
**Status:** Current and Validated
