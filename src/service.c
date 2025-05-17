#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <libusb-1.0/libusb.h>

// USB device properties
#define VENDOR_ID   0x0b05
#define PRODUCT_ID  0x1b2c
#define REPORT_ID   0x5A
#define WVALUE      0x035A
#define WINDEX      4
#define WLENGTH     16

// File path definitions
#define STATE_FILE  "/var/lib/keyboard-service/backlight-level"
#define FIFO_PATH   "/run/keyboard-service/fifo"
#define CONFIG_PATH "/etc/keyboard-service/hypr_monitor.conf"

// inotify constants
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Global variables
volatile sig_atomic_t running = 1;
libusb_context *usb_ctx = NULL;
bool device_connected = false;
const char *config_path = NULL;

// Function declarations
void add_watch_for_subdirs(int inotify_fd, const char *base_path);
bool check_usb_device(const char *device_id);
void update_monitor_config(const char *config_path, bool device_connected);
int read_backlight_level();
void save_backlight_level(int level);
int set_keyboard_backlight(int level);
void signal_handler(int sig);
void create_fifo();
void create_directories();

// Functions from keyboard_backlight.c
int read_backlight_level() {
    FILE *file = fopen(STATE_FILE, "r");
    if (!file) {
        return 0;
    }
    
    int level;
    fscanf(file, "%d", &level);
    fclose(file);
    
    if (level < 0 || level > 3) {
        level = 0;
    }
    
    return level;
}

void save_backlight_level(int level) {
    FILE *file = fopen(STATE_FILE, "w");
    if (!file) {
        fprintf(stderr, "Failed to create state file\n");
        return;
    }
    
    fprintf(file, "%d", level);
    fclose(file);
}

// Functions from watch_keyboard.c
void add_watch_for_subdirs(int inotify_fd, const char *base_path) {
    DIR *dir = opendir(base_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && 
            strcmp(entry->d_name, ".") != 0 && 
            strcmp(entry->d_name, "..") != 0) {
            char subdir_path[1024];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", base_path, entry->d_name);
            if (inotify_add_watch(inotify_fd, subdir_path, IN_ATTRIB) == -1) {
                perror("inotify_add_watch");
            }
        }
    }
    closedir(dir);
}

bool check_usb_device(const char *device_id) {
    FILE *lsusb_output = popen("lsusb", "r");
    if (!lsusb_output) {
        perror("popen");
        return false;
    }

    char line[256];
    bool device_found = false;
    while (fgets(line, sizeof(line), lsusb_output)) {
        if (strstr(line, device_id)) {
            device_found = true;
            break;
        }
    }

    pclose(lsusb_output);
    return device_found;
}

void update_monitor_config(const char *config_path, bool device_connected) {
    if (!config_path) return;
    
    FILE *config_file = fopen(config_path, "w");
    if (!config_file) {
        perror("fopen");
        return;
    }

    if (device_connected) {
        fprintf(config_file, "monitor = eDP-2, disable\n# monitor = eDP-2, highres , 0x1200, 1.5\n");
    } else {
        fprintf(config_file, "# monitor = eDP-2, disable\nmonitor = eDP-2, highres , 0x1200, 1.5\n");
    }

    fclose(config_file);
}

// Merged backlight control functionality from keyboard_backlight.c
int set_keyboard_backlight(int level) {
    if (level < 0 || level > 3) {
        fprintf(stderr, "Invalid level. Must be an integer between 0 and 3.\n");
        return EXIT_FAILURE;
    }

    // Save current level
    save_backlight_level(level);
    
    // If device is offline, only save level, don't try to set it
    if (!device_connected) {
        return EXIT_SUCCESS;
    }

    uint8_t data[WLENGTH] = {0};
    data[0] = REPORT_ID;
    data[1] = 0xBA;
    data[2] = 0xC5;
    data[3] = 0xC4;
    data[4] = (uint8_t)level;

    libusb_device_handle *dev_handle = NULL;
    int ret;

    // Open device
    dev_handle = libusb_open_device_with_vid_pid(usb_ctx, VENDOR_ID, PRODUCT_ID);
    if (!dev_handle) {
        fprintf(stderr, "Device not found (Vendor ID: 0x%04X, Product ID: 0x%04X)\n", VENDOR_ID, PRODUCT_ID);
        return EXIT_FAILURE;
    }

    // Detach kernel driver (if needed)
    if (libusb_kernel_driver_active(dev_handle, WINDEX)) {
        ret = libusb_detach_kernel_driver(dev_handle, WINDEX);
        if (ret < 0) {
            fprintf(stderr, "Could not detach kernel driver: %s\n", libusb_error_name(ret));
            libusb_close(dev_handle);
            return EXIT_FAILURE;
        }
    }

    // Send control transfer
    ret = libusb_control_transfer(dev_handle,
                                  0x21, // bmRequestType: Host to Device | Class | Interface
                                  0x09, // bRequest: SET_REPORT
                                  WVALUE, // wValue
                                  WINDEX, // wIndex
                                  data, WLENGTH, 1000); // data, length, timeout
    
    if (ret < 0) {
        fprintf(stderr, "Control transfer failed: %s\n", libusb_error_name(ret));
    } else if (ret != WLENGTH) {
        fprintf(stderr, "Warning: Only %d bytes sent out of %d.\n", ret, WLENGTH);
    } else {
        printf("Backlight level set to: %d\n", level);
    }

    // Reattach kernel driver (if needed)
    libusb_attach_kernel_driver(dev_handle, WINDEX);

    // Cleanup
    libusb_close(dev_handle);
    
    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

// Signal handler function
void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        running = 0;
    }
}

