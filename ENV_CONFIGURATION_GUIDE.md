# MMITSS Environment Configuration Guide

## Overview

This guide explains how to configure MMITSS for your local environment using environment variables instead of hardcoded paths. This makes the setup portable across different machines and user accounts.

---

## Quick Start

### Option 1: Automated Setup (Recommended)

Run the automated setup script:

```bash
cd $MMITSS_ROOT
bash build/scripts/setup-mmitss-env.sh
```

This script will:
- Auto-detect your MMITSS root directory
- Prompt you to select your network adapter
- Set the processor architecture
- Update all `.env` files
- Show you how to persist the settings

### Option 2: Manual Setup

#### Step 1: Set Environment Variables

Add these lines to your shell configuration file (`~/.bashrc`, `~/.zshrc`, or equivalent):

```bash
# MMITSS Configuration
export MMITSS_ROOT=/path/to/your/mmitss-az
export MMITSS_NETWORK_ADAPTER=enp0s3  # Change to your network interface
export PROCESSOR=amd64                 # amd64, arm64, or armhf
```

Replace `/path/to/your/mmitss-az` with your actual MMITSS repository path.

#### Step 2: Reload Your Shell

```bash
source ~/.bashrc  # or ~/.zshrc
```

#### Step 3: Verify Configuration

```bash
echo $MMITSS_ROOT
echo $MMITSS_NETWORK_ADAPTER
echo $PROCESSOR
```

---

## Configuration Files

The following `.env` files now support environment variable substitution:

- `build/docker-compose/server/speedway-sample/.env`
- `build/docker-compose/simulation/speedway-sample/.env`

Each contains:

```env
# Example .env file
MMITSS_ROOT=/home/norris/Documents/mmitss-az
MMITSS_NETWORK_ADAPTER=enp0s3
PROCESSOR=amd64
```

**Note**: The `.env` files are automatically loaded by docker-compose, so you don't need to source them manually.

---

## Docker Compose Variable Substitution

### How It Works

Docker Compose will automatically substitute environment variables in `docker-compose.yml`:

- **Old (hardcoded):**
  ```yaml
  source: /home/norris/Documents/mmitss-az/config/speedway-sample/...
  ```

- **New (variable-based):**
  ```yaml
  source: ${MMITSS_ROOT}/config/speedway-sample/...
  ```

### Variable Resolution Order

Docker Compose resolves variables in this order (first match wins):

1. **Shell environment variables** (highest priority)
2. `.env` file in the docker-compose directory
3. Default values in the Compose file (if specified)
4. Undefined (variable remains as-is if not found)

### Example

```bash
# Set environment variable
export MMITSS_ROOT=/custom/path

# This will use /custom/path (shell variable takes precedence)
cd /home/norris/Documents/mmitss-az/build/docker-compose/server/speedway-sample
docker-compose config  # Shows substituted values
```

---

## Network Adapters

### Finding Your Network Adapter

```bash
# List all network interfaces
ip link show

# Example output:
# 1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536
# 2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500
# 3: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500
```

