# build

This directory acts as a warehouse for four elements:
- Application binaries (build/bin)
- Dockerfiles for different types of deployment systems (build/dockerfiles)
- Docker-compose files for different types deployment systems (build/docker-compose)
- Build and helper scripts (build/scripts)

---

## Build Documentation Update (November 2025)

### Critical Fixes Applied

#### 1. Docker-Compose YAML Validation ✅
**Issue:** Invalid YAML syntax prevented docker-compose deployment  
**Status:** FIXED - All docker-compose files now pass validation  
**Files Updated:** `docker-compose/server/speedway-sample/docker-compose.yml` and related files

```bash
# Verify fixes
docker-compose -f docker-compose/server/speedway-sample/docker-compose.yml config
```

#### 2. Error Handling Improvements ✅
**Issue:** Silent failures made debugging impossible  
**Status:** FIXED - All components now log errors to stderr  
**Impact:** Better visibility into configuration and runtime issues

#### 3. Memory Management Corrections ✅
**Issue:** Zero-sized array allocations caused crashes  
**Status:** FIXED - Memory allocation now occurs after proper initialization  
**Files Updated:** `scripts/build-mmitss.sh` dependencies and related tools

---

## Building MMITSS

### Prerequisites
- Ubuntu 18.04 LTS
- Docker and Docker-compose installed
- GCC/G++ with C++14 support
- Make build system

### Build Steps

```bash
# 1. Set environment variables
export MMITSS_ROOT=/path/to/mmitss-az
export MMITSS_NETWORK_ADAPTER=eth0  # or your network adapter
export PROCESSOR=x86                  # or arm

# 2. Navigate to build directory
cd build/

# 3. Run build script
./scripts/build-mmitss.sh

# 4. Verify build
ls -la bin/  # Check for compiled binaries
```

### Build Troubleshooting

**Issue:** Build fails with JSON parsing errors
- **Cause:** Updated error handling now validates JSON configurations
- **Solution:** Check `../lib/mmitss.conf` for valid JSON syntax
- **Validation:** `cat ../lib/mmitss.conf | python -m json.tool`

**Issue:** Segmentation fault during build/test
- **Cause:** Previous zero-sized array allocations issue
- **Solution:** Rebuild using updated source code
- **Verification:** Run `make clean && make` to rebuild from scratch

**Issue:** Docker image build fails
- **Cause:** Possible network adapter misconfiguration
- **Solution:** Verify `MMITSS_NETWORK_ADAPTER` is set correctly
- **Check:** Run `ifconfig` to list available adapters

---

## Configuration Files

### Key Configuration Files
- `../lib/mmitss.conf` - Main system configuration (JSON format)
- `../config/speedway-sample/` - Sample configurations for testing
- `dockerfiles/*/Dockerfile` - Container definitions

### Configuration Validation

All configuration files are now validated with improved error reporting:

```bash
# Validate main config
python -m json.tool < ../lib/mmitss.conf > /dev/null && echo "Valid"

# Check Docker setup
docker-compose config -f docker-compose/server/speedway-sample/docker-compose.yml

# Test component startup
docker run --rm [image-name] --config-check  # If supported
```

---

## Deployment

### Docker-Compose Deployment

```bash
# Navigate to deployment directory
cd docker-compose/server/speedway-sample/

# Set required environment variables
export MMITSS_ROOT=/path/to/mmitss-az
export MMITSS_NETWORK_ADAPTER=eth0
export PROCESSOR=x86

# Validate configuration
docker-compose config

# Start services
docker-compose up -d

# Monitor startup
docker-compose logs -f
```

### Available Deployment Configurations
- **Server-based:** `docker-compose/server/` - Multiple intersections on single machine
- **Simulation:** `docker-compose/simulation/` - Simulated intersections with VISSIM integration

---

## Post-Build Verification

### Check Build Artifacts
```bash
# Binary locations
ls -la bin/*/                    # Check compiled applications
ls -la dockerfiles/*/           # Check container definitions

# Test compilation
cd ../src/ && make test         # Run component tests
```

### Verify Docker Images
```bash
# List built images
docker images | grep mmitss

# Test container startup
docker-compose config  # Validate setup
```

---

## Recent Changes and Impact

### November 2025 Updates

| Component | Change | Impact |
|-----------|--------|--------|
| Docker-Compose YAML | Fixed syntax errors | Deployment now works without validation errors |
| JSON Parsing | Added error logging | Better debugging of config issues |
| Memory Allocation | Fixed zero-sized arrays | Tools no longer crash from uninitialized memory |
| Exception Handling | Improved logging | Python components now report errors clearly |

### Backward Compatibility
- ✅ All changes are backward compatible
- ✅ Existing configurations continue to work
- ✅ No breaking API changes
- ✅ Enhanced error reporting is transparent

---

## Troubleshooting

### Common Build Issues

**Problem:** `make: command not found`
```bash
sudo apt-get install build-essential
```

**Problem:** Missing dependencies
```bash
# Check build script for requirements
cat scripts/build-mmitss.sh | grep apt-get
```

**Problem:** Permission denied on scripts
```bash
chmod +x scripts/*.sh
chmod +x scripts/deployment/*.sh
```

---

## Related Documentation

- Full fix details: See `../FIXES_SUMMARY.md`
- Docker deployment: See `docker-compose/README.md`
- Component-specific: See `bin/*/README.md`
- System architecture: See main `../README.md`

---

## Support and Questions

For issues or questions:
1. Check this documentation and troubleshooting guides
2. Review component-specific README files
3. Check `docker-compose logs` for error details
4. Consult `FIXES_SUMMARY.md` for technical details of recent fixes
