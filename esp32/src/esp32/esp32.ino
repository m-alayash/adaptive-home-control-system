#include <WiFi.h>
#include <WebServer.h>
#include "website.h"
extern const char PAGE_DATA[];
const char* ssid = "HONOR-10AP1K";
const char* password = "0503844860";

#define RX_PIN 18 
#define TX_PIN 17 

WebServer server(80);
String temp = "0", hum = "0";

// Helper to ensure we only have numeric characters
String clean(String s) {
  String out = "";
  for(int i=0; i<s.length(); i++) {
    if(isDigit(s[i]) || s[i] == '.' || s[i] == '-') out += s[i];
  }
  return out;
}



void setup() {
  // so serial monitor doesn't freak out
  delay(1000);
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  
  // Routes
  server.on("/", [](){ server.send_P(200, "text/html", PAGE_DATA); });
  
  server.on("/data", [](){
    String json = "{\"t\":\"" + clean(temp) + "\",\"h\":\"" + clean(hum) + "\"}";
    server.send(200, "application/json", json);
  });

  server.begin();
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

  if (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    int hIdx = line.indexOf("H:");
    int tIdx = line.indexOf("T:");
    
    if (hIdx != -1 && tIdx != -1) {
      hum = clean(line.substring(hIdx + 2, line.indexOf(",", hIdx)));
      temp = clean(line.substring(tIdx + 2));
    }
  }
}