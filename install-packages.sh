#!/bin/bash

set -euo pipefail

# Don't ask for manual feedback
export DEBIAN_FRONTEND=noninteractive

# Update
apt-get update
apt-get -y upgrade

# Install dependencies
apt-get -y install --no-install-recommends \
    build-essential \
    clang \
    libtbb-dev \
    libboost-program-options-dev \
    cmake

# Cleanup
apt-get clean
rm -rf /var/lib/apt/lists/*