

// Compiler
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Led_Grid.h>
#include <arduinoFFT.h>
#include <iot_core.h>

// IOT Core function

IotCore iot;

////// FFT VARIABLES //////

arduinoFFT FFT = arduinoFFT();
const int fft_width = 128;
double vReal[fft_width];
double vImag[fft_width];

////// LIGHT GRID VARIABLES //////

LedGrid grid(20, 11, 13);
int loop_delay = 20;
int light_mode = 0;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to wifi");

  while (!iot.connect_to_wifi())
    Serial.println("Wifi Connection failed.");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  iot.set_server_ip();

  // Testing connectiong and purging any previously connected clients:
  Serial.print("Testing connection...");
  WiFiClient client;
  
  while (!client.connect(iot.server_ip, SERVER_PORT)) {
    Serial.print("[E]");
    delay(5000);
  }
  Serial.println(" Success!");
  iot.send_command(client, "client_restart");
  client.stop();

  initialize_grid();

}

void loop() {

  Serial.print("Connecting to server...");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(iot.server_ip, SERVER_PORT)) {
    Serial.println("connection failed");
    return;
  }
  Serial.println("Connected.");


  // Subscribing to the light grid config value:
  iot.send_command(client, "config_subscribe light_grid_setting");
  while (! client.available())
    delay(1);
  String grid_mode = "0";
  delay(100);
  
  while (client.connected() || client.available()) {

    if (client.available()) {
      grid_mode = client.readStringUntil('\n');
      Serial.println("Setting grid setting to " + grid_mode);
      apply_grid_arguments(grid_mode);
    }
    
    grid.update();
    delay(loop_delay);
    
  }

  // Manually shuts down the TCP connection from the client side.
  // This is NOT the same as 'unplugging' the internet connection or restarting the device.
  client.stop();
  Serial.println("Connection closed");
  delay(1000);
}

// FUNCTIONAL FUNCTIONS

void apply_grid_arguments(String grid_mode) {
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
  
  if (light_mode == 5)
    update_visualizer();
  else if (light_mode == 8)
    update_time();
}

void update_time() {

  grid.set_time_to_display(0, 0, false);
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
