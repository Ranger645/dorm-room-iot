
#include "iot_core.h"

int IotCore::connect_to_wifi() {
	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	int count = 0;
	while (WiFi.status() != WL_CONNECTED && count < WIFI_CONNECT_RETRIES) {
		delay(500);
		count++;
	}
	WiFi.setSleepMode(WIFI_NONE_SLEEP);
	return count < WIFI_CONNECT_RETRIES && WiFi.status() == WL_CONNECTED;
}

void *IotCore::connect_to_server() {
	
}

void *IotCore::connect_to_server_udp_broadcast() {

}

void IotCore::set_server_ip() {
	// Retrieving server IP address on same subnet with a UDP broadcast
	Serial.println("Obtaining server address...");

	WiFiUDP udp;
	udp.begin(UDP_PORT);

	byte broadcast_ip[4];
	char incomingPacket[255]; // buffer for incoming udp packets

	for (int i = 0; i < 3; i++)
		broadcast_ip[i] = WiFi.localIP()[i];
	broadcast_ip[3] = 255;

	int packetSize = 0;

	while (packetSize == 0) {

		Serial.println("Broadcasting ip request");
		udp.beginPacket(broadcast_ip, UDP_PORT);
		udp.write("ip_address", 10);
		udp.endPacket();

		// Waiting for a response with the server IP address:
		unsigned long time = millis();
		while (!packetSize && millis() - time < 1000) {
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
	}

	// Parsing out the ip address from the received bytes
	for (int i = 0; i < 4; i++)
		server_ip[i] = incomingPacket[i];
	Serial.printf("Retreived server IP address: %d.%d.%d.%d\n", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);

	udp.stop();
}

void IotCore::send_command(WiFiClient client, char *command) {
  	client.println(command);
}
