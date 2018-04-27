#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
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
WebSocketsServer webSocket = WebSocketsServer(81);

MDNSResponder mdns;
ESP8266WiFiMulti WiFiMulti;


void setup() {
  for (int i = 0; i < sizeof(motors) / sizeof(int[2]); i++) {
    for (int x = 0; x < sizeof(motors[i]) / sizeof(int); x++) {
      pinMode(motors[i][x], OUTPUT);
    }
  }

  pinMode(fail_led, OUTPUT);
  pinMode(success_led, OUTPUT);
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(fail_led, 1);
      delay(500);
      digitalWrite(fail_led, 0);
      delay(500);
    }
    digitalWrite(fail_led, 1);
    ESP.deepSleep(0);
  } else {
    if (!WiFi.softAP(ssid)) {

      for (int i = 0; i < 4; i++) {
        digitalWrite(fail_led, 1);
        delay(500);
        digitalWrite(fail_led, 0);
        delay(500);
      }
      digitalWrite(fail_led, 1);
      ESP.deepSleep(0);
    } else {

      //i'm not sure what does DNS responder do.
      //I think it will work without it, as we are not resolving any domains.
      if (!mdns.begin("espWebSock", WiFi.localIP())) {

        for (int i = 0; i < 8; i++) {
          digitalWrite(fail_led, 1);
          delay(500);
          digitalWrite(fail_led, 0);
          delay(500);
        }
        digitalWrite(fail_led, 1);
        ESP.deepSleep(0);

      }

      mdns.addService("http", "tcp", 80);
      mdns.addService("ws", "tcp", 81);

      server.on("/", handleRoot);
      server.onNotFound(handleRoot);

      server.begin();

      webSocket.begin();
      webSocket.onEvent(webSocketEvent);
      digitalWrite(success_led, 1);
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      digitalWrite(fail_led, 1);
      digitalWrite(success_led, 0);
    break;
    case WStype_CONNECTED:
      digitalWrite(fail_led, 0);
      digitalWrite(success_led, 1);
    break;
    case WStype_TEXT:
    if (strcmp("left:forward", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 1);
      digitalWrite(motors[0][1], 0);
    } else if (strcmp("left:backward", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 0);
      digitalWrite(motors[0][1], 1);
    } else if (strcmp("left:stop", (const char * ) payload) == 0) {
      digitalWrite(motors[0][0], 0);
      digitalWrite(motors[0][1], 0);
    } else if (strcmp("right:forward", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 1);
      digitalWrite(motors[1][1], 0);
    } else if (strcmp("right:backward", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 0);
      digitalWrite(motors[1][1], 1);
    } else if (strcmp("right:stop", (const char * ) payload) == 0) {
      digitalWrite(motors[1][0], 0);
      digitalWrite(motors[1][1], 0);
    }
    break;
  }
}

void handleRoot() {
  server.send_P(200, "text/html", "ESP8266 controlled car<br><br>IP: 192.168.1.1<br>HTTP port: 80<br>Websocket port: 81");
}


void loop() {
  webSocket.loop();
  server.handleClient();
}

