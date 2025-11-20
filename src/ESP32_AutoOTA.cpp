/**
 * ESP32_AutoOTA.cpp
 * 
 * Implementation of ESP32_AutoOTA library
 */

#include "ESP32_AutoOTA.h"
#include <esp_system.h>

// Constructor
ESP32_AutoOTA::ESP32_AutoOTA() {
    _firmwareURL[0] = '\0';
    _versionURL[0] = '\0';
    strcpy(_currentVersion, "0.0.0");
    _checkInterval = DEFAULT_CHECK_INTERVAL;
    _minRandomDelay = DEFAULT_MIN_RANDOM_DELAY;
    _maxRandomDelay = DEFAULT_MAX_RANDOM_DELAY;
    _staggeredRollout = false;
    _rolloutPercentage = 50;
    _statusLED = -1;
    _maxRetries = DEFAULT_MAX_RETRIES;
    _debugMode = true;
    _isRunning = false;
    _taskHandle = NULL;
    _lastCheckTime = 0;
    _retryCount = 0;
    _lastError[0] = '\0';
    _forceCheckFlag = false;
    _onUpdateStart = NULL;
    _onUpdateProgress = NULL;
    _onUpdateComplete = NULL;
    _onUpdateError = NULL;
    _onVersionCheck = NULL;
}

// Destructor
ESP32_AutoOTA::~ESP32_AutoOTA() {
    stop();
}

// ========== Configuration Methods ==========

void ESP32_AutoOTA::setFirmwareURL(const char* url) {
    strncpy(_firmwareURL, url, sizeof(_firmwareURL) - 1);
    _firmwareURL[sizeof(_firmwareURL) - 1] = '\0';
}

void ESP32_AutoOTA::setVersionURL(const char* url) {
    strncpy(_versionURL, url, sizeof(_versionURL) - 1);
    _versionURL[sizeof(_versionURL) - 1] = '\0';
}

void ESP32_AutoOTA::setCurrentVersion(const char* version) {
    strncpy(_currentVersion, version, sizeof(_currentVersion) - 1);
    _currentVersion[sizeof(_currentVersion) - 1] = '\0';
}

void ESP32_AutoOTA::setCheckInterval(unsigned long intervalMs) {
    _checkInterval = intervalMs;
}

void ESP32_AutoOTA::setRandomDelay(unsigned long minMs, unsigned long maxMs) {
    _minRandomDelay = minMs;
    _maxRandomDelay = maxMs;
}

void ESP32_AutoOTA::setStaggeredRollout(bool enable, uint8_t percentage) {
    _staggeredRollout = enable;
    _rolloutPercentage = constrain(percentage, 0, 100);
}

