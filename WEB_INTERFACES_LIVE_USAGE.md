# 🚀 MMITSS Web Interfaces - LIVE & RUNNING

## ✅ Status: ALL SYSTEMS ONLINE

```
✓ speedway-campbell    RUNNING (45 minutes)
✓ speedway-cherry      RUNNING (45 minutes)  
✓ speedway-mountain    RUNNING (53 minutes)
✓ speedway-park        RUNNING (45 minutes)

✓ Port 9001: RESPONDING
✓ Port 9002: RESPONDING
✓ Port 9003: RESPONDING
✓ Port 9004: RESPONDING
```

---

## 🌐 OPEN IN BROWSER NOW

Click on these URLs to access your interfaces:

### Campbell (Port 9001)
👉 **http://localhost:9001**

### Cherry (Port 9002)
👉 **http://localhost:9002**

### Mountain (Port 9003)
👉 **http://localhost:9003**

### Park (Port 9004)
👉 **http://localhost:9004**

---

## 📖 HOW TO USE SUPERVISORD INTERFACE

Once you open any of the 4 URLs above, you'll see the Supervisord dashboard. Here's how to use it:

### 1. PROCESS STATUS VIEW (Main Dashboard)

You'll see a table with columns:

```
Process Name              Status     Uptime    CPU %   Memory (MB)
─────────────────────────────────────────────────────────────────
MsgEncoder               RUNNING    23:14     0.1%    45 MB
MessageDistributor       RUNNING    18:32     0.2%    52 MB
PriorityRequestServer    RUNNING    15:47     0.3%    61 MB
TrafficControllerIface   RUNNING    14:19     0.1%    38 MB
(... 230+ more processes ...)
```

**What to look for:**
- 🟢 **RUNNING** = Service is healthy (good!)
- 🟡 **STOPPED** = Service is off (manual stop)
- 🔴 **ERROR** = Service crashed (needs investigation)
- ⚫ **FATAL** = Service won't start (serious issue)

### 2. CONTROL BUTTONS (Next to each process)

Each process has buttons:

- **START** - Click to start a stopped process
  ```
  Use when: Service is STOPPED and you want to turn it on
  ```

- **STOP** - Click to stop a running process
  ```
  Use when: You need to shut down a service
  ```

- **RESTART** - Click to restart a service
  ```
  Use when: Service is acting weird, needs fresh start
  ```

### 3. VIEW PROCESS LOGS

**Click the process name** to see its logs:

```
Example: Click "MsgEncoder" → Opens log viewer
Shows all console output from that service
```

**What to look for in logs:**
```
✓ Normal:
  [INFO] Service started
  [DEBUG] Processing request
  [INFO] Connected to port 5001

✗ Problems:
  [ERROR] Failed to connect
  [WARN] Connection timeout
  [FATAL] Segmentation fault
```

### 4. MONITOR RESOURCE USAGE

In the dashboard table:

```
CPU %     → How much processor the service is using (0-100%)
Memory    → How much RAM the service is using (in MB)
Uptime    → How long the service has been running
```

**Warning signs:**
- CPU stuck at 100% = Service might be looping
- Memory constantly increasing = Possible memory leak
- Frequent restarts = Service keeps crashing

---

## 🎯 STEP-BY-STEP EXAMPLE

### Example 1: Check if all services are running

1. Open **http://localhost:9001** (Campbell)
2. Look at the dashboard table
3. Count processes with status = **RUNNING**
4. Expected: **230+** processes
5. ✓ If you see 230+, system is healthy!
6. ✗ If you see fewer, click on STOPPED/ERROR processes to see why

### Example 2: Check a specific service's logs

1. Open **http://localhost:9001**
2. Scroll down and find **"MsgEncoder"** in the list
3. Click **"MsgEncoder"** (the blue text name)
4. A log window opens showing all output from that service
5. Look for [ERROR] or [FATAL] messages
6. Copy any errors for troubleshooting

### Example 3: Restart a failed service

1. Open **http://localhost:9001**
2. Find the service in RED status (ERROR or FATAL)
3. Click the **RESTART** button
4. Watch the status change:
   - First it says "ERROR"
   - Then "STARTING" appears
   - Then it goes back to "RUNNING" (success!)
   - Or stays "ERROR" (problem)

### Example 4: Monitor CPU/Memory usage

1. Open **http://localhost:9001**
2. Look at the "CPU %" column
3. Healthy: Most services show 0.0% - 0.3%
4. Look at "Memory (MB)" column
5. Healthy: Most services use 30-100 MB each
6. Watch for any service with CPU stuck at 100%

---

## 💡 QUICK REFERENCE CARD

### What Each Port Shows

| Port | Container | Purpose |
|------|-----------|---------|
| **9001** | Campbell | Main traffic signal controller |
| **9002** | Cherry | Secondary traffic controller |
| **9003** | Mountain | Tertiary traffic controller |
| **9004** | Park | Fourth traffic controller |

All 4 show the same Supervisord interface - manage any from any port.

### Common Tasks

**Task**: Stop the message encoder
```
1. Open http://localhost:9001
2. Find "MsgEncoder"
3. Click STOP button
4. Status changes to STOPPED
```

**Task**: Check why a service crashed
```
1. Open http://localhost:9001
2. Find the service with ERROR status
3. Click on the service name
4. Read the log output
5. Look for [ERROR] messages
```

**Task**: Restart all services
```
Method 1 - Via UI:
1. Open http://localhost:9001
2. Scroll to top, click "Restart All"
3. Wait 30 seconds for all to restart

Method 2 - Via command line:
docker exec speedway-campbell supervisorctl restart all
```

