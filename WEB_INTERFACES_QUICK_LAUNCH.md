# MMITSS Web Interfaces - Quick Launch Card

## ⚡ Ultra-Quick Start (5 steps)

```bash
# 1. Set environment
export MMITSS_ROOT=/home/norris/Documents/mmitss-az

# 2. Go to docker directory
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample

# 3. Start containers
docker-compose up -d

# 4. Wait 10 seconds for Supervisord to start

# 5. Open interfaces (automatic)
bash $MMITSS_ROOT/build/scripts/launch-interfaces.sh
```

**OR manually open in browser:**
- Campbell: http://localhost:9001
- Cherry: http://localhost:9002
- Mountain: http://localhost:9003
- Park: http://localhost:9004

---

## 📍 Access URLs

| Name | URL | Status |
|------|-----|--------|
| **Campbell** | http://localhost:9001 | Supervisord |
| **Cherry** | http://localhost:9002 | Supervisord |
| **Mountain** | http://localhost:9003 | Supervisord |
| **Park** | http://localhost:9004 | Supervisord |

---

## ✅ What You Should See

### Supervisord Dashboard
```
Process Name              Status    Uptime  CPU%  Memory
────────────────────────────────────────────────────────
MsgEncoder               RUNNING   12:34    0.1%  45 MB
MessageDistributor       RUNNING    5:22    0.2%  52 MB
PriorityRequestServer    RUNNING    8:15    0.3%  61 MB
TrafficControllerIface   RUNNING    6:47    0.1%  38 MB
SnmpEngine              RUNNING    4:33    0.0%  28 MB
MapSpatBroadcaster      RUNNING   12:34    0.2%  55 MB
... (200+ more processes)
```

**Expected**: 230+ processes in RUNNING state

---

## 🔧 Common Commands

```bash
# View all containers
docker-compose ps

# View logs
docker-compose logs -f

# Check specific container
docker logs -f speedway-campbell

# Restart all
docker-compose restart

# Stop all
docker-compose down

# Start all
docker-compose up -d

# Check port
lsof -i :9001
```

---

## ⚠️ Troubleshooting

### Issue: "Cannot connect to localhost:9001"

```bash
# Check if containers are running
docker-compose ps

# Check if port is listening
netstat -tlnp | grep 9001

# If not listening, wait 10-20 seconds for startup
```

### Issue: Supervisord shows no processes

```bash
# Check if supervisord started
docker exec speedway-campbell supervisorctl status | head

# Check logs for errors
docker logs speedway-campbell | grep -i error

# Restart supervisord
docker exec speedway-campbell supervisorctl restart all
```

### Issue: Port already in use

```bash
# Find what's using the port
lsof -i :9001

# Kill the process or change port in docker-compose.yml
docker-compose down  # This will free up ports
docker-compose up -d
```

---

## 🚀 Automated Launch

The launch script checks everything and opens all 4 interfaces:

```bash
bash $MMITSS_ROOT/build/scripts/launch-interfaces.sh
```

**What it does:**
1. ✓ Verifies Docker is installed
2. ✓ Checks if containers are running
3. ✓ Tests if ports are responding
4. ✓ Opens all 4 URLs in your browser
5. ✓ Shows what to do next

---

## 📊 Monitoring

### Via Supervisord Web UI
- Click any process name to see its logs
- Watch for ERROR or FATAL status
- Check memory/CPU usage

### Via Command Line
```bash
# Real-time monitoring
docker stats speedway-campbell

# Check for errors
docker-compose logs | grep -i error | tail -20

# Process-specific logs
docker exec speedway-campbell supervisorctl tail MsgEncoder
```

---

## 🛑 Stopping Services

```bash
# Stop all (but keep containers)
docker-compose stop

# Stop specific container
docker-compose stop speedway-campbell

# Stop and remove containers
docker-compose down

# Stop and remove everything
docker-compose down -v  # Also removes volumes
```

---

## 📚 Documentation

- **Full Guide**: [WEB_INTERFACES_GUIDE.md](../WEB_INTERFACES_GUIDE.md)
- **Docker Reference**: [DOCKER_QUICK_REFERENCE.md](../DOCKER_QUICK_REFERENCE.md)
- **Deployment**: [DEPLOYMENT_GUIDE.md](../DEPLOYMENT_GUIDE.md)
- **Environment Setup**: [ENV_CONFIGURATION_GUIDE.md](../ENV_CONFIGURATION_GUIDE.md)

---

## 💡 Pro Tips

### Tip 1: SSH into Container
```bash
docker exec -it speedway-campbell /bin/bash
supervisorctl status    # See all processes
```

### Tip 2: View Specific Service Logs
```bash
docker exec speedway-campbell supervisorctl tail MsgEncoder -f
```

### Tip 3: Keep Terminal Open
```bash
docker-compose logs -f
# Keeps showing live logs until you Ctrl+C
```

### Tip 4: Check System Resources
```bash
docker stats --no-stream
```

### Tip 5: Multiple Terminals
```bash
# Terminal 1: Keep logs streaming
docker-compose logs -f

# Terminal 2: Monitor resources
docker stats

# Terminal 3: Work on other tasks
```

---

## 🎯 Status Check Workflow

```bash
# 1. Are containers running?
docker-compose ps

# 2. Can I reach Supervisord?
curl http://localhost:9001

# 3. How many processes are running?
docker exec speedway-campbell supervisorctl status | wc -l

# 4. Are there any errors?
docker logs speedway-campbell | grep -i error
```

---

**Quick Start**: `bash $MMITSS_ROOT/build/scripts/launch-interfaces.sh`

**Date**: November 15, 2025  
**Status**: Ready to Use ✅
