/**
 * Wireless door controller, which allows a relay module
 * to be driven HIGH or LOW by clients making HTTP requests
 * to /H or /L URLs. 
 * This version uses AP mode to host its own Wi-Fi network
 * which clients connect to, and assigns an IP address to
 * itself of 192.168.4.1
 * Reference: Playful Technology
*/
// INCLUDES
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// CONSTANTS
// Create AP name
const char ssid[] = "DoorController";
// Create AP password 
// Password need to be 8 chars
const char password[] = "12345678";
const int relayPin = D1;
// After what time will door automatically re-lock
const unsigned long relockDelay = 5000;

// GLOBALS
ESP8266WebServer server(80);
// Timestamp at which door was last unlocked
unsigned long lastUnlockTime;
enum State { Locked, Unlocked };
State state = State::Locked;

// CALLBACKS
void displayPage() {
  server.send(200, "text/html", "<h1>Relay Controller</h1>Relay <a href=\"/H\">HIGH</a><br>Relay <a href=\"/L\">LOW</a>");
}
// Send error message when cannot find the server
void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  // Start serial connection for debugging
  Serial.begin(115200);
  Serial.println(__FILE__ __DATE__);

  // Initialise relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Initialise the network
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("Access Point ");
  Serial.print(ssid);
  Serial.println(" started");
  // Default IP address is 192.168.4.1
  // Type this IP address to Unity App to control maglock
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Define the different resources which the user can request
  server.on("/", []() {
    displayPage();
  });
  server.on("/H", [](){
    Serial.println(F("Unlocking"));
    digitalWrite(relayPin, HIGH);
    lastUnlockTime = millis();
    state = State::Unlocked;
    displayPage();
  });
  server.on("/L", [](){
    Serial.println(F("Locking"));
    digitalWrite(relayPin, LOW);
    state = State::Locked;
    displayPage();
  });
  server.onNotFound(handleNotFound);
  
  // Start the server
  server.begin();
  Serial.println(F("HTTP server started"));
}

void loop() {
  server.handleClient();
  if(state == State::Unlocked && millis() - lastUnlockTime > relockDelay) {
    Serial.println(F("Auto Re-locking"));
    digitalWrite(relayPin, LOW);
    state = State::Locked;
  }
}
