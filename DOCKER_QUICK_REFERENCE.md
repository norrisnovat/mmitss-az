# MMITSS Docker Quick Reference Card

## Essential Setup
```bash
# Set environment variable (REQUIRED for all commands)
export MMITSS_ROOT=

export MMITSS_NETWORK_ADAPTER=enp0s3

# Navigate to working directory
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
```

---

## Quick Commands

### Start All Services
```bash
docker-compose up -d
```

### Stop All Services
```bash
docker-compose down
```

### Check Status
```bash
docker-compose ps
docker ps --format "table {{.Names}}\t{{.Status}}"
```

### View Logs
```bash
# All containers
docker-compose logs -f

# Specific container
docker logs speedway-campbell -f
docker logs speedway-cherry --tail=100

# Search for errors
docker-compose logs | grep -i error
```

### Restart Services
```bash
docker-compose restart
docker-compose restart speedway-campbell
```

### Remove Everything (Clean Slate)
```bash
docker-compose down -v
docker network prune
docker image prune
```

---

## Diagnostics

### Check Container Details
```bash
# Running containers
docker ps

# All containers (including stopped)
docker ps -a

# Network information
docker network inspect speedway-sample_mmitss_vlan

# Container statistics
docker stats

# Container processes
docker top speedway-campbell
```

### Service Verification
```bash
# Count running MMITSS services
docker logs speedway-campbell | grep "success.*RUNNING" | wc -l

# Check for errors
docker logs speedway-campbell | grep -i error | tail -10

# Check supervisor status
docker logs speedway-campbell | grep -E "(CRITICAL|ERROR|FAIL)"
```

### Network Testing
```bash
# Test inter-container connectivity
docker exec speedway-campbell ping speedway-cherry
docker exec speedway-campbell ping speedway-mountain
docker exec speedway-campbell ping speedway-park

# Test service ports
docker exec speedway-campbell netstat -tlnp | grep 9001
```

---

## Configuration Files

### Key Locations
```
MMITSS_ROOT/
├── build/docker-compose/server/speedway-sample/
│   └── docker-compose.yml          ← Main orchestration file
├── config/speedway-sample/field/
│   ├── speedway-campbell/
│   │   ├── bin/
│   │   │   ├── supervisord.conf    ← Process manager config
│   │   │   └── mmitss-*.json       ← MMITSS configuration
│   │   └── nojournal/              ← Mounted as /nojournal in container
│   ├── speedway-cherry/
│   ├── speedway-mountain/
│   └── speedway-park/
└── docs/
    ├── DEPLOYMENT_GUIDE.md         ← Troubleshooting & setup
    ├── DOCKER_TEST_REPORT.md       ← Technical analysis
    └── DOCKER_SESSION_COMPLETE.md  ← This session's work
```

### Edit Configuration
```bash
# Edit supervisord config
nano $MMITSS_ROOT/config/speedway-sample/field/speedway-campbell/bin/supervisord.conf

# Edit MMITSS config
nano $MMITSS_ROOT/config/speedway-sample/field/speedway-campbell/bin/mmitss-phase3-master-config.json

# After editing, restart container
docker-compose restart speedway-campbell
```

---

## Network Options

### Current: BRIDGE (Default)
- For local testing and development
- Simple setup, containers communicate via DNS
- No special host configuration needed

### MACVLAN (Production)
Edit docker-compose.yml networks section:
```yaml
networks:
    mmitss_vlan:
        driver: macvlan
        driver_opts:
            parent: $MMITSS_NETWORK_ADAPTER
        ipam:
            config:
              - subnet: 10.12.6.0/24
```
Also uncomment `ipv4_address` in each service.

### OVERLAY (Docker Swarm)
Requires Docker Swarm to be initialized first.

---

## Common Issues & Fixes

### Issue: "Cannot connect to daemon" 
**Fix**: Ensure Docker daemon is running
```bash
sudo systemctl status docker
sudo systemctl start docker
```

