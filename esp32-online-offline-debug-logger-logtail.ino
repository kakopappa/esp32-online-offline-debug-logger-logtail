#include <WiFi.h>
#include <SPIFFS.h>

#include "MessageLogger.h"

const char* ssid = "";
const char* password = "";
const char* logTailSourceToken = "";

MessageLogger logger = MessageLogger(logTailSourceToken);

void setup() {
    Serial.begin(115200);
    
    if (!SPIFFS.begin(true)) {
        Serial.println("An error occurred while mounting SPIFFS");
    }

    logger.logMessage("This message was added before wifi connection!");

    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }

    logger.setupTime();

    Serial.println("Connected to WiFi");

    logger.logMessage("This message was added after wifi connection!");
}

void loop() {
    logger.logMessage("This is an info message");
    logger.logMessage("This is a warning message", "warn");
    logger.logMessage("This is an error message", "error");
    logger.checkAndPostStored();
    delay(60000); // Wait for a minute before next log
}