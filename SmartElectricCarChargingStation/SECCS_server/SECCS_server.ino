// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
	Name:       PametnaPolnilnica_server.ino
	Created:	2. 12. 2018 20:56:59
	Author:     DENIS-PC\Denis
*/

// Define User Types below here or use a .h file
//

#include <WebSocketsServer.h>
#include <WiFi.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "WROVER_KIT_LCD.h"

#define NUM_OF_SCT013_SENSORS 3

#define INTERRUPT_PIN 2
int interruptCounter = 0;
float kWhMeter = 0.0;

WROVER_KIT_LCD Display;

#define WEBSOCKET_PORT 81
WebSocketsServer webSocket = WebSocketsServer(WEBSOCKET_PORT);

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";
// Define Functions below here or use other .ino or cpp files

int average[NUM_OF_SCT013_SENSORS];
int clientNum = NULL;
// The setup() function runs once each time the micro-controller starts
void setup()
{
	Serial.begin(115200);
	initDisplay();
	initWiFi();
	initWebSocket();
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), increaseCounter, RISING);
}

void initDisplay() {
	Serial.printf("[%s][%d] Initializing LCD Display ... ", "initDisplay", millis());
	Display.begin();
	Display.fillScreen(WROVER_BLACK);
	Serial.printf(" OK\n");
}

void initWiFi() {
	Serial.printf("[%s] [%d] Initializing Access Point .", "initWiFi", millis());
	while (!WiFi.softAP(ssid, password)) {
		delay(100);
		Serial.printf(".");
	}
	Serial.printf(" OK\n");
	Serial.printf("[%s][%d] AP IP: ", "initWiFi", millis());
	Serial.println(WiFi.softAPIP());
}

void initWebSocket() {
	Serial.printf("[%s] [%d] Initializing WebSocket ... ", "initWebSocket", millis());
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	Serial.printf(" OK\n");
}
// Add the main program code into the continuous loop() function
void loop()
{
	webSocket.loop();
}

//void pingClients() {
//	Serial.println(webSocket.connectedClients());
//}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

	switch (type) {
	case WStype_DISCONNECTED:
		Serial.printf("[%s] [%d] [%u] Disconnected!\n", "webSocketEvent", millis(), num);
		break;
	case WStype_CONNECTED:{
		IPAddress ip = webSocket.remoteIP(num);
		Serial.printf("[%s] [%d] [%u] Connected from %d.%d.%d.%d url: %s\n", "webSocketEvent", millis(), num, ip[0], ip[1], ip[2], ip[3], payload);

		// send message to client
		webSocket.sendTXT(num, "Connected");
		clientNum = num;
		break;
	}
	case WStype_TEXT:{
		String tempPayload = (char *)payload;
		if (tempPayload == "OK") {
			refreshLCD();
		}
		else if (tempPayload == "hi") {
			Serial.printf("[%s] [%d] New WebSocket Connection open\n", "webSocketEvent", millis());
		}
		else {
			//const char delim[2] = ":";
			//Serial.println((char *)payload);
			Serial.printf("[%s] [%d] Splitting payload: \"%s\" to index and value\n", "webSocketEvent", millis(), payload);
			char *token;
			token = strtok((char *)payload, ":");
			int index;
			index = (String(token).toInt());
			//Serial.print(index);
			token = strtok(NULL, ":");
			int value;
			value = String(token).toInt();
			//Serial.println(value);
			
			Serial.printf("[%s] [%d] Index: %d | value: %d\n", "webSocketEvent", millis(), index, value);
			average[index] = value;
		}
		break;
}
	case WStype_ERROR:
	case WStype_FRAGMENT_TEXT_START:
	case WStype_FRAGMENT_BIN_START:
	case WStype_FRAGMENT:
	case WStype_FRAGMENT_FIN:
		break;
	}
	//printAverage();
	refreshLCD();
}

void refreshLCD() {
	Display.setRotation(1);
	Display.setCursor(0, 0);
	Display.setTextSize(2);
	Display.setTextColor(WROVER_WHITE);
	Display.fillScreen(WROVER_BLACK);
	Display.printf("%4s | %4s", "Sensor", "Value\n");
	Display.println("-------------------------");
	for (int i = 0; i < NUM_OF_SCT013_SENSORS; i++) {
		Display.printf("  #%2d  | %3d ", i + 1, average[i]);
		Display.println();
	}
	Display.println("-------------------------");

}

//void printAverage() {
//	for (int i = 0; i < NUM_OF_SCT013_SENSORS; i++)
//		Serial.printf("| %d ", average[i]);
//	Serial.println("|");
//}

void increaseCounter() {
	interruptCounter++;
	kWhMeter = interruptCounter / 1000.0;
	Serial.printf("[%s] [%d] Number of interrupts: %d / kWh: %.3f\n", "increaseCounter", millis(), interruptCounter, kWhMeter);
}