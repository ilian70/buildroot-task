#!/bin/sh

set -u
set -e

# Add SSH public key for root user to enable key-based authentication
mkdir -p "${TARGET_DIR}/root/.ssh"
chmod 700 "${TARGET_DIR}/root/.ssh"

# Add your SSH public key
cat >> "${TARGET_DIR}/root/.ssh/authorized_keys" << 'EOF'
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIAzbpqmTXzWcBpCoffFLGE/GIJJmVz7m3NWdWyeEeGRn
EOF

chmod 600 "${TARGET_DIR}/root/.ssh/authorized_keys"

echo "SSH public key added to root's authorized_keys"

# Create graphics.target for systemd to ensure graphics services are properly ordered
mkdir -p "${TARGET_DIR}/etc/systemd/system"
cat > "${TARGET_DIR}/etc/systemd/system/graphics.target" << 'EOF'
[Unit]
Description=Graphics Target
Documentation=man:systemd.target(5)
After=multi-user.target
EOF

echo "graphics.target created for SDL/graphics service ordering"

# Configure auto-login on console
# Modify getty@tty1.service to auto-login as root
#rm -r "${TARGET_DIR}/etc/systemd/system/getty@tty1.service.d"
#mkdir -p "${TARGET_DIR}/etc/systemd/system/getty@tty1.service.d"
#cat > "${TARGET_DIR}/etc/systemd/system/getty@tty1.service.d/autologin.conf" << 'EOF'
#[Service]
#ExecStart=
#ExecStart=-/sbin/agetty --autologin root --noclear %I $TERM
#EOF
#echo "Auto-login configured for tty1"

# Optional: Also configure serial console auto-login if needed
#rm -r "${TARGET_DIR}/etc/systemd/system/serial-getty@.service.d"
#mkdir -p "${TARGET_DIR}/etc/systemd/system/serial-getty@.service.d"
#cat > "${TARGET_DIR}/etc/systemd/system/serial-getty@.service.d/autologin.conf" << 'EOF'
#[Service]
#ExecStart=
#ExecStart=-/sbin/agetty --autologin root --noclear %I $TERM
#EOF
#echo "Auto-login configured for serial console"
