#!/usr/bin/env python3
"""
Redis Remote Console for Image Viewer Application
Connects to Redis server and allows remote control of the image viewer
"""

import redis
import json
import time
import sys
import threading
from datetime import datetime

class RedisConsole:
    def __init__(self, redis_host='192.168.0.100', redis_port=6379):
        self.redis_host = redis_host
        self.redis_port = redis_port
        self.redis_client = None
        self.running = False
        self.commands = {
            'help': self.show_help,
            'status': self.get_status,
            'set_image': self.set_image,
            'get_image': self.get_current_image,
            'list_images': self.list_available_images,
            'config': self.show_config,
            'ping': self.ping_redis,
            'stats': self.get_redis_stats,
            'clear': self.clear_screen,
            'exit': self.exit_console,
            'quit': self.exit_console
        }

    def connect(self):
        """Connect to Redis server"""
        try:
            self.redis_client = redis.Redis(
                host=self.redis_host, 
                port=self.redis_port, 
                decode_responses=True,
                socket_connect_timeout=5
            )
            # Test connection
            self.redis_client.ping()
            print(f"✓ Connected to Redis server at {self.redis_host}:{self.redis_port}")
            return True
        except Exception as e:
            print(f"✗ Failed to connect to Redis: {e}")
            return False

    def show_help(self, args=None):
        """Show available commands"""
        help_text = """
╔══════════════════════════════════════════════════════════════╗
║                    Redis Image Viewer Console                ║
╠══════════════════════════════════════════════════════════════╣
║  help                - Show this help message               ║
║  status              - Get application status               ║
║  set_image <id>      - Set current image by ID (0-5)       ║
║  get_image           - Get current image ID                 ║
║  list_images         - List available images               ║
║  config              - Show application configuration       ║
║  ping                - Ping Redis server                   ║
║  stats               - Show Redis server statistics        ║
║  clear               - Clear console screen                ║
║  exit/quit           - Exit console                        ║
╚══════════════════════════════════════════════════════════════╝
        """
        print(help_text)

    def get_status(self, args=None):
        """Get application status from Redis"""
        try:
            # Check if application is running by looking for heartbeat
            heartbeat = self.redis_client.get("App:Heartbeat")
            if heartbeat:
                print(f"✓ Application Status: RUNNING (Last heartbeat: {heartbeat})")
            else:
                print("⚠ Application Status: UNKNOWN (No heartbeat found)")
            
            # Get current image
            current_image = self.redis_client.get("Image:Id")
            if current_image:
                print(f"Current Image ID: {current_image}")
            else:
                print("Current Image ID: Not set")
                
        except Exception as e:
            print(f"✗ Error getting status: {e}")

    def set_image(self, args):
        """Set current image ID"""
        if not args or len(args) == 0:
            print("Usage: set_image <id>")
            print("Available IDs: 0, 1, 2, 3, 4, 5")
            return
            
        try:
            image_id = int(args[0])
            if 0 <= image_id <= 5:
                self.redis_client.set("Image:Id", str(image_id))
                print(f"✓ Set current image to: img{image_id}.png")
                
                # Set command for application to refresh
                self.redis_client.set("App:Command", "refresh")
                print("✓ Sent refresh command to application")
            else:
                print("✗ Invalid image ID. Use 0-5")
        except ValueError:
            print("✗ Invalid image ID. Please enter a number 0-5")
        except Exception as e:
            print(f"✗ Error setting image: {e}")

    def get_current_image(self, args=None):
        """Get current image ID"""
        try:
            current_image = self.redis_client.get("Image:Id")
            if current_image:
                print(f"Current Image: img{current_image}.png (ID: {current_image})")
            else:
                print("No image currently set")
        except Exception as e:
            print(f"✗ Error getting current image: {e}")

    def list_available_images(self, args=None):
        """List available images"""
        images = [
            "img0.png - Sample Image 0",
            "img1.png - Sample Image 1", 
            "img2.png - Sample Image 2",
            "img3.png - Sample Image 3",
            "img4.png - Sample Image 4",
            "img5.png - Sample Image 5"
        ]
        
        print("\n📁 Available Images:")
        for i, img in enumerate(images):
            current = self.redis_client.get("Image:Id")
            marker = " ← CURRENT" if current == str(i) else ""
            print(f"  {i}: {img}{marker}")

    def show_config(self, args=None):
        """Show application configuration"""
        try:
            config_keys = [
                "Config:RedisHost", "Config:RedisPort", 
                "Config:ImageFolder", "Config:RefreshInterval",
                "Config:ScreenWidth", "Config:ScreenHeight"
            ]
            
            print("\n⚙️  Application Configuration:")
            for key in config_keys:
                value = self.redis_client.get(key)
                if value:
                    print(f"  {key.split(':')[1]}: {value}")
                    
        except Exception as e:
            print(f"✗ Error getting configuration: {e}")

    def ping_redis(self, args=None):
        """Ping Redis server"""
        try:
            start_time = time.time()
            response = self.redis_client.ping()
            end_time = time.time()
            latency = (end_time - start_time) * 1000
            
            if response:
                print(f"✓ PONG - Redis server is responding (latency: {latency:.2f}ms)")
            else:
                print("✗ No response from Redis server")
        except Exception as e:
            print(f"✗ Redis ping failed: {e}")

    def get_redis_stats(self, args=None):
        """Get Redis server statistics"""
        try:
            info = self.redis_client.info()
            
            print(f"\n📊 Redis Server Statistics:")
            print(f"  Version: {info.get('redis_version', 'Unknown')}")
            print(f"  Uptime: {info.get('uptime_in_seconds', 0)} seconds")
            print(f"  Connected clients: {info.get('connected_clients', 0)}")
            print(f"  Used memory: {info.get('used_memory_human', 'Unknown')}")
            print(f"  Total keys: {self.redis_client.dbsize()}")
            
        except Exception as e:
            print(f"✗ Error getting Redis stats: {e}")

    def clear_screen(self, args=None):
        """Clear console screen"""
        import os
        os.system('clear' if os.name == 'posix' else 'cls')

    def exit_console(self, args=None):
        """Exit the console"""
        self.running = False
        print("Goodbye! 👋")

    def run(self):
        """Main console loop"""
        if not self.connect():
            return
            
        self.running = True
        print(f"\n🚀 Redis Image Viewer Console - Connected to {self.redis_host}:{self.redis_port}")
        print("Type 'help' for available commands")
        
        while self.running:
            try:
                command_line = input("\nredis-console> ").strip()
                if not command_line:
                    continue
                    
                parts = command_line.split()
                command = parts[0].lower()
                args = parts[1:] if len(parts) > 1 else []
                
                if command in self.commands:
                    self.commands[command](args)
                else:
                    print(f"Unknown command: {command}")
                    print("Type 'help' for available commands")
                    
            except KeyboardInterrupt:
                print("\n\nExiting...")
                self.running = False
            except EOFError:
                print("\n\nExiting...")
                self.running = False
            except Exception as e:
                print(f"✗ Error: {e}")

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description='Redis Image Viewer Remote Console')
    parser.add_argument('--host', default='192.168.0.100', 
                       help='Redis server host (default: 192.168.0.100)')
    parser.add_argument('--port', type=int, default=6379,
                       help='Redis server port (default: 6379)')
    
    args = parser.parse_args()
    
    console = RedisConsole(args.host, args.port)
    console.run()

if __name__ == "__main__":
    main()
