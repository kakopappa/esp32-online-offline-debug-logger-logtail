#include "MessageLogger.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

// Root certificate for in.logtail.com
const char* root_ca = R"(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)";

MessageLogger::MessageLogger(const char* sourceToken)
  : logTailSourceToken(sourceToken),
    logFilePath("/log.txt"),
    ntpServer("pool.ntp.org"),
    gmtOffset_sec(0),
    daylightOffset_sec(0) {}

void MessageLogger::setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

bool MessageLogger::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String MessageLogger::getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  char timeStringBuff[30];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
  return String(timeStringBuff);
}

String MessageLogger::formatLogMessage(const String& message, const String& level) {
  String macAddress = WiFi.macAddress();
  String timestamp = getTimestamp();
  return "{\"dt\":\"" + timestamp + "\",\"level\":\"" + level + "\",\"message\":\"" + message + "\",\"mac\":\"" + macAddress + "\"}";
}

void MessageLogger::appendToFile(const String& formattedMessage) {
  File file = SPIFFS.open(logFilePath, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.println(formattedMessage)) {
    Serial.println("Message appended to file");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void MessageLogger::postToLogTail(const String& formattedMessage) {
    WiFiClientSecure client;
    client.setCACert(root_ca);

    if (!client.connect("in.logtail.com", 443)) {
        Serial.println("Connection failed!");
        return;
    }

    String httpRequest = String("POST / HTTP/1.1\r\n") +
                         "Host: in.logtail.com\r\n" +
                         "Authorization: Bearer " + logTailSourceToken + "\r\n" +
                         "Content-Type: application/json\r\n" +
                         "Content-Length: " + String(formattedMessage.length()) + "\r\n" +
                         "Connection: close\r\n\r\n" +
                         formattedMessage;

    client.print(httpRequest);

    // Wait for the server's response
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

     // Read the response status line
    String statusLine = client.readStringUntil('\r');
    int statusCode = statusLine.substring(9, 12).toInt();

    // Skip the headers
    while (client.available()) {
        String line = client.readStringUntil('\r');
        if (line == "\n") {
            break;
        }
    }

     // Read the response body (if any)
    String responseBody = client.readString();

    client.stop();

    // Check the status code and print the result
    if (statusCode >= 200 && statusCode < 300) {
        Serial.println("Post successful. Status code: " + String(statusCode));
    } else {
        Serial.println("Post failed. Status code: " + String(statusCode));
        Serial.println("Response: " + responseBody);
    }    
}

void MessageLogger::postStoredMessages() {
  File file = SPIFFS.open(logFilePath, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String line;
  while (file.available()) {
    line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      postToLogTail(line);
    }
  }
  file.close();

  // Clear the file after posting all messages
  SPIFFS.remove(logFilePath);
}

void MessageLogger::logMessage(const String& message, const String& level) {
  String formattedMessage = formatLogMessage(message, level);
  if (isConnected()) {
    postToLogTail(formattedMessage);
    postStoredMessages();
  } else {
    appendToFile(formattedMessage);
  }
}

void MessageLogger::checkAndPostStored() {
  if (isConnected()) {
    postStoredMessages();
  }
}