Common names:
- `eth0`, `eth1` - Ethernet
- `enp0s3`, `enp0s25` - Modern naming (en = ethernet, p = PCI)
- `wlan0`, `wlan1` - Wireless
- `docker0` - Docker bridge (don't use for MACVLAN)

Set the one connected to your actual network:

```bash
export MMITSS_NETWORK_ADAPTER=enp0s3
```

---

## Processor Architecture

Select based on your system:

| Value | Architecture | System | Image Tag |
|-------|------------|--------|-----------|
| `amd64` | x86-64 | Intel/AMD 64-bit | `mmitssuarizona/mmitss-mrp-amd64:3.0` |
| `arm64` | ARMv8 | ARM 64-bit (Raspberry Pi 4, etc.) | `mmitssuarizona/mmitss-mrp-arm64:3.0` |
| `armhf` | ARMv7 | ARM 32-bit (Raspberry Pi 3, etc.) | `mmitssuarizona/mmitss-mrp-armhf:3.0` |

Check your system:

```bash
# Show system architecture
uname -m

# For Docker images
docker system info | grep Architecture
```

Set the matching value:

```bash
export PROCESSOR=amd64
```

---

## Troubleshooting

### Issue: "Variable not substituted"

**Symptom**: Docker tries to use `${MMITSS_ROOT}` literally instead of the path

**Solutions**:
1. Verify the `.env` file exists in the docker-compose directory
2. Check the variable is set: `echo $MMITSS_ROOT`
3. Show substituted values: `docker-compose config`
4. Reload your shell: `source ~/.bashrc`

### Issue: "Volume mount permission denied"

**Symptom**: Container fails to mount volume

**Solutions**:
1. Verify `MMITSS_ROOT` points to the correct directory
2. Ensure user has read/write access: `ls -ld $MMITSS_ROOT`
3. Add user to docker group: `sudo usermod -aG docker $USER`

### Issue: "Path not found"

**Symptom**: Container can't find mounted directory

**Solutions**:
1. Verify path exists: `ls -la $MMITSS_ROOT/config/`
2. Check for typos in `.env` file
3. Verify no trailing slashes: `$MMITSS_ROOT/config` not `$MMITSS_ROOT/config/`

---

## Persistence

To make settings permanent across shell sessions:

### Bash

Edit `~/.bashrc`:

```bash
# MMITSS Configuration (add these lines)
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export MMITSS_NETWORK_ADAPTER=enp0s3
export PROCESSOR=amd64
```

Then reload:

```bash
source ~/.bashrc
```

### Zsh

Edit `~/.zshrc`:

```bash
# MMITSS Configuration (add these lines)
export MMITSS_ROOT=/home/norris/Documents/mmitss-az
export MMITSS_NETWORK_ADAPTER=enp0s3
export PROCESSOR=amd64
```

Then reload:

```bash
source ~/.zshrc
```

### Fish Shell

Create/edit `~/.config/fish/config.fish`:

```fish
# MMITSS Configuration
set -gx MMITSS_ROOT /home/norris/Documents/mmitss-az
set -gx MMITSS_NETWORK_ADAPTER enp0s3
set -gx PROCESSOR amd64
```

---

## Verification Checklist

After setup, verify everything works:

```bash
# 1. Check environment variables
echo "MMITSS_ROOT: $MMITSS_ROOT"
echo "MMITSS_NETWORK_ADAPTER: $MMITSS_NETWORK_ADAPTER"
echo "PROCESSOR: $PROCESSOR"

# 2. Verify .env files exist
ls -la $MMITSS_ROOT/build/docker-compose/*/speedway-sample/.env

# 3. Show variable substitution
cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample
docker-compose config | grep -A5 "volumes:"

# 4. Test docker-compose
docker-compose up --dry-run

# 5. Start services
docker-compose up -d
docker-compose ps
```

---

## Multiple Environments

You can manage multiple MMITSS environments by creating environment-specific shell profiles:

### Create environment-specific files

```bash
# ~/.mmitss-dev
export MMITSS_ROOT=/home/user/mmitss-dev
export MMITSS_NETWORK_ADAPTER=eth0
export PROCESSOR=amd64

# ~/.mmitss-prod  
export MMITSS_ROOT=/home/user/mmitss-prod
export MMITSS_NETWORK_ADAPTER=eth1
export PROCESSOR=arm64
```

### Load specific environment

```bash
# Load development environment
source ~/.mmitss-dev

# Load production environment
source ~/.mmitss-prod
```

Or create aliases in `~/.bashrc`:

```bash
alias mmitss-dev='source ~/.mmitss-dev && cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample'
alias mmitss-prod='source ~/.mmitss-prod && cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample'
```

Then use:

```bash
mmitss-dev  # Switches to dev environment
mmitss-prod # Switches to prod environment
```

---

## Advanced: Custom Variable Expansion

You can create derived variables in `.env`:

```env
# .env file
MMITSS_ROOT=/home/user/mmitss-az

# Derived paths
CONFIG_PATH=${MMITSS_ROOT}/config
SCRIPTS_PATH=${MMITSS_ROOT}/build/scripts
BUILD_PATH=${MMITSS_ROOT}/build
```

Then use in docker-compose.yml:

```yaml
volumes:
  - type: bind
    source: ${CONFIG_PATH}/speedway-sample/field/speedway-campbell
    target: /nojournal
```

---

## Related Documentation

- [Docker Environment Variable Documentation](https://docs.docker.com/compose/environment-variables/)
- [DOCKER_QUICK_REFERENCE.md](../../DOCKER_QUICK_REFERENCE.md)
- [DEPLOYMENT_GUIDE.md](../../DEPLOYMENT_GUIDE.md)

---

**Last Updated**: November 15, 2025  
**Version**: 1.0  
**Status**: Current
