# MMITSS Environment Refactoring - Iteration Summary

## Date: November 15, 2025

---

## Objective

Remove all hardcoded paths from docker-compose files and replace them with environment variables (`$MMITSS_ROOT`) for portability across different machines and environments.

---

## Changes Made

### 1. **Docker Compose Files Updated**

#### File: `build/docker-compose/simulation/speedway-sample/docker-compose.yml`

**Status**: ✅ FIXED

**Changes**:
- Replaced 5 hardcoded paths with `${MMITSS_ROOT}` variable
- Added comments with original hardcoded paths for reference

**Before**:
```yaml
source: /home/norris/Documents/mmitss-az/config/speedway-sample/simulation/speedway-campbell/nojournal/bin
```

**After**:
```yaml
# source: /home/norris/Documents/mmitss-az/config/speedway-sample/simulation/speedway-campbell/nojournal/bin
source: ${MMITSS_ROOT}/config/speedway-sample/simulation/speedway-campbell/nojournal/bin
```

**Services Updated**:
- ✅ speedway-campbell
- ✅ speedway-cherry
- ✅ speedway-mountain
- ✅ speedway-park
- ✅ simulation_server-tools

#### File: `build/docker-compose/server/speedway-sample/docker-compose.yml`

**Status**: ✅ ALREADY FIXED

This file was already using `${MMITSS_ROOT}` variable. No changes needed.

---

### 2. **.env Files Created**

#### File: `build/docker-compose/server/speedway-sample/.env`

**Status**: ✅ CREATED

```env
MMITSS_ROOT=/home/norris/Documents/mmitss-az
MMITSS_NETWORK_ADAPTER=enp0s3
PROCESSOR=amd64
```

#### File: `build/docker-compose/simulation/speedway-sample/.env`

**Status**: ✅ CREATED

```env
MMITSS_ROOT=/home/norris/Documents/mmitss-az
MMITSS_NETWORK_ADAPTER=enp0s3
PROCESSOR=amd64
```

**Purpose**: These files provide default values that docker-compose automatically loads. Users can override via shell environment variables.

---

### 3. **Setup Script Created**

#### File: `build/scripts/setup-mmitss-env.sh`

**Status**: ✅ CREATED

**Features**:
- Auto-detects MMITSS root directory
- Interactive prompts for network adapter selection
- Processor architecture selection (amd64/arm64/armhf)
- Updates all `.env` files with user inputs
- Displays shell configuration code to add to `~/.bashrc` or `~/.zshrc`
- Color-coded output for clarity
- Validation checks

**Usage**:
```bash
bash build/scripts/setup-mmitss-env.sh
```

---

### 4. **Configuration Guide Created**

#### File: `ENV_CONFIGURATION_GUIDE.md`

**Status**: ✅ CREATED

**Contents**:
- Quick start instructions (automated & manual)
- Environment variable setup for different shells (bash, zsh, fish)
- Docker Compose variable substitution explained
- Network adapter identification guide
- Processor architecture reference table
- Troubleshooting section
- Persistence instructions
- Multi-environment management examples
- Advanced variable expansion techniques

---

## How It Works

### Before (Hardcoded)
```
User A (Linux): /home/alice/mmitss-az → Works
User B (macOS): /Users/bob/mmitss → ❌ Fails (path doesn't exist)
User C (Windows): C:\Users\charlie\mmitss → ❌ Fails (wrong path)
```

### After (Environment Variables)
```
User A: export MMITSS_ROOT=/home/alice/mmitss-az → ✅ Works
User B: export MMITSS_ROOT=/Users/bob/mmitss → ✅ Works
User C: export MMITSS_ROOT=C:\Users\charlie\mmitss → ✅ Works
```

---

## Variable Resolution Order

Docker Compose resolves variables in this priority order:

1. **Shell environment variables** (highest priority)
   ```bash
   export MMITSS_ROOT=/custom/path
   docker-compose up  # Uses /custom/path
   ```