**Task**: View live system logs
```
In terminal:
docker-compose logs -f

Press Ctrl+C to stop
```

---

## 🔍 UNDERSTANDING THE DASHBOARD

### Top Section - System Status

Shows:
- Uptime of supervisord itself
- Number of processes managed
- Current load

### Middle Section - Process List

Shows all 230+ processes with:
- Name
- Status
- Uptime
- CPU usage
- Memory usage
- Action buttons

### Bottom Section - Log Viewer

Opens when you click a process name to show:
- Real-time output
- Error messages
- Debug information
- Timestamps

---

## ⚠️ WHAT TO WATCH FOR

### Green/Healthy Signs ✓
```
✓ All processes showing RUNNING
✓ CPU usage under 1% per service
✓ Memory stable (not constantly growing)
✓ No ERROR or FATAL statuses
✓ Services running for hours/days without restart
```

### Yellow/Warning Signs ⚠️
```
⚠ Some processes STOPPED (but intentional?)
⚠ CPU usage 5-20% for a single service
⚠ Memory slowly increasing over time
⚠ Services restarting every few minutes
⚠ New ERROR messages in logs
```

### Red/Critical Signs 🔴
```
🔴 Many processes in ERROR/FATAL
🔴 CPU stuck at 100%
🔴 Memory suddenly jumps to hundreds of MB
🔴 Services can't restart
🔴 New CRITICAL errors in logs
```

---

## 📊 COMMANDS FOR POWER USERS

### Access Container Command Line

```bash
# Enter the Campbell container
docker exec -it speedway-campbell /bin/bash

# Once inside, use supervisorctl:
supervisorctl status              # Show all processes
supervisorctl status MsgEncoder   # Show one process
supervisorctl start MsgEncoder    # Start a process
supervisorctl stop MsgEncoder     # Stop a process
supervisorctl restart all         # Restart everything
supervisorctl tail MsgEncoder     # View logs
supervisorctl tail MsgEncoder -f  # Follow logs (live)
exit                              # Leave container
```

### View Logs from Host

```bash
# All logs from all containers
docker-compose logs -f

# Specific container
docker logs -f speedway-campbell

# Search for errors
docker logs speedway-campbell | grep ERROR

# Last 50 lines
docker logs speedway-campbell | tail -50
```

### Check System Resources

```bash
# Real-time stats
docker stats

# Memory of each container
docker stats --no-stream

# Process list in container
docker exec speedway-campbell ps aux
```

---

## 🚨 TROUBLESHOOTING

### Issue: "Cannot connect to localhost:9001"

**Solution:**
```bash
# 1. Check if containers running
docker-compose ps

# 2. Check port is listening
netstat -tlnp | grep 9001

# 3. If not listening, containers may not be started
docker-compose down
docker-compose up -d
sleep 20  # Wait for startup
```

### Issue: Page loads but shows no processes

**Solution:**
```bash
# Supervisord may still be starting
# Wait 20-30 seconds and reload page

# Or check logs
docker logs speedway-campbell | tail -20
```

### Issue: "ERROR" or "FATAL" processes

**Solution:**
```bash
# 1. Click the process name to see logs
# 2. Look for [ERROR] messages
# 3. Try restarting:
#    - Click RESTART button, or
#    - docker exec speedway-campbell supervisorctl restart ProcessName
# 4. If keeps failing, check system resources
```

### Issue: High CPU usage

**Solution:**
```bash
# 1. See which process is using CPU
docker stats

# 2. Click that process to view logs
# 3. Look for errors or infinite loops
# 4. Try restarting the service
# 5. If problem persists, may need code fix
```

---

## 📱 MOBILE/TABLET ACCESS

The Supervisord interface works on phones/tablets too!

**From another computer:**
```bash
# Find your VM's IP
hostname -I

# Then from another machine, open in browser:
http://<VM_IP>:9001
http://<VM_IP>:9002
http://<VM_IP>:9003
http://<VM_IP>:9004
```

Example:
```
VM IP is 192.168.1.100
Open: http://192.168.1.100:9001 on your phone
```

---

## 🎓 LEARNING TIPS

1. **Start with Campbell** (port 9001) - get familiar with interface
2. **Find a RUNNING process** - click it to see logs
3. **Try restarting a service** - click STOP then START
4. **Watch the logs** - see what services are doing
5. **Monitor resources** - see CPU/memory in dashboard
6. **Command line commands** - once comfortable with UI, try CLI

---

## 📚 MORE INFORMATION

For detailed information, see:
- `WEB_INTERFACES_GUIDE.md` - Full documentation
- `WEB_INTERFACES_QUICK_LAUNCH.md` - Quick reference
- `DOCKER_QUICK_REFERENCE.md` - Docker commands
- `DEPLOYMENT_GUIDE.md` - System deployment

---

## ✨ YOU'RE READY!

Open any of these links in your browser:
- http://localhost:9001 ← START HERE
- http://localhost:9002
- http://localhost:9003
- http://localhost:9004

**What you should see:**
- A web dashboard with a table of 230+ processes
- Green "RUNNING" status on most processes
- Buttons to START/STOP/RESTART each
- Real-time CPU and memory usage

**Congratulations!** Your MMITSS system is running and monitored! 🎉

---

**Status**: ✅ **LIVE AND RUNNING**  
**Last Updated**: November 15, 2025  
**All 4 Interfaces**: RESPONDING
