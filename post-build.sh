#!/bin/sh

set -u
set -e

# Add SSH public key for root user to enable key-based authentication
mkdir -p "${TARGET_DIR}/root/.ssh"
chmod 700 "${TARGET_DIR}/root/.ssh"

# Add your SSH public key
cat >> "${TARGET_DIR}/root/.ssh/authorized_keys" << 'EOF'
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIAzbpqmTXzWcBpCoffFLGE/GIJJmVz7m3NWdWyeEeGRn ilian@ubuntu-git
EOF

chmod 600 "${TARGET_DIR}/root/.ssh/authorized_keys"

echo "SSH public key added to root's authorized_keys"
