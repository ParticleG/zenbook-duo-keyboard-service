# zenbook-duo-keyboard-service

## 项目简介

这是一个为华硕Zenbook Duo笔记本电脑设计的键盘背光服务。本服务能够监控键盘连接状态，控制键盘背光亮度级别，并在键盘重新连接时自动恢复之前的背光设置。

## 功能特点

- 自动检测键盘连接/断开状态
- 支持4级背光调节（0-3级）
- 支持循环切换背光级别
- 自动保存上次设置的背光级别
- 在键盘重新连接时自动恢复背光设置
- 支持自定义显示器配置（适用于Hyprland窗口管理器）

## 系统要求

- Arch Linux 或基于Arch的发行版
- libusb库
- systemd

## 安装方法

### 通过AUR安装

```bash
# 使用yay
yay -S zenbook-duo-keyboard-service

# 或使用paru
paru -S zenbook-duo-keyboard-service
```

### 从源码编译安装

1. 下载源码包
   ```bash
   git clone https://github.com/ParticleG/zenbook-duo-keyboard-service.git
   cd zenbook-duo-keyboard-service
   ```

2. 构建并安装
   ```bash
   makepkg -si
   ```

## 使用方法

### 启动服务

```bash
sudo systemctl enable --now zenbook-duo-keyboard.service
```

### 控制键盘背光

```bash
keyboard-control [0|1|2|3|cycle]
```

参数说明：
- `0` - 关闭背光
- `1` - 低亮度
- `2` - 中亮度
- `3` - 高亮度
- `cycle` - 循环切换所有亮度级别

## 配置文件

配置文件位于 `/etc/keyboard-service/hypr_monitor.conf`，用于控制当键盘连接/断开时Hyprland的显示器配置。

默认配置：
- 当键盘连接时：禁用eDP-2显示器
- 当键盘断开时：启用eDP-2显示器（高分辨率，缩放1.5）

## 工作原理

1. 服务监控USB设备事件，检测特定的Zenbook Duo键盘（VendorID: 0x0b05, ProductID: 0x1b2c）
2. 通过libusb库发送控制命令来调整键盘背光
3. 背光设置保存在 `/var/lib/keyboard-service/backlight-level` 文件中
4. 使用命名管道 `/run/keyboard-service/fifo` 接收命令

## 卸载

```bash
sudo pacman -R zenbook-duo-keyboard-service
```

卸载后，配置文件和状态文件不会被自动删除。如果需要完全清除，请手动删除：
```bash
sudo rm -rf /etc/keyboard-service /var/lib/keyboard-service
```

## 许可证

本项目采用GPL许可证。详情请参阅LICENSE文件。

## 作者

ParticleG <particle_g@outlook.com>