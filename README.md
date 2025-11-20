# ESP32_AutoOTA Library

**Automatic Over-The-Air (OTA) update library for ESP32 with intelligent randomized checking and staggered rollout support.**

Designed for mass deployment of ESP32 devices with automatic firmware updates from GitHub (or any HTTP server).

---

## 🌟 Features

- ✅ **Randomized Update Checking** - Prevents server overload with thousands of devices
- ✅ **Staggered Rollout** - Gradually roll out updates to percentage of devices based on MAC address hash
- ✅ **Memory-Safe FreeRTOS Task** - Runs in background without interfering with your application
- ✅ **GitHub CDN Cache-Busting** - Always gets the latest firmware
- ✅ **Automatic Retry** - Configurable retry attempts on failure
- ✅ **Callback Support** - Monitor update progress, errors, and completion
- ✅ **LED Status Indication** - Visual feedback during updates
- ✅ **Version Comparison** - Only downloads when new version available
- ✅ **Easy Integration** - Simple API, minimal configuration required

---

## 📦 Installation

### Method 1: Local Library (Recommended for Development)

1. Place the `ESP32_AutoOTA_Library` folder in your workspace
2. In your project's `platformio.ini`, add:

```ini
[env:esp32dev]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino

lib_deps =
    file://../ESP32_AutoOTA_Library  ; Relative path to library
```

### Method 2: Git Repository (Future)

```ini
lib_deps =
    https://github.com/YourUsername/ESP32_AutoOTA_Library.git
```

---

## 🚀 Quick Start

### 1. Basic Usage

```cpp
#include <ESP32_AutoOTA.h>

ESP32_AutoOTA ota;

void setup() {
    // Connect to WiFi first
    WiFi.begin("YourSSID", "YourPassword");
    while (WiFi.status() != WL_CONNECTED) delay(500);
    
    // Configure OTA
    ota.setFirmwareURL("https://raw.githubusercontent.com/user/repo/main/releases/firmware.bin");
    ota.setVersionURL("https://raw.githubusercontent.com/user/repo/main/releases/version.txt");
    ota.setCurrentVersion("1.0.0");
    
    // Start automatic updates
    ota.begin();
}

void loop() {
    // Your application code
    // OTA runs in background automatically
}
```

### 2. Advanced Configuration

```cpp
// Customize behavior
ota.setCheckInterval(300000);           // Check every 5 minutes
ota.setRandomDelay(60000, 180000);      // Random 1-3 minute initial delay
ota.setStaggeredRollout(true, 50);      // 50% of devices update immediately
ota.setStatusLED(2);                    // Use GPIO 2 for status indication
ota.setMaxRetries(3);                   // Retry 3 times on failure

// Register callbacks
ota.onUpdateStart([]() {
    Serial.println("Update starting...");
});

ota.onUpdateProgress([](size_t current, size_t total) {
    Serial.printf("Progress: %d%%\n", (current * 100) / total);
});

ota.onUpdateComplete([]() {
    Serial.println("Update complete!");
});

ota.onUpdateError([](const char* error) {
    Serial.printf("Error: %s\n", error);
});
```

---

## 📖 API Reference

### Configuration Methods

#### `setFirmwareURL(const char* url)`
Set the URL for firmware binary file.

```cpp
ota.setFirmwareURL("https://raw.githubusercontent.com/user/repo/main/releases/firmware.bin");
```

#### `setVersionURL(const char* url)`
Set the URL for version text file.

```cpp
ota.setVersionURL("https://raw.githubusercontent.com/user/repo/main/releases/version.txt");
```

#### `setCurrentVersion(const char* version)`
Set current firmware version.

```cpp
ota.setCurrentVersion("1.0.3");
// Or use build flag:
ota.setCurrentVersion(FIRMWARE_VERSION);
```

#### `setCheckInterval(unsigned long intervalMs)`
Set how often to check for updates (milliseconds).

