#include <ESP8266WiFi.h>

const char * ssid = "WIRELESS_CAR";

int buttons[4] = {16, 5, 4, 0}; //up down left right
int fail_led = 14;
int success_led = 12;

WiFiClient client;

void setup() {
  for (int i = 0; i < sizeof(buttons) / sizeof(int); i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  pinMode(fail_led, OUTPUT);
  pinMode(success_led, OUTPUT);
  digitalWrite(fail_led, 1);
  digitalWrite(success_led, 1);

  WiFi.begin(ssid);

  //both red and green leds flicker while connecting
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(fail_led, 0);
    digitalWrite(success_led, 0);
    delay(100);
    digitalWrite(fail_led, 1);
    digitalWrite(success_led, 1);
    delay(100);
  }

  //create a connection to the car
  if (!client.connect("192.168.1.1", 81)) {
    digitalWrite(fail_led, 0);
    ESP.deepSleep(0); //if fails, shutdown
  }
}

void loop() {
  if (client.connected()) {
    digitalWrite(success_led, 0);
    digitalWrite(fail_led, 1);

    //read buttons (to-do; use bytes to lower the size)
    bool cl = false;
    for (int i = 0; i < sizeof(buttons) / sizeof(int); i++) {
      if (!digitalRead(buttons[i])) { //if button IS pressed (because it's connected to GND, to use internal pullup)
        cl = true; //at least a button was clicked
        switch (i) {
          case 0:
            client.print("lf\r");
            client.print("rf\r");
            break;
          case 1:
            client.print("lb\r");
            client.print("rb\r");
            break;
          case 2:
            client.print("ls\r");
            client.print("rf\r");
            break;
          case 3:
            client.print("lf\r");
            client.print("rs\r");
            break;
        }
      }
    }

    //no buttons clicked? stop all motors
    if (!cl) {
      client.print("ls\r");
      client.print("rs\r");
    }
  } else {
    digitalWrite(success_led, 1);
    digitalWrite(fail_led, 0);
  }

  delay(100);
}