void ESP32_AutoOTA::setStatusLED(int pin) {
    _statusLED = pin;
    if (pin >= 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
}

void ESP32_AutoOTA::setMaxRetries(uint8_t retries) {
    _maxRetries = retries;
}

void ESP32_AutoOTA::setDebugMode(bool enable) {
    _debugMode = enable;
}

// ========== Callback Registration ==========

void ESP32_AutoOTA::onUpdateStart(OTACallback callback) {
    _onUpdateStart = callback;
}

void ESP32_AutoOTA::onUpdateProgress(OTAProgressCallback callback) {
    _onUpdateProgress = callback;
}

void ESP32_AutoOTA::onUpdateComplete(OTACallback callback) {
    _onUpdateComplete = callback;
}

void ESP32_AutoOTA::onUpdateError(OTAErrorCallback callback) {
    _onUpdateError = callback;
}

void ESP32_AutoOTA::onVersionCheck(OTACallback callback) {
    _onVersionCheck = callback;
}

// ========== Control Methods ==========

bool ESP32_AutoOTA::begin() {
    if (_isRunning) {
        log("[AutoOTA] Already running");
        return false;
    }

    if (strlen(_firmwareURL) == 0 || strlen(_versionURL) == 0) {
        setError("Firmware or version URL not set");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        setError("WiFi not connected");
        return false;
    }

    log("[AutoOTA] Starting OTA task...");
    
    BaseType_t result = xTaskCreate(
        taskWrapper,
        "AutoOTA_Task",
        DEFAULT_STACK_SIZE,
        this,
        DEFAULT_TASK_PRIORITY,
        &_taskHandle
    );

    if (result == pdPASS) {
        _isRunning = true;
        log("[AutoOTA] Task started successfully");
        return true;
    } else {
        setError("Failed to create task");
        return false;
    }
}

void ESP32_AutoOTA::stop() {
    if (_taskHandle != NULL) {
        vTaskDelete(_taskHandle);
        _taskHandle = NULL;
    }
    _isRunning = false;
    log("[AutoOTA] Task stopped");
}

bool ESP32_AutoOTA::isRunning() {
    return _isRunning;
}

void ESP32_AutoOTA::forceCheck() {
    _forceCheckFlag = true;
    log("[AutoOTA] Force check requested");
}

const char* ESP32_AutoOTA::getCurrentVersion() {
    return _currentVersion;
}

unsigned long ESP32_AutoOTA::getLastCheckTime() {
    return _lastCheckTime;
}

const char* ESP32_AutoOTA::getLastError() {
    return _lastError;
}

// ========== Private Methods ==========

void ESP32_AutoOTA::taskWrapper(void* parameter) {
    ESP32_AutoOTA* instance = static_cast<ESP32_AutoOTA*>(parameter);
    instance->otaTask();
}

void ESP32_AutoOTA::otaTask() {
    // Random initial delay (60-180 seconds by default)
    unsigned long initialDelay = getRandomDelay();
    logf("[AutoOTA] Waiting %lu seconds before first check...", initialDelay / 1000);
    vTaskDelay(initialDelay / portTICK_PERIOD_MS);

    log("[AutoOTA] Starting update monitoring");

    while (true) {
        // Check if WiFi is connected
        if (WiFi.status() != WL_CONNECTED) {
            log("[AutoOTA] WiFi disconnected, waiting...");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
            continue;
        }

        // Check for updates
        if (_forceCheckFlag || (millis() - _lastCheckTime >= _checkInterval)) {
            _forceCheckFlag = false;
            
            if (checkForUpdate()) {
                _retryCount = 0; // Reset retry count on success
            } else {
                _retryCount++;
                if (_retryCount >= _maxRetries) {
                    logf("[AutoOTA] Max retries reached (%d), resetting counter", _maxRetries);
                    _retryCount = 0;
                }
            }
            
            _lastCheckTime = millis();
        }

        // Randomize next check interval (±10% variation)
        unsigned long variation = _checkInterval / 10;
        unsigned long nextCheck = _checkInterval + random(-variation, variation);
        vTaskDelay(nextCheck / portTICK_PERIOD_MS);
    }
}

bool ESP32_AutoOTA::checkForUpdate() {
    log("[AutoOTA] Checking for firmware update...");
    
    if (_onVersionCheck) {
        _onVersionCheck();
    }

    HTTPClient http;
    http.begin(_versionURL);
    
    // Cache-busting headers
    http.addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Expires", "0");

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String remoteVersion = http.getString();
        remoteVersion.trim();
        
        logf("[AutoOTA] Current: %s, Remote: %s", _currentVersion, remoteVersion.c_str());

        if (remoteVersion.equals(_currentVersion)) {
            log("[AutoOTA] Firmware is up to date");
            http.end();
            return true;
        } else {
            log("[AutoOTA] New version available!");
            
            // Check staggered rollout
            if (_staggeredRollout && !shouldUpdateNow()) {
                logf("[AutoOTA] Staggered rollout: Delaying update (device not in %d%% group)", _rolloutPercentage);
                http.end();
                return true;
            }
            
            http.end();
            return performUpdate();
        }
    } else {
        logf("[AutoOTA] Version check failed: HTTP %d", httpCode);
        setError("Version check failed");
        http.end();
        return false;
    }
}