```cpp
ota.setCheckInterval(300000);  // 5 minutes
ota.setCheckInterval(600000);  // 10 minutes
```

#### `setRandomDelay(unsigned long minMs, unsigned long maxMs)`
Set random delay range for initial check (prevents simultaneous checks).

```cpp
ota.setRandomDelay(60000, 180000);  // Random 1-3 minute delay
```

#### `setStaggeredRollout(bool enable, uint8_t percentage)`
Enable gradual rollout (devices update based on MAC address hash).

```cpp
ota.setStaggeredRollout(true, 50);  // 50% update immediately
ota.setStaggeredRollout(true, 10);  // 10% update immediately (slower rollout)
```

#### `setStatusLED(int pin)`
Set LED pin for status indication (-1 to disable).

```cpp
ota.setStatusLED(2);   // Use GPIO 2
ota.setStatusLED(-1);  // Disable LED
```

#### `setMaxRetries(uint8_t retries)`
Set maximum retry attempts on failure.

```cpp
ota.setMaxRetries(3);  // Retry 3 times
```

#### `setDebugMode(bool enable)`
Enable/disable debug output to Serial.

```cpp
ota.setDebugMode(true);   // Enable debug
ota.setDebugMode(false);  // Disable debug
```

### Control Methods

#### `begin()`
Start the OTA update task. Returns `true` on success.

```cpp
if (ota.begin()) {
    Serial.println("OTA started successfully");
} else {
    Serial.printf("OTA failed: %s\n", ota.getLastError());
}
```

#### `stop()`
Stop the OTA update task.

```cpp
ota.stop();
```

#### `isRunning()`
Check if OTA task is running.

```cpp
if (ota.isRunning()) {
    Serial.println("OTA is active");
}
```

#### `forceCheck()`
Force an immediate update check (bypasses random delay).

```cpp
ota.forceCheck();
```

### Callback Registration

#### `onUpdateStart(OTACallback callback)`
Called when update begins.

```cpp
ota.onUpdateStart([]() {
    Serial.println("Starting update...");
    // Pause application activities
});
```

#### `onUpdateProgress(OTAProgressCallback callback)`
Called during update with progress information.

```cpp
ota.onUpdateProgress([](size_t current, size_t total) {
    int percent = (current * 100) / total;
    Serial.printf("Progress: %d%%\n", percent);
});
```

#### `onUpdateComplete(OTACallback callback)`
Called when update completes successfully (before reboot).

```cpp
ota.onUpdateComplete([]() {
    Serial.println("Update complete! Rebooting...");
    // Save important data before reboot
});
```

#### `onUpdateError(OTAErrorCallback callback)`
Called on update error.

```cpp
ota.onUpdateError([](const char* error) {
    Serial.printf("Update failed: %s\n", error);
    // Handle error
});
```

#### `onVersionCheck(OTACallback callback)`
Called when version check starts.

```cpp
ota.onVersionCheck([]() {
    Serial.println("Checking for updates...");
});
```

---

## 🏗️ Project Structure

### Firmware Repository Structure

Your GitHub repository should have this structure:

```
YourProject/
├── releases/
│   ├── firmware.bin    ← Compiled binary
│   └── version.txt     ← Version number (e.g., "1.0.3")
├── src/
│   └── main.cpp
└── platformio.ini
```

### Version File Format

The `version.txt` file should contain only the version number:

```
1.0.3
```

**Important:** No extra whitespace, newlines, or characters!

---

## 💡 Usage Patterns

### Pattern 1: Same Firmware for All Devices

**Use Case:** All devices run identical firmware (e.g., smart light bulbs)

```cpp
// All devices check same URLs
ota.setFirmwareURL("https://raw.githubusercontent.com/user/SmartBulb/main/releases/firmware.bin");
ota.setVersionURL("https://raw.githubusercontent.com/user/SmartBulb/main/releases/version.txt");
```

### Pattern 2: Different Products

