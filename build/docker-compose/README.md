# docker-compose

This directory is a warehouse of docker-compose files customized for two types of deployment environments:
- Server-based deployment, where containers of multiple intersection run on a single machine.
- Simulation deployment, where containers of multiple simulated intersections, along with a container hosting simulation tools run on a single machine.

To use any of the available docker-compose files under this directory, following environment variables must be added to the ~/.bashrc file:
1. MMITSS_ROOT: Local path of the directory where the mmitss-az git repository is cloned.
2. MMITSS_NETWORK_ADAPTER: Name of the network adapter that will be used by MMITSS containers. This can be obtained from running command `ifconfig` in the console. For example, an output of the `ifconfig` command is shown below, where the name of the network adapter is highlighted with bold text.
<pre>
docker0: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        inet 172.17.0.1  netmask 255.255.0.0  broadcast 172.17.255.255
        inet6 fe80::42:61ff:fe79:ad2  prefixlen 64  scopeid 0x20<link>
        ether 02:42:61:79:0a:d2  txqueuelen 0  (Ethernet)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 11  bytes 1366 (1.3 KB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

<b>enp0s31f6</b>: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500,
        inet 10.12.6.252  netmask 255.255.255.0  broadcast 10.12.6.255
        inet6 fe80::b417:3aa0:cfac:74fc  prefixlen 64  scopeid 0x20<link>
        ether 64:00:6a:7d:15:c5  txqueuelen 1000  (Ethernet)
        RX packets 1883174  bytes 166606949 (166.6 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 22387  bytes 3306725 (3.3 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 19  memory 0xf7e80000-f7ea0000  
        
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1000  (Local Loopback)
        RX packets 26009  bytes 2720630 (2.7 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 26009  bytes 2720630 (2.7 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

wlp2s0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.1.20  netmask 255.255.255.0  broadcast 192.168.1.255
        inet6 fe80::cd1c:d7af:981b:64fa  prefixlen 64  scopeid 0x20<link>
        ether bc:54:2f:d2:0f:32  txqueuelen 1000  (Ethernet)
        RX packets 460409  bytes 449341593 (449.3 MB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 168502  bytes 28321239 (28.3 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
</pre>
3. PROCESSOR: The architecture of the processor. This should take either of the following values:
    - arm
    - x86

---

## Recent Updates (November 2025)

### YAML Configuration Changes
The docker-compose YAML files have been corrected to ensure proper validation. Key changes include:
- **Fixed YAML anchor syntax**: Image references are now on single lines for proper YAML parsing
- **Corrected indentation**: All service definitions now use consistent 4-space indentation
- **Validation**: All files now pass `docker-compose config` validation without errors

### Example: Fixed Configuration
```yaml
# BEFORE (Invalid)
x-var: &IMAGE_MRP
    mmitssuarizona/mmitss-mrp-$PROCESSOR:2.3
    speedway-campbell:
        container_name: speedway-campbell

# AFTER (Valid)
x-var: &IMAGE_MRP mmitssuarizona/mmitss-mrp-$PROCESSOR:2.3

speedway-campbell:
    container_name: speedway-campbell
```

---

## Deployment Troubleshooting Guide

### Issue 1: Docker-Compose Validation Fails
**Error Message:** `Implicit keys need to be on a single line` or similar YAML errors

**Solution:**
- Ensure your docker-compose file uses the latest corrected version from `build/docker-compose/`
- Verify YAML anchors (like `&IMAGE_MRP`) are on the same line as their values
- Check indentation is consistent (4 spaces)
- Run: `docker-compose config` to validate

**Fix Applied (November 2025):**
All docker-compose files have been corrected. If you have local copies, update from repository.

---

### Issue 2: Container Won't Start - JSON Configuration Error
**Error Message:** `JSON parsing error in [component]` in container logs

**Solution:**
- Check that `/nojournal/bin/mmitss-phase3-master-config.json` is properly formatted JSON
- Validate JSON: `cat /path/to/config.json | python -m json.tool`
- Ensure all required fields are present and properly typed
- Check file permissions: `chmod 644 mmitss-phase3-master-config.json`

**Background:** 
Error checking has been improved (November 2025) to log JSON parsing failures instead of silently failing. This helps with debugging configuration issues.

---

### Issue 3: BSM Blob Processing Fails Silently
**Error Message:** No errors visible, but BSM data not processing

**Solution:**
- Check stderr output: `docker logs [container_name] 2>&1 | grep -i error`
- Review detailed error messages that are now logged
- Verify BSM blob format matches expected structure
- Check network connectivity between components

**Background:**
Error handling improved (November 2025) - exceptions are now logged to stderr instead of being silently ignored.

---

### Issue 4: Memory Allocation Errors in Tools
**Error Message:** Segmentation fault in priorityList or similar tools

**Solution:**
- Ensure JSON input files exist and are properly formatted
- Verify required fields are present in JSON (e.g., `noOfRequest`)
- Check system has sufficient free memory
- Use `valgrind` for memory debugging: `valgrind ./priorityList`

**Background:**
Memory allocation bug fixed (November 2025) - arrays now allocated with correct size after reading JSON.

---

### Issue 5: Network Configuration Problems
**Error Message:** `Cannot connect to RSU` or network timeouts

**Checklist:**
- Verify `MMITSS_NETWORK_ADAPTER` environment variable is set correctly
- Confirm IP addresses in config match docker-compose network settings
- Check firewall rules allow container-to-container communication
- Verify network adapter is up: `ifconfig $MMITSS_NETWORK_ADAPTER`
- Test connectivity: `ping [container_ip]` from host or another container

---

## Configuration Changes Summary

### Docker Compose Files Updated
- `build/docker-compose/server/speedway-sample/docker-compose.yml` - YAML syntax corrected
- All related server and simulation docker-compose files - Consistent formatting applied

### Code Changes That May Affect Configuration
1. **JSON Error Logging** - Malformed config files now produce detailed error messages
2. **Improved Error Handling** - Python exceptions now logged instead of silently ignored
3. **Memory Management** - Tool crashes from uninitialized memory fixed

### Required Actions
- No breaking changes to existing configurations
- Backward compatible with current setup
- Recommended: Update to corrected docker-compose files for better error reporting

---

## Pre-Deployment Checklist

- [ ] Set `MMITSS_ROOT` environment variable
- [ ] Set `MMITSS_NETWORK_ADAPTER` environment variable  
- [ ] Set `PROCESSOR` environment variable (arm or x86)
- [ ] Verify docker-compose files pass validation: `docker-compose config`
- [ ] Check all required JSON config files exist and are valid
- [ ] Confirm network adapter is active: `ifconfig`
- [ ] Test Docker installation: `docker ps`
- [ ] Review container logs for any startup errors

---

## Useful Commands

```bash
# Validate docker-compose configuration
docker-compose -f build/docker-compose/server/speedway-sample/docker-compose.yml config

# Dry-run deployment (don't actually start containers)
docker-compose -f build/docker-compose/server/speedway-sample/docker-compose.yml up --dry-run

# Start containers
docker-compose -f build/docker-compose/server/speedway-sample/docker-compose.yml up -d

# View logs with error highlighting
docker-compose logs -f 2>&1 | grep -i error

# Check specific container logs
docker logs [container_name] -f

# Validate JSON config file
cat /path/to/config.json | python -m json.tool

# Check network adapter
ifconfig $MMITSS_NETWORK_ADAPTER
```

---

## Related Documentation
- See `FIXES_SUMMARY.md` in repository root for detailed technical changes
- Review individual component READMEs for specific configuration requirements

Above requirements can be automatically satisfied by using the `setup-build-environment.sh` script located in the `mmitss/scripts` directory.
