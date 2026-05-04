#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include "time.h"
#include "website.h"

const char* ssid = "HONOR-10AP1K";
const char* password = "0503844860";

// NTP Settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 10800;  // GMT+3 (60*60*3)
const int daylightOffset_sec = 0;

#define RX_PIN 18
#define TX_PIN 17

WebServer server(80);
String temp = "0", hum = "0", motion = "0";

// Logging Variables
struct HourlyData {
  float avgTemp = 0;
  float avgHum = 0;
  int motionCount = 0;
  int hour = -1;
  bool active = false;
};

HourlyData history[24];
float currentHourSumT = 0, currentHourSumH = 0;
int readingCount = 0;
int lastHour = -1;
int lastDay = -1;
int savedHistoryDay = -1;
int motionCount = 0;
int lastMotionState = 0;
String lastMotionTime = "Never";

String clean(String s) {
  String out = "";
  for (int i = 0; i < s.length(); i++) {
    if (isDigit(s[i]) || s[i] == '.' || s[i] == '-') out += s[i];
  }
  return out;
}

String getField(String line, String key) {
  int start = line.indexOf(key);
  if (start == -1) return "";

  start += key.length();

  int end = line.indexOf(",", start);
  if (end == -1) end = line.length();

  return clean(line.substring(start, end));
}

void clearHistory() {
  for (int i = 0; i < 24; i++) {
    history[i].avgTemp = 0;
    history[i].avgHum = 0;
    history[i].motionCount = 0;
    history[i].hour = -1;
    history[i].active = false;
  }
}

void saveHistory(int day) {
  File file = LittleFS.open("/history.json", "w");
  if (!file) return;

  file.print("{\"day\":");
  file.print(day);
  file.print(",\"lastMotion\":\"");
  file.print(lastMotionTime);
  file.print("\",\"data\":[");


  bool first = true;
  for (int i = 0; i < 24; i++) {
    if (history[i].active) {
      if (!first) file.print(",");

      file.print("{\"hr\":");
      file.print(history[i].hour);
      file.print(",\"t\":");
      file.print(history[i].avgTemp, 1);
      file.print(",\"h\":");
      file.print(history[i].avgHum, 1);
      file.print(",\"m\":");
      file.print(history[i].motionCount);
      file.print("}");

      first = false;
    }
  }

  file.print("]}");
  file.close();
}

int getJsonInt(String obj, String key) {
  int start = obj.indexOf(key);
  if (start == -1) return -1;
  start += key.length();

  int end = obj.indexOf(",", start);
  if (end == -1) end = obj.indexOf("}", start);

  return obj.substring(start, end).toInt();
}

String getJsonString(String obj, String key) {
  int start = obj.indexOf(key);
  if (start == -1) return "";

  start += key.length();

  int end = obj.indexOf("\"", start);
  if (end == -1) return "";

  return obj.substring(start, end);
}

float getJsonFloat(String obj, String key) {
  int start = obj.indexOf(key);
  if (start == -1) return 0;
  start += key.length();

  int end = obj.indexOf(",", start);
  if (end == -1) end = obj.indexOf("}", start);

  return obj.substring(start, end).toFloat();
}

void loadHistory() {
  if (!LittleFS.exists("/history.json")) return;

  File file = LittleFS.open("/history.json", "r");
  if (!file) return;

  String json = file.readString();
  file.close();

  savedHistoryDay = getJsonInt(json, "\"day\":");

  String savedLastMotion = getJsonString(json, "\"lastMotion\":\"");
  if (savedLastMotion != "") {
    lastMotionTime = savedLastMotion;
  }

  int pos = 0;
  while (true) {
    int start = json.indexOf("{\"hr\":", pos);
    if (start == -1) break;

    int end = json.indexOf("}", start);
    if (end == -1) break;

    String obj = json.substring(start, end + 1);

    int hr = getJsonInt(obj, "\"hr\":");
    float t = getJsonFloat(obj, "\"t\":");
    float h = getJsonFloat(obj, "\"h\":");
    int m = getJsonInt(obj, "\"m\":");
    if (m < 0) m = 0;

    if (hr >= 0 && hr < 24) {
      history[hr].hour = hr;
      history[hr].avgTemp = t;
      history[hr].avgHum = h;
      history[hr].motionCount = m;
      history[hr].active = true;
    }

    pos = end + 1;
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
  } else {
    loadHistory();
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.on("/", []() {
    server.send_P(200, "text/html", PAGE_DATA);
  });

  server.on("/data", []() {
    String json = "{\"t\":\"" + clean(temp) + "\",\"h\":\"" + clean(hum) + "\",\"m\":\"" + motion + "\",\"lastMotion\":\"" + lastMotionTime + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/history", []() {
    String json = "[";
    bool first = true;

    for (int i = 0; i < 24; i++) {
      if (history[i].active) {
        if (!first) json += ",";

        json += "{\"hr\":" + String(history[i].hour) + ",\"t\":" + String(history[i].avgTemp) + ",\"h\":" + String(history[i].avgHum) + ",\"m\":" + String(history[i].motionCount) + "}";

        first = false;
      }
    }

    json += "]";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');

    String newHum = getField(line, "H:");
    String newTemp = getField(line, "T:");
    String newMotion = getField(line, "M:");

    if (newMotion != "") {
      motion = newMotion;
    }

    if (newHum != "" && newTemp != "") {
      hum = newHum;
      temp = newTemp;

      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        int currentHour = timeinfo.tm_hour;
        int currentDay = timeinfo.tm_yday;

        if (lastDay == -1) {
          lastDay = currentDay;

          if (savedHistoryDay != -1 && savedHistoryDay != currentDay) {
            clearHistory();
            saveHistory(currentDay);
          }
        }

        if (currentHour == 0 && currentDay != lastDay) {
          clearHistory();

          currentHourSumT = 0;
          currentHourSumH = 0;
          readingCount = 0;
          motionCount = 0;
          lastHour = currentHour;
          lastDay = currentDay;

          saveHistory(currentDay);
        }

        if (currentHour != lastHour) {
          if (lastHour != -1) {
            saveHistory(currentDay);
          }

          currentHourSumT = 0;
          currentHourSumH = 0;
          readingCount = 0;
          motionCount = 0;
          lastHour = currentHour;
        }

        currentHourSumT += temp.toFloat();
        currentHourSumH += hum.toFloat();
        readingCount++;

        int currentMotionState = motion.toInt();

        if (currentMotionState == 1 && lastMotionState == 0) {
          motionCount++;

          char timeText[32];
          strftime(timeText, sizeof(timeText), "%Y-%m-%d %H:%M:%S", &timeinfo);
          lastMotionTime = String(timeText);

          saveHistory(currentDay);
        }

        lastMotionState = currentMotionState;

        history[currentHour].avgTemp = currentHourSumT / readingCount;
        history[currentHour].avgHum = currentHourSumH / readingCount;
        history[currentHour].motionCount = motionCount;
        history[currentHour].hour = currentHour;
        history[currentHour].active = true;
      }
    }
  }
}