### Issue: "Permission denied"
**Fix**: Add user to docker group
```bash
sudo usermod -aG docker $USER
# Log out and back in
```

### Issue: "Address already in use"
**Fix**: Container already running or port in use
```bash
docker ps -a | grep speedway
docker rm -f <container_name>
```

### Issue: Container exits immediately
**Fix**: Check logs
```bash
docker logs <container_name> 2>&1 | tail -30
```

### Issue: "Config file not found"
**Fix**: Set MMITSS_ROOT correctly
```bash
echo $MMITSS_ROOT  # Should show full path
# If not set, run: export MMITSS_ROOT=/home/norris/Documents/mmitss-az
```

---

## Performance Monitoring

### Check Resource Usage
```bash
# Real-time stats
docker stats

# Memory usage
docker stats --no-stream

# Specific container
docker stats speedway-campbell
```

### Check Disk Space
```bash
docker system df
docker system prune -a  # Clean up unused images/containers
```

### Check Network
```bash
docker network ls
docker network inspect speedway-sample_mmitss_vlan
```

---

## Useful One-Liners

```bash
# Kill all containers
docker kill $(docker ps -q)

# Remove all stopped containers
docker container prune -f

# View all logs from all containers
docker-compose logs --timestamps

# Watch logs in real-time with timestamps
docker-compose logs -f --timestamps

# Get container IP addresses
docker ps -q | xargs docker inspect --format='{{.Name}} - {{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}'

# Execute command in running container
docker exec -it speedway-campbell /bin/sh

# Copy file from container
docker cp speedway-campbell:/nojournal/bin/supervisord.conf ./

# Check container resource limits
docker inspect speedway-campbell | grep -E "Memory|Cpu"
```

---

## Recovery Procedures

### Restart Single Container
```bash
docker-compose restart speedway-campbell
```

### Force Restart All
```bash
docker-compose down
docker-compose up -d
```

### Clear All and Start Fresh
```bash
docker-compose down -v
docker system prune -a
docker-compose up -d
```

### Backup Before Changes
```bash
docker-compose ps > /tmp/status_backup.txt
docker exec speedway-campbell tar czf /tmp/config_backup.tar.gz /nojournal
```

---

## Documentation References

| Document | Purpose |
|----------|---------|
| DEPLOYMENT_GUIDE.md | Comprehensive deployment and troubleshooting |
| DOCKER_TEST_REPORT.md | Technical details and solutions |
| DOCKER_SESSION_COMPLETE.md | Full session summary and accomplishments |
| docker-compose.yml | Container orchestration and network config |
| supervisord.conf | Process management configuration |

---

## Emergency Contacts / Logs

### Application Logs
```bash
docker logs speedway-campbell 2>&1 | grep -i "error\|critical\|fail"
```

### System Logs
```bash
sudo journalctl -u docker -f
```

### Network Logs
```bash
docker network inspect speedway-sample_mmitss_vlan
```

---

## Checklists

### Pre-Deployment
- [ ] Docker running (`docker ps`)
- [ ] MMITSS_ROOT set correctly
- [ ] Network interface available
- [ ] Config files in place
- [ ] Sufficient disk space (50GB+)
- [ ] Sufficient RAM (4GB+)

### Post-Deployment
- [ ] All 4 containers running (`docker-compose ps`)
- [ ] 233+ services in RUNNING state
- [ ] No errors in logs (`docker-compose logs | grep -i error`)
- [ ] Network reachable (`docker network inspect`)
- [ ] supervisord bound to port 9001

### Daily Operations
- [ ] Check container health (`docker stats`)
- [ ] Monitor logs for errors
- [ ] Verify inter-container communication
- [ ] Check disk space usage

---

**Last Updated**: November 15, 2025  
**Status**: Current and Validated  
**Quick Start**: Set MMITSS_ROOT, then run `docker-compose up -d`