bool ESP32_AutoOTA::performUpdate() {
    log("[AutoOTA] Starting firmware download...");
    blinkLED(3, 100); // Quick blinks to indicate update starting
    
    if (_onUpdateStart) {
        _onUpdateStart();
    }

    HTTPClient http;
    http.begin(_firmwareURL);
    
    // Cache-busting headers
    http.addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    http.addHeader("Pragma", "no-cache");
    http.addHeader("Expires", "0");

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        int contentLength = http.getSize();
        
        if (contentLength > 0) {
            logf("[AutoOTA] Firmware size: %d bytes", contentLength);
            
            bool canBegin = Update.begin(contentLength);
            
            if (canBegin) {
                log("[AutoOTA] Writing firmware to flash...");
                
                WiFiClient& stream = http.getStream();
                size_t written = 0;
                size_t total = contentLength;
                uint8_t buffer[128];
                
                while (http.connected() && (written < total)) {
                    size_t available = stream.available();
                    if (available) {
                        size_t bytesToRead = min(available, sizeof(buffer));
                        size_t bytesRead = stream.readBytes(buffer, bytesToRead);
                        
                        size_t bytesWritten = Update.write(buffer, bytesRead);
                        written += bytesWritten;
                        
                        // Progress callback
                        if (_onUpdateProgress && (written % 10240 == 0 || written == total)) {
                            _onUpdateProgress(written, total);
                        }
                        
                        // Blink LED during update
                        if (_statusLED >= 0 && written % 4096 == 0) {
                            digitalWrite(_statusLED, !digitalRead(_statusLED));
                        }
                    }
                    delay(1);
                }
                
                if (_statusLED >= 0) {
                    digitalWrite(_statusLED, LOW);
                }
                
                logf("[AutoOTA] Wrote: %d bytes", written);
                
                if (Update.end()) {
                    if (Update.isFinished()) {
                        log("[AutoOTA] Update successful! Rebooting...");
                        
                        if (_onUpdateComplete) {
                            _onUpdateComplete();
                        }
                        
                        blinkLED(5, 200); // Success pattern
                        delay(1000);
                        ESP.restart();
                        return true;
                    } else {
                        setError("Update not finished");
                    }
                } else {
                    char errorMsg[64];
                    snprintf(errorMsg, sizeof(errorMsg), "Update failed: error %d", Update.getError());
                    setError(errorMsg);
                }
            } else {
                setError("Not enough space for OTA");
            }
        } else {
            setError("Content length is zero");
        }
    } else {
        char errorMsg[64];
        snprintf(errorMsg, sizeof(errorMsg), "Download failed: HTTP %d", httpCode);
        setError(errorMsg);
    }
    
    http.end();
    return false;
}

unsigned long ESP32_AutoOTA::getRandomDelay() {
    return random(_minRandomDelay, _maxRandomDelay);
}

bool ESP32_AutoOTA::shouldUpdateNow() {
    // Hash device MAC address to get consistent but distributed decision
    uint32_t deviceHash = getDeviceHash();
    uint8_t devicePercentile = deviceHash % 100;
    
    return (devicePercentile < _rolloutPercentage);
}

uint32_t ESP32_AutoOTA::getDeviceHash() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    // Simple hash of MAC address
    uint32_t hash = 0;
    for (int i = 0; i < 6; i++) {
        hash = hash * 31 + mac[i];
    }
    
    return hash;
}

void ESP32_AutoOTA::setError(const char* error) {
    strncpy(_lastError, error, sizeof(_lastError) - 1);
    _lastError[sizeof(_lastError) - 1] = '\0';
    
    logf("[AutoOTA] ERROR: %s", error);
    
    if (_onUpdateError) {
        _onUpdateError(error);
    }
}

void ESP32_AutoOTA::blinkLED(int times, int delayMs) {
    if (_statusLED < 0) return;
    
    for (int i = 0; i < times; i++) {
        digitalWrite(_statusLED, HIGH);
        delay(delayMs);
        digitalWrite(_statusLED, LOW);
        delay(delayMs);
    }
}

void ESP32_AutoOTA::log(const char* message) {
    if (_debugMode) {
        Serial.println(message);
    }
}

void ESP32_AutoOTA::logf(const char* format, ...) {
    if (_debugMode) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.println(buffer);
    }
}
