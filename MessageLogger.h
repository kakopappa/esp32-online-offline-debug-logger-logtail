#ifndef MESSAGE_LOGGER_H
#define MESSAGE_LOGGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

class MessageLogger {
private:
  const char* logTailSourceToken;
  const char* logFilePath;
  const char* ntpServer;
  const long gmtOffset_sec;
  const int daylightOffset_sec;

  bool isConnected();
  String getTimestamp();
  String formatLogMessage(const String& message, const String& level);
  void appendToFile(const String& formattedMessage);
  void postToLogTail(const String& formattedMessage);
  void postStoredMessages();

public:
  MessageLogger(const char* sourceToken);
  void setupTime();
  void logMessage(const String& message, const String& level = "info");
  void checkAndPostStored();
};

#endif  // MESSAGE_LOGGER_H