/**
 * BasicAutoOTA.ino
 * 
 * Basic example of using ESP32_AutoOTA library
 * 
 * This example demonstrates:
 * - Simple setup with WiFiManager for credentials
 * - Automatic version checking with randomized intervals
 * - Custom callbacks for monitoring update progress
 * 
 * Hardware Required:
 * - ESP32 development board
 * - Built-in LED (or connect external LED to GPIO 2)
 * 
 * Configuration:
 * 1. Update the GitHub URLs below with your repository
 * 2. Upload this sketch
 * 3. Connect to WiFi using captive portal
 * 4. Device will automatically check for updates
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32_AutoOTA.h>

// ========== CONFIGURATION ==========
// Update these with your GitHub repository URLs
const char* FIRMWARE_URL = "https://raw.githubusercontent.com/YourUsername/YourRepo/main/releases/firmware.bin";
const char* VERSION_URL = "https://raw.githubusercontent.com/YourUsername/YourRepo/main/releases/version.txt";
const char* CURRENT_VERSION = "1.0.0"; // Update this in platformio.ini using build flags

// WiFi credentials (for testing - use WiFiManager in production)
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// GPIO pins
const int LED_PIN = 2;          // Built-in LED
const int STATUS_LED = LED_PIN; // Same LED for OTA status
// ===================================

// Create OTA instance
ESP32_AutoOTA ota;

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n[Setup] Starting BasicAutoOTA example...");
    
    // Initialize LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Connect to WiFi
    Serial.print("[WiFi] Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink while connecting
    }
    
    Serial.println("\n[WiFi] Connected!");
    Serial.print("[WiFi] IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, LOW);
    
    // ========== Configure AutoOTA ==========
    
    // Required: Set firmware URLs
    ota.setFirmwareURL(FIRMWARE_URL);
    ota.setVersionURL(VERSION_URL);
    ota.setCurrentVersion(CURRENT_VERSION);
    
    // Optional: Customize behavior
    ota.setCheckInterval(300000);           // Check every 5 minutes
    ota.setRandomDelay(60000, 180000);      // Random 1-3 minute initial delay
    ota.setStaggeredRollout(true, 50);      // 50% of devices update immediately
    ota.setStatusLED(STATUS_LED);           // Use LED for status indication
    ota.setMaxRetries(3);                   // Retry 3 times on failure
    ota.setDebugMode(true);                 // Enable debug output
    
    // Optional: Register callbacks
    ota.onUpdateStart([]() {
        Serial.println("[Callback] ⬇️  OTA update starting...");
    });
    
    ota.onUpdateProgress([](size_t current, size_t total) {
        static int lastPercent = -1;
        int percent = (current * 100) / total;
        
        // Only print every 10%
        if (percent != lastPercent && percent % 10 == 0) {
            Serial.printf("[Callback] 📊 Progress: %d%% (%d / %d bytes)\n", 
                          percent, current, total);
            lastPercent = percent;
        }
    });
    
    ota.onUpdateComplete([]() {
        Serial.println("[Callback] ✅ Update complete! Device will reboot...");
    });
    
    ota.onUpdateError([](const char* error) {
        Serial.printf("[Callback] ❌ Update error: %s\n", error);
    });
    
    ota.onVersionCheck([]() {
        Serial.println("[Callback] 🔍 Checking for new firmware version...");
    });
    
    // Start OTA task
    if (ota.begin()) {
        Serial.println("[Setup] ✅ AutoOTA started successfully!");
        Serial.printf("[Setup] Current version: %s\n", ota.getCurrentVersion());
        Serial.println("[Setup] Device will check for updates automatically");
        Serial.println("[Setup] Your application code continues below...\n");
    } else {
        Serial.println("[Setup] ❌ Failed to start AutoOTA!");
        Serial.printf("[Setup] Error: %s\n", ota.getLastError());
    }
}

void loop() {
    // ========== Your Application Code Here ==========
    
    // Example: Simple blink to show device is running
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    
    // The OTA library runs in the background on a separate task
    // Your application continues normally
    
    Serial.println("[App] Application running normally...");
    
    // Example: Force an update check (optional)
    // Uncomment to check immediately instead of waiting for random delay
    // ota.forceCheck();
}
