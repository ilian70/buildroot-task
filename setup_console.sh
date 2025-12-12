#!/bin/bash

# Redis Image Viewer Console Setup Script

echo "Setting up Redis Image Viewer Remote Console..."

# Check if Python3 is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is required but not installed"
    exit 1
fi

# Check if pip is installed
if ! command -v pip3 &> /dev/null; then
    echo "Installing pip..."
    sudo apt update
    sudo apt install -y python3-pip
fi

# Install required Python packages
echo "Installing required packages..."
pip3 install --user redis

# Make the console script executable
chmod +x redis_console.py

echo ""
echo "✓ Setup complete!"
echo ""
echo "Usage:"
echo "  ./redis_console.py --host 192.168.0.200 --port 6379"
echo "  ./redis_console.py --host 192.168.1.200 --port 6379"
echo ""
echo "Available commands in console:"
echo "  help        - Show available commands"
echo "  status      - Get application status"
echo "  set_image 3 - Set current image to img3.png"
echo "  ping        - Test Redis connection"
echo "  stats       - Show Redis statistics"
echo "  exit        - Exit console"