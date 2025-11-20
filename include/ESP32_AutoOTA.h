/**
 * ESP32_AutoOTA.h
 * 
 * Automatic Over-The-Air (OTA) Update Library for ESP32
 * Designed for mass deployment with randomized update checking
 * 
 * Features:
 * - Randomized check intervals to prevent server overload
 * - Random initial delay (60-180 seconds default)
 * - Staggered rollout based on device MAC address
 * - Memory-safe FreeRTOS task implementation
 * - GitHub CDN cache-busting headers
 * - Callback support for custom handling
 * - Automatic retry on failure
 * 
 * Author: KeenanKE
 * Date: November 2025
 * License: MIT
 */

#ifndef ESP32_AUTOOTA_H
#define ESP32_AUTOOTA_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// Default configuration values
#define DEFAULT_MIN_RANDOM_DELAY 60000       // 60 seconds
#define DEFAULT_MAX_RANDOM_DELAY 180000      // 180 seconds
#define DEFAULT_CHECK_INTERVAL 300000        // 5 minutes
#define DEFAULT_RETRY_DELAY 60000            // 1 minute
#define DEFAULT_MAX_RETRIES 3                // 3 retry attempts
#define DEFAULT_STACK_SIZE 8192              // 8KB stack for OTA task
#define DEFAULT_TASK_PRIORITY 1              // Low priority

// Callback function types
typedef void (*OTACallback)();
typedef void (*OTAProgressCallback)(size_t current, size_t total);
typedef void (*OTAErrorCallback)(const char* error);

class ESP32_AutoOTA {
public:
    /**
     * Constructor
     */
    ESP32_AutoOTA();

    /**
     * Destructor
     */
    ~ESP32_AutoOTA();

    // ========== Configuration Methods ==========
    
    /**
     * Set the URL for firmware binary
     * @param url Full URL to firmware.bin file
     */
    void setFirmwareURL(const char* url);

    /**
     * Set the URL for version file
     * @param url Full URL to version.txt file
     */
    void setVersionURL(const char* url);

    /**
     * Set current firmware version
     * @param version Version string (e.g., "1.0.3")
     */
    void setCurrentVersion(const char* version);

    /**
     * Set check interval (how often to check for updates)
     * @param intervalMs Interval in milliseconds (default: 5 minutes)
     */
    void setCheckInterval(unsigned long intervalMs);

    /**
     * Set random delay range for initial check
     * @param minMs Minimum delay in milliseconds (default: 60 seconds)
     * @param maxMs Maximum delay in milliseconds (default: 180 seconds)
     */
    void setRandomDelay(unsigned long minMs, unsigned long maxMs);

    /**
     * Enable/disable staggered rollout
     * When enabled, only a percentage of devices update immediately
     * based on device MAC address hash
     * @param enable True to enable, false to disable
     * @param percentage Percentage of devices to update immediately (0-100)
     */
    void setStaggeredRollout(bool enable, uint8_t percentage = 50);

    /**
     * Set LED pin for status indication
     * @param pin GPIO pin number (-1 to disable)
     */
    void setStatusLED(int pin);

    /**
     * Set maximum retry attempts on failure
     * @param retries Number of retries (default: 3)
     */
    void setMaxRetries(uint8_t retries);

    /**
     * Enable/disable debug output
     * @param enable True to enable, false to disable
     */
    void setDebugMode(bool enable);

    // ========== Callback Registration ==========
    
    /**
     * Register callback for when update starts
     */
    void onUpdateStart(OTACallback callback);

    /**
     * Register callback for update progress
     */
    void onUpdateProgress(OTAProgressCallback callback);

    /**
     * Register callback for when update completes
     */
    void onUpdateComplete(OTACallback callback);

    /**
     * Register callback for errors
     */
    void onUpdateError(OTAErrorCallback callback);

    /**
     * Register callback for version check
     */
    void onVersionCheck(OTACallback callback);

    // ========== Control Methods ==========
    
    /**
     * Start the OTA update task
     * Call this after WiFi is connected and all configuration is set
     * @return true if task started successfully, false otherwise
     */
    bool begin();

    /**
     * Stop the OTA update task
     */
    void stop();

    /**
     * Check if OTA task is running
     * @return true if running, false otherwise
     */
    bool isRunning();

    /**
     * Force an immediate update check (bypasses random delay)
     */
    void forceCheck();

    /**
     * Get current version string
     * @return Current version
     */
    const char* getCurrentVersion();

    /**
     * Get last check time
     * @return Milliseconds since last check
     */
    unsigned long getLastCheckTime();

    /**
     * Get last error message
     * @return Error message string
     */
    const char* getLastError();

private:
    // Configuration
    char _firmwareURL[256];
    char _versionURL[256];
    char _currentVersion[32];
    unsigned long _checkInterval;
    unsigned long _minRandomDelay;
    unsigned long _maxRandomDelay;
    bool _staggeredRollout;
    uint8_t _rolloutPercentage;
    int _statusLED;
    uint8_t _maxRetries;
    bool _debugMode;

    // State
    bool _isRunning;
    TaskHandle_t _taskHandle;
    unsigned long _lastCheckTime;
    uint8_t _retryCount;
    char _lastError[128];
    bool _forceCheckFlag;

    // Callbacks
    OTACallback _onUpdateStart;
    OTAProgressCallback _onUpdateProgress;
    OTACallback _onUpdateComplete;
    OTAErrorCallback _onUpdateError;
    OTACallback _onVersionCheck;

    // Internal methods
    static void taskWrapper(void* parameter);
    void otaTask();
    bool checkForUpdate();
    bool performUpdate();
    unsigned long getRandomDelay();
    bool shouldUpdateNow();
    uint32_t getDeviceHash();
    void setError(const char* error);
    void blinkLED(int times, int delayMs = 200);
    void log(const char* message);
    void logf(const char* format, ...);
};

#endif // ESP32_AUTOOTA_H