**Use Case:** Multiple product types with different firmware

**Project A (Blink Lights):**
```cpp
ota.setFirmwareURL("https://raw.githubusercontent.com/user/Project_BlinkLights/main/releases/firmware.bin");
ota.setVersionURL("https://raw.githubusercontent.com/user/Project_BlinkLights/main/releases/version.txt");
```

**Project B (Motor Control):**
```cpp
ota.setFirmwareURL("https://raw.githubusercontent.com/user/Project_MotorControl/main/releases/firmware.bin");
ota.setVersionURL("https://raw.githubusercontent.com/user/Project_MotorControl/main/releases/version.txt");
```

### Pattern 3: Using Build Flags

In `platformio.ini`:
```ini
build_flags = 
    -D FIRMWARE_VERSION=\"1.0.3\"
    -D OTA_FIRMWARE_URL=\"https://raw.githubusercontent.com/user/repo/main/releases/firmware.bin\"
    -D OTA_VERSION_URL=\"https://raw.githubusercontent.com/user/repo/main/releases/version.txt\"
```

In code:
```cpp
ota.setFirmwareURL(OTA_FIRMWARE_URL);
ota.setVersionURL(OTA_VERSION_URL);
ota.setCurrentVersion(FIRMWARE_VERSION);
```

---

## 🎯 Mass Deployment Best Practices

### 1. Randomized Checking

With thousands of devices, use randomized delays to prevent server overload:

```cpp
// Random initial delay: 1-5 minutes
ota.setRandomDelay(60000, 300000);

// Check interval: 10-15 minutes (±10% random variation built-in)
ota.setCheckInterval(600000);
```

### 2. Staggered Rollout

Roll out updates gradually to detect issues early:

```cpp
// Day 1: 10% of devices (test group)
ota.setStaggeredRollout(true, 10);

// Day 2: Increase to 50% if stable
ota.setStaggeredRollout(true, 50);

// Day 3: Full rollout
ota.setStaggeredRollout(false);  // or set to 100%
```

### 3. Version Strategy

Use semantic versioning:
- **Patch** (1.0.X): Bug fixes
- **Minor** (1.X.0): New features
- **Major** (X.0.0): Breaking changes

### 4. Testing Before Deployment

1. Test update on 1-2 devices first
2. Wait 24 hours to ensure stability
3. Gradually increase rollout percentage
4. Monitor for errors via callbacks

---

## 🔧 Troubleshooting

### Update Not Triggering

**Check:**
- WiFi is connected: `WiFi.status() == WL_CONNECTED`
- URLs are correct and accessible
- `version.txt` contains different version than current
- Initial random delay hasn't elapsed yet (60-180 seconds by default)

**Debug:**
```cpp
ota.setDebugMode(true);  // Enable verbose logging
Serial.printf("Current version: %s\n", ota.getCurrentVersion());
Serial.printf("Last check: %lu ms ago\n", millis() - ota.getLastCheckTime());
```

### Update Fails

**Check:**
- Firmware size fits in available OTA partition
- `firmware.bin` file is valid and not corrupted
- GitHub URLs use `raw.githubusercontent.com` (not `github.com`)
- Error callbacks for specific error messages

**Debug:**
```cpp
ota.onUpdateError([](const char* error) {
    Serial.printf("Error: %s\n", error);
});

Serial.printf("Last error: %s\n", ota.getLastError());
```

### Staggered Rollout Not Working

**Check:**
- Each device has unique MAC address (should be automatic)
- Percentage is set correctly (0-100)

**Test:**
```cpp
// Temporarily disable to test
ota.setStaggeredRollout(false);
```

---

## 📜 License

MIT License - See LICENSE file for details

---

## 🤝 Contributing

Contributions welcome! Please open an issue or pull request.

---

## 📞 Support

For issues, questions, or feature requests, please open an issue on GitHub.

---

**Made with ❤️ for IoT deployments**
