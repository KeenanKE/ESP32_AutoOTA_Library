/**
 * ota_config.h
 * 
 * Configuration template for ESP32_AutoOTA library
 * Copy this file to your project's include/ folder and customize
 * 
 * This approach allows each project to have its own OTA settings
 * without modifying the library code
 */

#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// ========== PROJECT-SPECIFIC CONFIGURATION ==========

// GitHub Repository URLs (REQUIRED - Update these!)
#define OTA_FIRMWARE_URL "https://raw.githubusercontent.com/YourUsername/YourRepo/main/releases/firmware.bin"
#define OTA_VERSION_URL "https://raw.githubusercontent.com/YourUsername/YourRepo/main/releases/version.txt"

// Current version (use FIRMWARE_VERSION from build flags, or hardcode)
#ifndef FIRMWARE_VERSION
    #define FIRMWARE_VERSION "1.0.0"
#endif

// ========== UPDATE BEHAVIOR ==========

// Check interval (milliseconds)
#define OTA_CHECK_INTERVAL 300000       // 5 minutes (300,000 ms)

// Random initial delay range (milliseconds)
#define OTA_MIN_RANDOM_DELAY 60000      // 60 seconds
#define OTA_MAX_RANDOM_DELAY 180000     // 180 seconds (3 minutes)

// Staggered rollout settings
#define OTA_STAGGERED_ROLLOUT true      // Enable gradual rollout
#define OTA_ROLLOUT_PERCENTAGE 50       // 50% of devices update immediately

// Retry settings
#define OTA_MAX_RETRIES 3               // Number of retry attempts

// ========== HARDWARE CONFIGURATION ==========

// Status LED pin (-1 to disable)
#define OTA_STATUS_LED 2                // GPIO 2 (built-in LED on most ESP32 boards)

// ========== DEBUG SETTINGS ==========

// Enable debug output
#define OTA_DEBUG_MODE true             // Set to false for production

#endif // OTA_CONFIG_H
