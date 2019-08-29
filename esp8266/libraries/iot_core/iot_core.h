/* 
 * 
 */

#ifndef iot_core_h
#define iot_core_h

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define WIFI_CONNECT_RETRIES 30

#define UDP_PORT 10101
#define SERVER_PORT 10101

class IotCore {
	public:
		
		// Constructors:
		IotCore() {}

		// Variables
		const char* ssid     = "living_room_ap";
		const char* password = "PlayingWithFire$$$";

		byte server_ip[4];

		int connect_to_wifi();
		void *connect_to_server();
		void *connect_to_server_udp_broadcast();
		void set_server_ip();
		void send_command(WiFiClient client, char *command);
		
	private:
		
};

#endif