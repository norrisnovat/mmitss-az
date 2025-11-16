#!/bin/bash
# ============================================================================
# MMITSS Environment Setup Script
# ============================================================================
# This script configures the MMITSS_ROOT and other environment variables
# for your local environment. Run this ONCE, and the settings will persist.
#
# Usage:
#   ./setup-mmitss-env.sh
#
# ============================================================================

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}============================================================================${NC}"
echo -e "${BLUE}MMITSS Environment Setup${NC}"
echo -e "${BLUE}============================================================================${NC}"

# Get current script location
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Try to detect MMITSS_ROOT from script location
# Assume script is in: /path/to/mmitss-az/build/scripts/
DETECTED_ROOT="$(dirname "$(dirname "$(dirname "$SCRIPT_DIR")")")"

echo ""
echo -e "${YELLOW}Step 1: Setting MMITSS_ROOT${NC}"
echo "Current location: $SCRIPT_DIR"
echo "Detected MMITSS_ROOT: $DETECTED_ROOT"
echo ""
read -p "Enter MMITSS_ROOT path [${DETECTED_ROOT}]: " USER_ROOT
MMITSS_ROOT="${USER_ROOT:-$DETECTED_ROOT}"

# Validate path exists
if [ ! -d "$MMITSS_ROOT" ]; then
    echo -e "${RED}✗ Error: Directory not found: $MMITSS_ROOT${NC}"
    exit 1
fi

echo -e "${GREEN}✓ MMITSS_ROOT set to: $MMITSS_ROOT${NC}"

# Step 2: Network adapter
echo ""
echo -e "${YELLOW}Step 2: Setting Network Adapter (for MACVLAN mode)${NC}"
echo "Available network interfaces:"
ip link show | grep "^[0-9]" | awk -F': ' '{print "  - " $2}' || echo "  (Could not detect interfaces)"
echo ""
read -p "Enter network adapter name [enp0s3]: " NETWORK_ADAPTER
MMITSS_NETWORK_ADAPTER="${NETWORK_ADAPTER:-enp0s3}"
echo -e "${GREEN}✓ Network adapter set to: $MMITSS_NETWORK_ADAPTER${NC}"

# Step 3: Processor architecture
echo ""
echo -e "${YELLOW}Step 3: Setting Docker Processor Architecture${NC}"
echo "Common options:"
echo "  - amd64  (Intel/AMD 64-bit) - DEFAULT"
echo "  - arm64  (ARM 64-bit)"
echo "  - armhf  (ARM 32-bit)"
echo ""
read -p "Enter processor architecture [amd64]: " PROCESSOR
PROCESSOR="${PROCESSOR:-amd64}"
echo -e "${GREEN}✓ Processor set to: $PROCESSOR${NC}"

# Step 4: Export to shell configuration
echo ""
echo -e "${YELLOW}Step 4: Saving Configuration${NC}"

# Update .env files
echo "Updating .env files..."

for ENV_FILE in \
    "$MMITSS_ROOT/build/docker-compose/server/speedway-sample/.env" \
    "$MMITSS_ROOT/build/docker-compose/simulation/speedway-sample/.env"
do
    if [ -f "$ENV_FILE" ]; then
        # Use sed to update the values while preserving comments
        sed -i "s|^MMITSS_ROOT=.*|MMITSS_ROOT=$MMITSS_ROOT|" "$ENV_FILE"
        sed -i "s|^MMITSS_NETWORK_ADAPTER=.*|MMITSS_NETWORK_ADAPTER=$MMITSS_NETWORK_ADAPTER|" "$ENV_FILE"
        sed -i "s|^PROCESSOR=.*|PROCESSOR=$PROCESSOR|" "$ENV_FILE"
        echo "  ✓ Updated: $ENV_FILE"
    fi
done

# Export to current shell environment
export MMITSS_ROOT
export MMITSS_NETWORK_ADAPTER
export PROCESSOR

echo ""
echo -e "${GREEN}============================================================================${NC}"
echo -e "${GREEN}✓ Configuration Complete!${NC}"
echo -e "${GREEN}============================================================================${NC}"
echo ""
echo "Environment variables set:"
echo "  MMITSS_ROOT=$MMITSS_ROOT"
echo "  MMITSS_NETWORK_ADAPTER=$MMITSS_NETWORK_ADAPTER"
echo "  PROCESSOR=$PROCESSOR"
echo ""
echo -e "${YELLOW}Next Steps:${NC}"
echo "  1. Add these lines to your ~/.bashrc or ~/.zshrc to persist across sessions:"
echo ""
echo "     export MMITSS_ROOT=$MMITSS_ROOT"
echo "     export MMITSS_NETWORK_ADAPTER=$MMITSS_NETWORK_ADAPTER"
echo "     export PROCESSOR=$PROCESSOR"
echo ""
echo "  2. Then reload your shell:"
echo "     source ~/.bashrc  # or ~/.zshrc"
echo ""
echo "  3. Navigate to docker-compose directory:"
echo "     cd $MMITSS_ROOT/build/docker-compose/server/speedway-sample"
echo ""
echo "  4. Start services:"
echo "     docker-compose up -d"
echo ""
