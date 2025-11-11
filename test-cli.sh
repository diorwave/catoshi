#!/bin/bash
# Simple test wrapper for Catoshi/FoodChain
# Uses Bitcoin-core compatible test infrastructure
# See test/README.md for more information

set -e

# Color codes for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Catoshi Test Suite${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Check if binaries exist
if [ ! -f "./src/obj/foodchaind" ]; then
    echo -e "${RED}Error: foodchaind binary not found.${NC}"
    echo "Please build first: cd src && make"
    exit 1
fi

# Check/create test config if needed
if [ ! -f "./test/config.ini" ]; then
    echo -e "${YELLOW}Creating test/config.ini...${NC}"
    cat > ./test/config.ini <<EOF
# Copyright (c) 2013-2016 The Bitcoin Core developers
# Configuration for Catoshi/FoodChain functional tests

[environment]
SRCDIR=$(pwd)
BUILDDIR=$(pwd)
EXEEXT=

[components]
ENABLE_WALLET=true
ENABLE_UTILS=true
ENABLE_BITCOIND=true
ENABLE_ZMQ=true
EOF
    echo -e "${GREEN}✓ Created test/config.ini${NC}\n"
fi

echo -e "${YELLOW}Running Bitcoin-core compatible functional tests...${NC}\n"
echo "For more options, see: test/functional/test_runner.py --help"
echo "Or read: test/README.md"
echo ""

# Run the Bitcoin-core test runner
# Pass through any arguments provided to this script
if [ $# -eq 0 ]; then
    # No arguments - run default test suite
    exec test/functional/test_runner.py
else
    # Arguments provided - pass them to test runner
    exec test/functional/test_runner.py "$@"
fi

