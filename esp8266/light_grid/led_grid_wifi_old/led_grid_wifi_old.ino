
#include <ESP8266WiFi.h>
#include <Led_Grid.h>
#include <arduinoFFT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

////// WIFI PARAMETERS //////

// WiFi parameters
#define WLAN_SSID       "SpeedStreamMF"
#define WLAN_PASS       "GregTim2812"

// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "Ranger645"
#define AIO_KEY         "e28577c869e24a2fb489ceadff583895"

// Wifi client object that will be passed to the MQTT object to initialize it.
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Subscriptions:
Adafruit_MQTT_Subscribe sub_time = Adafruit_MQTT_Subscribe(&mqtt, "time/ISO-8601");
Adafruit_MQTT_Subscribe sub_mode = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/LED Strip Mode");

// Connect function declaration
void connect();

////// FFT VARIABLES //////

arduinoFFT FFT = arduinoFFT();
const int fft_width = 128;
double vReal[fft_width];
double vImag[fft_width];

///////////////////////////

LedGrid grid(20, 11, 13);
int loop_delay = 20;
int light_mode = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);
  
  initialize_grid();
  initialize_wifi();
}

void loop() {
  if (! mqtt.connected()) {
    Serial.println("MQTT Disconnected, Reconnecting...");
    connect();
  }
  
  Adafruit_MQTT_Subscribe *subscription;
  subscription = mqtt.readSubscription(1);
  
  if (subscription == &sub_mode) {
    char *incoming_data = (char *) sub_mode.lastread;
    String grid_mode = String(incoming_data);

    // Parsing the arguments in the mode switch command.
    int arg_count = 1;
    for (int i = 0; i < grid_mode.length(); i++)
      if (grid_mode[i] == ',')
        arg_count++;
    String arguments[arg_count];
    int current_argument = 0;
    for (int i = 0; i < grid_mode.length(); i++)
      if (grid_mode[i] == ',')
        current_argument++;
      else
        arguments[current_argument] += grid_mode[i];
    grid.turn_off();
    light_mode = arguments[0].toInt();
    loop_delay = 20;
    switch (light_mode) {
      case 1:
        grid.setup_solid_color(arguments[1].toInt(), arguments[2].toInt(), arguments[3].toInt(), arguments[4].toInt());
        break;
      case 2:
        grid.setup_breathing_color(arguments[1].toInt(), arguments[2].toInt(), arguments[3].toInt(), arguments[4].toInt());
        break;
      case 3:
        grid.setup_rainbow(arguments[1].toInt(), arguments[2].toInt());
        break;
      case 4:
        grid.setup_rainbow_wave(arguments[1].toInt(), arguments[2].toInt());
        break;
      case 5:
        grid.setup_sound_visualizer(arguments[1].toInt());
        loop_delay = 0;
        break;
      case 6:
        grid.setup_party_mode(arguments[1].toInt(), arguments[2].toInt());
        loop_delay = 0;
        break;
      case 7:
        grid.setup_text(arguments[1], arguments[2].toInt(), arguments[3].toInt(), arguments[4].toInt(), arguments[5].toInt(), arguments[6].toInt(), false);
        break;
      case 8:
        grid.setup_time(arguments[1].toInt(), arguments[2].toInt(), arguments[3].toInt(), arguments[4].toInt(), arguments[5].toInt(), false);
        break;
      default:
        grid.turn_off();
    }
  }
  
  if (light_mode == 5)
    update_visualizer();
  else if (light_mode == 8)
    update_time();
  
  grid.update();
  delay(loop_delay);
}

void update_time() {
  char *raw_time_values = (char *) sub_time.lastread;
  String time_string = String(raw_time_values);

  int startOfHoursIndex = time_string.indexOf("T") + 1;
  int firstColonIndex = time_string.indexOf(":");
  int hour = time_string.substring(startOfHoursIndex, firstColonIndex).toInt();
  time_string = time_string.substring(firstColonIndex + 1);
  int minute = time_string.substring(0, time_string.indexOf(":")).toInt();
  hour -= 4;
  if (hour <= 0)
    hour += 12;

  grid.set_time_to_display(hour, minute, false);
}

void update_visualizer() {
  for (int i = 0; i < 128; i++) {
    vReal[i] = analogRead(A0) - 512;
    vImag[i] = 0;
  }

  FFT.Windowing(vReal, fft_width, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  FFT.Compute(vReal, vImag, (uint16_t)fft_width, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, (uint16_t)fft_width); /* Compute magnitudes */

  grid.set_fft_raw_data(&(vReal[8]), 40);
}

void initialize_grid() {
  for (int i = 0; i < fft_width; i++) {
    vReal[i] = 0;
    vImag[i] = 0;
  }
  
  grid.start_grid();
  
  grid.turn_off();
  grid.update();
}

void initialize_wifi() {
  Serial.println();

  Serial.print("Connecting to SSID: ");
  Serial.print(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS, 0);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected");
  mqtt.subscribe(&sub_time);
  mqtt.subscribe(&sub_mode);

  // connect to adafruit io
  connect();
}

// connect to adafruit io via MQTT
void connect() {
  int8_t ret;

  Serial.println("Connecting to adafruit IO");
  while ((ret = mqtt.connect()) != 0) {

    if (ret >= 0)
      mqtt.disconnect();
    delay(5000);

  }
  Serial.println("Successfully connected to adafruit IO");

}

