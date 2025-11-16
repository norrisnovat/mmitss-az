#!/bin/bash

################################################################################
# MMITSS Web Interfaces Launcher
################################################################################
# This script opens all MMITSS web interfaces in your default browser
# 
# Supervisord Interfaces:
#   - Campbell:  http://localhost:9001
#   - Cherry:    http://localhost:9002
#   - Mountain:  http://localhost:9003
#   - Park:      http://localhost:9004
#
# Usage:
#   ./launch-interfaces.sh
#   OR
#   bash launch-interfaces.sh
#
################################################################################

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
INTERFACES=(
    "Campbell:9001"
    "Cherry:9002"
    "Mountain:9003"
    "Park:9004"
)

HOST="localhost"

echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}MMITSS Web Interfaces Launcher${NC}"
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""

# Step 1: Verify Docker
echo -e "${YELLOW}Step 1: Verifying Docker...${NC}"
if ! command -v docker &> /dev/null; then
    echo -e "${RED}✗ Docker not found. Please install Docker.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Docker is installed${NC}"
echo ""

# Step 2: Check containers
echo -e "${YELLOW}Step 2: Checking containers...${NC}"
RUNNING_CONTAINERS=0
for interface in "${INTERFACES[@]}"; do
    NAME="${interface%%:*}"
    CONTAINER="speedway-${NAME,,}"
    
    if docker ps --filter "name=$CONTAINER" --format "{{.Names}}" | grep -q "$CONTAINER"; then
        echo -e "${GREEN}✓ $CONTAINER is running${NC}"
        ((RUNNING_CONTAINERS++))
    else
        echo -e "${RED}✗ $CONTAINER is not running${NC}"
    fi
done
echo ""

if [ $RUNNING_CONTAINERS -eq 0 ]; then
    echo -e "${RED}No containers running! Start them first:${NC}"
    echo ""
    echo -e "  ${YELLOW}export MMITSS_ROOT=\$HOME/path/to/mmitss-az${NC}"
    echo -e "  ${YELLOW}cd \$MMITSS_ROOT/build/docker-compose/server/speedway-sample${NC}"
    echo -e "  ${YELLOW}docker-compose up -d${NC}"
    echo ""
    exit 1
fi
echo ""

# Step 3: Check ports
echo -e "${YELLOW}Step 3: Checking ports...${NC}"
AVAILABLE_PORTS=0
for interface in "${INTERFACES[@]}"; do
    NAME="${interface%%:*}"
    PORT="${interface##*:}"
    
    if nc -z -w 1 $HOST $PORT 2>/dev/null; then
        echo -e "${GREEN}✓ Port $PORT is responding (${NAME})${NC}"
        ((AVAILABLE_PORTS++))
    else
        echo -e "${YELLOW}⚠ Port $PORT not responding yet (${NAME})${NC}"
        echo "  Supervisord may still be starting..."
    fi
done
echo ""

if [ $AVAILABLE_PORTS -eq 0 ]; then
    echo -e "${YELLOW}⚠ Warning: Ports not responding yet${NC}"
    echo "  Supervisord is likely still starting..."
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${RED}Cancelled${NC}"
        exit 0
    fi
fi
echo ""

# Step 4: Determine browser
echo -e "${YELLOW}Step 4: Opening interfaces...${NC}"

BROWSER=""
if command -v xdg-open &> /dev/null; then
    BROWSER="xdg-open"  # Linux
elif command -v open &> /dev/null; then
    BROWSER="open"      # macOS
elif command -v start &> /dev/null; then
    BROWSER="start"     # Windows (Git Bash)
else
    BROWSER=""
fi

# Step 5: Open interfaces
if [ -n "$BROWSER" ]; then
    for interface in "${INTERFACES[@]}"; do
        NAME="${interface%%:*}"
        PORT="${interface##*:}"
        URL="http://${HOST}:${PORT}"
        
        echo -e "  Opening ${BLUE}${NAME}${NC} → ${BLUE}${URL}${NC}"
        $BROWSER "$URL" &
        sleep 0.5  # Stagger opening to avoid overwhelming the browser
    done
    echo ""
    echo -e "${GREEN}✓ All interfaces opened in your browser!${NC}"
else
    echo -e "${YELLOW}Could not detect browser. Please open manually:${NC}"
    echo ""
    for interface in "${INTERFACES[@]}"; do
        NAME="${interface%%:*}"
        PORT="${interface##*:}"
        URL="http://${HOST}:${PORT}"
        echo -e "  ${BLUE}${NAME}:${NC} ${URL}"
    done
fi
echo ""

# Step 6: Show next steps
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}What to do next:${NC}"
echo ""
echo "1. Wait for Supervisord interfaces to fully load (2-5 seconds)"
echo ""
echo "2. Each interface shows:"
echo "   - Process status (RUNNING/STOPPED)"
echo "   - Resource usage (CPU, Memory)"
echo "   - Service health"
echo ""
echo "3. To control processes:"
echo "   - Click 'Start' to start a process"
echo "   - Click 'Stop' to stop a process"
echo "   - Click process name to view logs"
echo ""
echo "4. Verify all processes are RUNNING (should see 230+)"
echo ""
echo "5. Monitor logs for errors:"
echo -e "   ${YELLOW}docker-compose logs -f${NC}"
echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "   ${GREEN}Supervisord Interfaces:${NC}"
for interface in "${INTERFACES[@]}"; do
    NAME="${interface%%:*}"
    PORT="${interface##*:}"
    echo -e "   • ${BLUE}${NAME}${NC}: http://localhost:${PORT}"
done
echo ""
echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}"
