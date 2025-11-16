# MMITSS Docker Testing Report - November 15, 2025

## Current Status

### ✅ Successfully Completed
1. **Environment Setup**: Docker, docker-compose, MMITSS_ROOT all configured
2. **YAML Validation**: Fixed docker-compose YAML syntax errors
3. **Network Configuration**: Implemented bridge network with full documentation
4. **One Container Running**: `speedway-mountain` successfully started with 11 MMITSS services

### 🔴 Current Issues
1. **supervisord IP Binding Error**: Other containers fail on inet_http_server binding
2. **Error**: `socket.error reported errno.EADDRNOTAVAIL (99)`
3. **Root Cause**: Config files hardcoded to 10.12.6.3 but bridge network assigns dynamic IPs

---

## Detailed Analysis

### Working Configuration: speedway-mountain
- ✅ Docker container started and running
- ✅ 11 MMITSS services successfully spawned and in RUNNING state:
  - M_WirelessMsgDecoder
  - M_MsgEncoder
  - M_MapSpatBroadcaster
  - M_PriorityRequestSolver
  - M_PriorityRequestServer
  - M_TrafficControllerInterface
  - M_TrajectoryAware
  - M_SystemInterface
  - M_V2XDataCollector
  - M_SignalCoordinationRequestGenerator
  - M_SnmpEngine (exits gracefully - expected behavior)

### Failing Containers: campbell, cherry, park
- ❌ supervisord fails to start
- ❌ Error on line: `port=10.12.6.3:9001` in supervisord.conf
- ❌ Cannot bind to 10.12.6.3 because dynamic IPs assigned by bridge network

---

## Technical Details

### The Problem
Each container config has:
```properties
[inet_http_server]
port=10.12.6.3:9001    # Hardcoded IP - not available on bridge network
```

When using bridge network driver:
- Docker assigns IPs dynamically from the subnet
- Each container gets a different IP
- campbell, cherry, park can't bind to 10.12.6.3
- mountain somehow works (likely because it's the first/only one getting that IP)

### Why mountain Works
The `speedway-mountain` container successfully:
1. Gets assigned an IP (likely 10.12.6.3 since it's first)
2. supervisord binds to 10.12.6.3:9001
3. All child processes start successfully

### Why Others Fail
- campbell, cherry, park try to bind to 10.12.6.3
- That IP is already taken or assigned differently
- `errno.EADDRNOTAVAIL (99)` = "Address not available"

---

## Network Configuration Options Documented

The docker-compose.yml now includes detailed comments for:
1. **BRIDGE** - Recommended for testing (current config)
2. **MACVLAN** - For production with hardware integration
3. **OVERLAY** - For Docker Swarm deployments
4. **HOST** - For raw host network access

---

## Solutions (To Implement)

### Option A: Fix supervisord.conf (Recommended)
Change `port=10.12.6.3:9001` to `port=*:9001` or `port=127.0.0.1:9001`

**File to modify:**
```
/home/norris/Documents/mmitss-az/config/speedway-sample/field/[each-container]/bin/supervisord.conf
```

**Line 19 in each supervisord.conf:**
```properties
# FROM:
port=10.12.6.3:9001

# TO (listen on all interfaces):
port=*:9001

# OR (localhost only):
port=127.0.0.1:9001
```

### Option B: Use MACVLAN Network
- Requires setting MMITSS_NETWORK_ADAPTER
- Assign specific IPs to each container
- Uncomment MACVLAN section in docker-compose.yml

### Option C: Create Bridge Network with Fixed IPs
- Create bridge network with --aux-address options
- Manually assign IPs 10.12.6.3, 10.12.6.6, 10.12.6.9, 10.12.6.12
- More complex but possible

---

## Next Steps

### Immediate (Quick Fix)
1. Modify supervisord.conf files for campbell, cherry, park
2. Change inet_http_server port to `*:9001` (listen on all interfaces)
3. Restart containers

### Testing After Fix
```bash
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
cd build/docker-compose/server/speedway-sample
docker-compose restart
docker-compose ps  # All should show Up
```

### Verification
```bash
# Check all services running
docker logs speedway-campbell 2>&1 | grep "success"

# Should see all 11 services in RUNNING state
```

---

## Key Learnings

1. **Bridge Network** - Perfect for local testing, no special setup needed
2. **Dynamic IPs** - Bridge networks assign IPs dynamically
3. **Hardcoded IPs** - Config files assume MACVLAN with fixed IPs
4. **supervisord** - Requires listening interface in config
5. **Docker Logs** - Essential for debugging container startup issues

---

## Files Modified This Session
- `/home/norris/Documents/mmitss-az/build/docker-compose/server/speedway-sample/docker-compose.yml`
  - Fixed YAML syntax
  - Changed network from macvlan to bridge
  - Added comprehensive network documentation
  - Updated to image version 3.0 (available locally)

---

## Current Container Status
- speedway-mountain: ✅ Running (Up 2 minutes)
- speedway-campbell: ❌ Stopped (supervisord error)
- speedway-cherry: ❌ Stopped (supervisord error)
- speedway-park: ❌ Stopped (supervisord error)

**Network**: speedway-sample_mmitss_vlan (bridge, 10.12.6.0/24)

---

**Report Generated**: November 15, 2025  
**Testing Status**: In Progress - Core functionality verified, configuration tweaks needed
