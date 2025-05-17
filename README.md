# zenbook-duo-keyboard-service

## Project Introduction

This is a keyboard backlight service designed for ASUS Zenbook Duo laptops. This service can monitor keyboard connection status, control keyboard backlight brightness levels, and automatically restore previous backlight settings when the keyboard reconnects.

## Features

- Automatically detect keyboard connection/disconnection status
- Support for 4 levels of backlight adjustment (levels 0-3)
- Support for cycling through backlight levels
- Automatically save the last set backlight level
- Automatically restore backlight settings when the keyboard reconnects
- Support for custom monitor configuration (for Hyprland window manager)

## System Requirements

- Arch Linux or Arch-based distributions
- libusb library
- systemd

## Installation

### Installing via AUR

```bash
# Using yay
yay -S zenbook-duo-keyboard-service

# Or using paru
paru -S zenbook-duo-keyboard-service
```

### Building and Installing from Source

1. Download the source package
   ```bash
   git clone https://github.com/ParticleG/zenbook-duo-keyboard-service.git
   cd zenbook-duo-keyboard-service
   ```

2. Build and install
   ```bash
   makepkg -si
   ```

## Usage

### Starting the Service

```bash
sudo systemctl enable --now zenbook-duo-keyboard.service
```

### Controlling the Keyboard Backlight

```bash
keyboard-control [0|1|2|3|cycle]
```

Parameter description:
- `0` - Turn off backlight
- `1` - Low brightness
- `2` - Medium brightness
- `3` - High brightness
- `cycle` - Cycle through all brightness levels

## Configuration File

The configuration file is located at `/etc/keyboard-service/hypr_monitor.conf`, used to control Hyprland's monitor configuration when the keyboard is connected/disconnected.

Default configuration:
- When keyboard is connected: Disable eDP-2 monitor
- When keyboard is disconnected: Enable eDP-2 monitor (high resolution, 1.5 scaling)

## How It Works

1. The service monitors USB device events, detecting the specific Zenbook Duo keyboard (VendorID: 0x0b05, ProductID: 0x1b2c)
2. Sends control commands through the libusb library to adjust the keyboard backlight
3. Backlight settings are saved in the `/var/lib/keyboard-service/backlight-level` file
4. Uses a named pipe `/run/keyboard-service/fifo` to receive commands

## Uninstallation

```bash
sudo pacman -R zenbook-duo-keyboard-service
```

After uninstallation, configuration files and state files are not automatically deleted. If you need to completely remove them, please manually delete:
```bash
sudo rm -rf /etc/keyboard-service /var/lib/keyboard-service
```

## License

This project is licensed under the GPL License. See the LICENSE file for details.

## Author

ParticleG <particle_g@outlook.com>