2. **`.env` file** (in docker-compose directory)
   ```env
   MMITSS_ROOT=/home/norris/Documents/mmitss-az
   ```

3. **Docker Compose file defaults** (if specified)

4. **Unresolved** (if not found anywhere)

---

## Quick Setup Instructions

### Option 1: Automated (Recommended)
```bash
cd $MMITSS_ROOT
bash build/scripts/setup-mmitss-env.sh
```

### Option 2: Manual
```bash
# Add to ~/.bashrc or ~/.zshrc
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export MMITSS_NETWORK_ADAPTER=enp0s3
export PROCESSOR=amd64

# Reload shell
source ~/.bashrc
```

### Option 3: Per-Session
```bash
# Set for current terminal only (no persistence)
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
cd build/docker-compose/server/speedway-sample
docker-compose up -d
```

---

## Verification

Test that everything works:

```bash
# 1. Check variables
echo $MMITSS_ROOT
echo $MMITSS_NETWORK_ADAPTER
echo $PROCESSOR

# 2. View substituted paths
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
docker-compose config | grep -A3 "source:"

# 3. Start services
docker-compose up -d
docker-compose ps
```

---

## Benefits

| Aspect | Before | After |
|--------|--------|-------|
| **Portability** | ❌ Hardcoded to one user | ✅ Works on any machine |
| **Multi-user** | ❌ Requires manual edits | ✅ Each user sets own path |
| **Team projects** | ❌ Path conflicts | ✅ Automatic resolution |
| **CI/CD** | ❌ Path hardcoded | ✅ Environment-aware |
| **Maintenance** | ❌ Edit each file | ✅ Single `.env` file |
| **Documentation** | ❌ Not self-documenting | ✅ Clear intent |

---

## Files Modified

| File | Status | Change |
|------|--------|--------|
| `build/docker-compose/simulation/speedway-sample/docker-compose.yml` | 🔧 Modified | Hardcoded paths → `${MMITSS_ROOT}` variables |
| `build/docker-compose/server/speedway-sample/docker-compose.yml` | ✅ Already Fixed | No changes needed |
| `build/docker-compose/server/speedway-sample/.env` | ✨ New | Global environment configuration |
| `build/docker-compose/simulation/speedway-sample/.env` | ✨ New | Global environment configuration |
| `build/scripts/setup-mmitss-env.sh` | ✨ New | Interactive setup automation |
| `ENV_CONFIGURATION_GUIDE.md` | ✨ New | Comprehensive configuration documentation |

---

## Related Documentation

- **Quick Reference**: See `DOCKER_QUICK_REFERENCE.md`
- **Deployment Guide**: See `DEPLOYMENT_GUIDE.md`
- **Configuration Details**: See `ENV_CONFIGURATION_GUIDE.md` (NEW)
- **Previous Session**: See `DOCKER_SESSION_COMPLETE.md`

---

## Future Improvements

1. **Add health checks** in docker-compose files
2. **Create validation script** to verify all paths exist
3. **Support for relative paths** (e.g., `../config`)
4. **Docker Compose override files** (.env.production, .env.development)
5. **Kubernetes manifest generation** with equivalent env var support

---

## Rollback Instructions

If needed to revert to hardcoded paths:

```bash
# Restore from git
git checkout build/docker-compose/simulation/speedway-sample/docker-compose.yml

# Remove new files
rm build/docker-compose/*/.env
rm build/scripts/setup-mmitss-env.sh
rm ENV_CONFIGURATION_GUIDE.md
```

---

## Status: ✅ COMPLETE

All hardcoded paths have been removed and replaced with environment variables. The setup is now portable across different machines and user environments.

**Next Action**: Users should run the setup script or follow the manual configuration guide to set their `MMITSS_ROOT` variable.

---

**Completed By**: GitHub Copilot  
**Date**: November 15, 2025  
**Iteration**: 1  
**Status**: READY FOR USE