// Setup signal handling
void handle_signals() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

// Create necessary directories for state and FIFO files
void create_directories() {
    // Create directory for state file
    mkdir("/var/lib/keyboard-service", 0755);
    
    // Create directory for FIFO
    mkdir("/run/keyboard-service", 0755);
    
    // Ensure proper permissions
    chmod("/var/lib/keyboard-service", 0755);
    chmod("/run/keyboard-service", 0755);
}

// Create FIFO (named pipe) for receiving commands
void create_fifo() {
    // If FIFO already exists, delete it first
    unlink(FIFO_PATH);
    
    // Create directory if it doesn't exist
    create_directories();
    
    // Create new FIFO
    if (mkfifo(FIFO_PATH, 0666) < 0) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    
    // Ensure proper permissions
    chmod(FIFO_PATH, 0666);
}

int main(int argc, char *argv[]) {
    int opt;
    
    // Parse command line arguments
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                config_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-c config_path]\n", argv[0]);
                fprintf(stderr, "  -c config_path: specify the monitor config file path (default: %s)\n", CONFIG_PATH);
                exit(EXIT_FAILURE);
        }
    }
    
    // Set default config path if not specified
    if (!config_path) {
        config_path = CONFIG_PATH;
    }
    
    // Create necessary directories
    create_directories();
    
    // Initialize libusb
    int ret = libusb_init(&usb_ctx);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize libusb: %s\n", libusb_error_name(ret));
        return EXIT_FAILURE;
    }
    
    // Setup signal handling
    handle_signals();
    
    // Create named pipe for receiving commands
    create_fifo();
    
    // Setup inotify monitoring
    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        libusb_exit(usb_ctx);
        return EXIT_FAILURE;
    }
    
    // Monitor all subdirectories under /dev/bus/usb
    add_watch_for_subdirs(inotify_fd, "/dev/bus/usb");
    
    // Check initial device status
    device_connected = check_usb_device("0b05:1b2c");
    
    // If config path is provided, update config
    if (config_path) {
        update_monitor_config(config_path, device_connected);
    }
    
    // If device is connected, apply saved backlight level
    if (device_connected) {
        printf("ASUS Zenbook Duo Keyboard connected.\n");
        int level = read_backlight_level();
        set_keyboard_backlight(level);
    } else {
        printf("ASUS Zenbook Duo Keyboard disconnected.\n");
    }
    
    // Open FIFO for reading commands
    int fifo_fd = open(FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("open fifo");
        close(inotify_fd);
        libusb_exit(usb_ctx);
        return EXIT_FAILURE;
    }
    
    // Main loop
    char buffer[EVENT_BUF_LEN];
    char cmd_buffer[128];
    
    while (running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(inotify_fd, &read_fds);
        FD_SET(fifo_fd, &read_fds);
        
        int max_fd = (inotify_fd > fifo_fd) ? inotify_fd : fifo_fd;
        
        struct timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout, allowing the loop to periodically check running variable
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }
        
        // Check inotify events
        if (FD_ISSET(inotify_fd, &read_fds)) {
            int length = read(inotify_fd, buffer, EVENT_BUF_LEN);
            if (length < 0) {
                perror("read inotify");
                break;
            }
            
            int i = 0;
            while (i < length) {
                struct inotify_event *event = (struct inotify_event *)&buffer[i];
                if (event->mask & IN_ATTRIB) {
                    bool current_status = check_usb_device("0b05:1b2c");
                    if (current_status != device_connected) {
                        device_connected = current_status;
                        
                        if (config_path) {
                            update_monitor_config(config_path, device_connected);
                        }
                        
                        if (device_connected) {
                            printf("ASUS Zenbook Duo Keyboard connected.\n");
                            // When keyboard connects, apply saved backlight level
                            int level = read_backlight_level();
                            set_keyboard_backlight(level);
                        } else {
                            printf("ASUS Zenbook Duo Keyboard disconnected.\n");
                        }
                    }
                }
                i += EVENT_SIZE + event->len;
            }
        }
        
        // Check FIFO commands
        if (FD_ISSET(fifo_fd, &read_fds)) {
            memset(cmd_buffer, 0, sizeof(cmd_buffer));
            int bytes_read = read(fifo_fd, cmd_buffer, sizeof(cmd_buffer) - 1);
            
            if (bytes_read > 0) {
                cmd_buffer[bytes_read] = '\0';
                
                // Remove trailing newline
                if (cmd_buffer[bytes_read-1] == '\n')
                    cmd_buffer[bytes_read-1] = '\0';
                
                // Parse command
                if (strncmp(cmd_buffer, "set ", 4) == 0) {
                    int level = atoi(&cmd_buffer[4]);
                    set_keyboard_backlight(level);
                } else if (strncmp(cmd_buffer, "cycle", 5) == 0) {
                    int level = read_backlight_level();
                    level = (level + 1) % 4;
                    set_keyboard_backlight(level);
                }
            }
        }
    }
    
    // Close and clean up resources
    close(fifo_fd);
    close(inotify_fd);
    unlink(FIFO_PATH);
    libusb_exit(usb_ctx);
    
    return EXIT_SUCCESS;
}
