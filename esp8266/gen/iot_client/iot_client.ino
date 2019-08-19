/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

*/

// Compiler
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define UDP_PORT 10101
#define SERVER_PORT 10101

// Functions
void set_server_ip();
void run_command(WiFiClient client, char *command);

// Variables
const char* ssid     = "living_room_ap";
const char* password = "PlayingWithFire$$$";

const char* host = "data.sparkfun.com";
const char* streamId   = "....................";
const char* privateKey = "....................";

byte server_ip[4];

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  set_server_ip();

  // Testing connectiong and purging any previously connected clients:
  Serial.print("Testing connection...");
  WiFiClient client;
  while (!client.connect(server_ip, SERVER_PORT)) {
    Serial.print("[E]");
    delay(5000);
  }
  Serial.println(" Success!");
  run_command(client, "client_restart");
  client.stop();

}

void loop() {

  Serial.print("Connecting to server...");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(server_ip, SERVER_PORT)) {
    Serial.println("connection failed");
    return;
  }
  Serial.println("Connected.");

  delay(20000);

//  // This will send the request to the server
//  client.print(String("execute ") + command);
//  unsigned long timeout = millis();
//  while (client.available() == 0) {
//    if (millis() - timeout > 5000) {
//      Serial.println(">>> Client Timeout !");
//      client.stop();
//      return;
//    }
//  }

//  // Read all the lines of the reply from server and print them to Serial
//  while (client.available()) {
//    String line = client.readStringUntil('\r');
//    Serial.print(line);
//  }

  // Manually shuts down the TCP connection from the client side.
  // This is NOT the same as 'unplugging' the internet connection or restarting the device.
  client.stop();
  delay(1000);
}

// PRIVATE FUNCTIONS

void set_server_ip() {
  // Retrieving server IP address on same subnet with a UDP broadcast
  Serial.println("Obtaining server address...");
  
  WiFiUDP udp;
  udp.begin(UDP_PORT);
  
  byte broadcast_ip[4];
  char incomingPacket[255];  // buffer for incoming udp packets
  
  for (int i = 0; i < 3; i++)
    broadcast_ip[i] = WiFi.localIP()[i];
  broadcast_ip[3] = 255;

  Serial.println("Broadcasting ip request");
  udp.beginPacket(broadcast_ip, UDP_PORT);
  udp.write("ip_address", 10);
  udp.endPacket();

  // Waiting for a response with the server IP address:
  int packetSize = 0;
  while (!packetSize) {
    packetSize = udp.parsePacket();
    if (packetSize) {
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
        Serial.println("WARNING: Received more bytes than expected");
      }
      incomingPacket[packetSize] = 0;
      
      Serial.printf("UDP packet contents: %04u\n", incomingPacket);
    }
  }

  // Parsing out the ip address from the received bytes
  for (int i = 0; i < 4; i++)
    server_ip[i] = incomingPacket[i];
  Serial.printf("Retreived server IP address: %d.%d.%d.%d\n", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
  
  udp.stop();
}

void run_command(WiFiClient client, char *command) {
  client.print(command);
}


