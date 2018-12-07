/*
 Name:		WebSocketTest.ino
 Created:	12/6/2018 3:20:23 AM
 Author:	Denis
*/
#include <WebSocketsClient.h>
#include <WiFi.h>

const int SERIAL_BAUD = 115200;

#define WEBSOCKET_PORT 81

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

String url = "/";
String host = "ws://192.168.4.1";

WebSocketsClient webSocketClient;
WiFiClient client;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(SERIAL_BAUD);
	Serial.printf("[%d] Serial COM started on baud rate %d", millis(), SERIAL_BAUD);
	initWiFi();
	initWebSocket();
}

void initWiFi() {
	int startTime = millis();
	Serial.printf("[%d] Initializing WiFi Connection: ", millis());
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(50);
		Serial.print(".");
	}
	Serial.println(" OK");
	host = WiFi.gatewayIP().toString();
	Serial.printf("[%d] Gateway IP is: %s\n", millis(), host);
	int deltaTime = millis() - startTime;
	Serial.printf("[%d] Wifi initialization took %d ms\n", millis(), deltaTime);
}

void initWebSocket() {
	int startTime = millis();
	Serial.printf("[%d] Initializing WebSocket connection ... ", millis());
	webSocketClient.begin(host, WEBSOCKET_PORT, url);
	delay(1000);
	webSocketClient.onEvent(webSocketEvent);
	int deltaTime = millis() - startTime;
	Serial.printf("[%d] WebSocket initialization took %d ms\n", millis(), deltaTime);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch (type) {
	case WStype_DISCONNECTED:
		Serial.printf("[%d][WebSocket] Disconnected!\n", millis());
		break;
	case WStype_CONNECTED:
		Serial.printf("[%d][WebSocket] Connected to url: %s\n", millis(), payload);
		webSocketClient.sendTXT("OK");
		delay(5000);
		break;
	case WStype_TEXT:
		Serial.printf("[%d][WebSocket] get text: %s\n", millis(), payload);
		if (payload[0] == 'O' && payload[1] == 'K' && payload[2] == '?')
			webSocketClient.sendTXT("OK");
		// webSocket.sendTXT("message here");
		break;
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
	webSocketClient.loop();
}
