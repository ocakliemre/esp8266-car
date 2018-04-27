#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

static const char ssid[] = "WIRELESS_CAR";
int motors[2][2] = {{16, 5}, {4,0}}; //forward, backward
                                     //pwms connected to 5v, always %100
                                     //left motor, right motor

int fail_led = 14;
int success_led = 12;

IPAddress local_IP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
WiFiServer tserver(81);
WiFiClient client;

void setup() {
  for (int i = 0; i < sizeof(motors) / sizeof(int[2]); i++) {
    for (int x = 0; x < sizeof(motors[i]) / sizeof(int); x++) {
      pinMode(motors[i][x], OUTPUT);
    }
  }

  pinMode(fail_led, OUTPUT);
  pinMode(success_led, OUTPUT);
  digitalWrite(fail_led, 1);
  digitalWrite(success_led, 1);

  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(fail_led, 0);
      delay(500);
      digitalWrite(fail_led, 1);
      delay(500);
    }
    digitalWrite(fail_led, 0);
    ESP.deepSleep(0);
  } else {
    if (!WiFi.softAP(ssid)) {

      for (int i = 0; i < 4; i++) {
        digitalWrite(fail_led, 0);
        delay(500);
        digitalWrite(fail_led, 1);
        delay(500);
      }
      digitalWrite(fail_led, 0);
      ESP.deepSleep(0);
    } else {
      server.on("/", handleRoot);
      server.onNotFound(handleRoot);

      server.begin();

      tserver.begin();
      tserver.setNoDelay(true);
      digitalWrite(success_led, 0);
    }
  }
}

void sevent(String payloads) {
  char payload[2];
  payloads.toCharArray(payload, 2);
    if (strcmp("lf", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 1);
      digitalWrite(motors[0][1], 0);
    } else if (strcmp("lb", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 0);
      digitalWrite(motors[0][1], 1);
    } else if (strcmp("ls", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 0);
      digitalWrite(motors[0][1], 0);
    } else if (strcmp("rf", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 1);
      digitalWrite(motors[1][1], 0);
    } else if (strcmp("rb", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 0);
      digitalWrite(motors[1][1], 1);
    } else if (strcmp("rs", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 0);
      digitalWrite(motors[1][1], 0);
    }
    return;
}

void handleRoot() {
  server.send_P(200, "text/html", "ESP8266 controlled car<br><br>IP: 192.168.1.1<br>HTTP port: 80<br>TCP control port: 81");
}


void loop() {
  server.handleClient();
    if (!client.connected()) {
        // try to connect to a new client
        client = tserver.available();
    } else {
        // read data from the connected client
        if (client.available() > 0) {
            sevent(client.readStringUntil('\r'));
            client.flush();
        }
    }
}


