/*
 Name:		PametnaPolnilnica.ino
 Created:	11/30/2018 2:33:23 PM
 Author:	Denis
*/
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <driver/adc.h>

#define WEBSOCKET_PORT 81

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

String url = "/";
String host = "ws://192.168.4.1";

WebSocketsClient webSocketClient;
WiFiClient client;

#define SCT013_ADC_CHANNEL ADC1_CHANNEL_0 
#define NUM_OF_READINGS 12
#define MAX_DELTA 20
#define MAX_NUM_OF_TRIES 5
#define NUM_OF_SCT013_SENSORS 3
#define ADC_WIDTH ADC_WIDTH_10Bit
adc1_channel_t SCT013SensorChannels[] = { ADC1_CHANNEL_0, ADC1_CHANNEL_3, ADC1_CHANNEL_6 };

int readings[NUM_OF_SCT013_SENSORS][NUM_OF_READINGS];
int average[NUM_OF_SCT013_SENSORS];


// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	initSCT013Sensors();
	initWiFi();
	initWebSocket();
}

void initSCT013Sensors() {
	Serial.printf("[%s] [%d] Initializing SCT013 Sensors\n", "initSCT013Sensors", millis());
	//pinMode(36, INPUT);
	adc1_config_width(ADC_WIDTH);
	for (int i = 0; i < NUM_OF_SCT013_SENSORS; i++) {
		Serial.printf("[%s] [%d] Initializing Sensor %d... ", "initSCT013Sensors", millis(), i);
		adc1_config_channel_atten(SCT013SensorChannels[i], ADC_ATTEN_11db);
		Serial.printf("OK\n");
	}
	Serial.printf("[%s] [%d] SCT013 sensors initialization COMPLETE\n", "initSCT013Sensors", millis());
}

void initWiFi() {
	int startTime = millis();
	Serial.printf("[%s] [%d] Initializing WiFi Connection: ", "initWifi", millis());
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(50);
		Serial.print(".");
	}
	Serial.println(" OK");
	host = WiFi.gatewayIP().toString();
	Serial.printf("[%s] [%d] Gateway IP is: %s\n", "initWiFi", millis(), host);
	int deltaTime = millis() - startTime;
	Serial.printf("[%s] [%d] Wifi initialization took %d ms\n", "initWiFi", millis(), deltaTime);
}

void initWebSocket() {
	int startTime = millis();
	Serial.printf("[%s] [%d] Initializing WebSocket connection ... ", "initWebSocket", millis());
	webSocketClient.begin(host, WEBSOCKET_PORT, url);
	delay(1000);
	webSocketClient.onEvent(webSocketEvent);
	int deltaTime = millis() - startTime;
	Serial.printf("[%s] [%d] WebSocket initialization took %d ms\n", "initWebSocket", millis(), deltaTime);
	webSocketClient.setReconnectInterval(1000);
}

// the loop function runs over and over again until power down or reset
void loop() {
	getReadings();
	webSocketClient.loop();
}

void checkWiFi() {
	if (WiFi.status() == WL_DISCONNECTED) {
		Serial.printf("[%s] [%d] WiFi Connection LOST, rebooting.", "checkWiFi", millis());
		ESP.restart();
	}
	else {
		Serial.printf("[%s] [%d] WiFi Connection OK\n", "checkWiFi", millis());
		return;
	}
}



void printAverage() {
	for (int i = 0; i < NUM_OF_SCT013_SENSORS; i++)
		Serial.printf("Average: %d\n", average[i]);
}

unsigned int previousSocketSendTime = 0;
unsigned int socketSendTime = 2000;
void getReadings() {
	if ((millis() - previousSocketSendTime) > socketSendTime) {
		checkWiFi();
		Serial.println("------------------------------------------------------");
		Serial.println("Getting readings");
		for (int sensor = 0; sensor < NUM_OF_SCT013_SENSORS; sensor++) {
			int delta = 0;
			int count = 0;
			Serial.printf("Getting readings from Sensor %d\n", sensor + 1);
			do {
				int min_reading = INT_MAX;
				int max_reading = INT_MIN;
				for (int reading = 0; reading < NUM_OF_READINGS; reading++) {
					int val = adc1_get_raw(SCT013SensorChannels[sensor]);
					readings[sensor][reading] = val;
					if (val < min_reading)
						min_reading = val;
					if (val > max_reading)
						max_reading = val;

					//Serial.printf("%d. %d\n", reading, val);
				}
				delta = max_reading - min_reading;
				Serial.printf("Min Reading: %d\n", min_reading);
				Serial.printf("Max Reading: %d\n", max_reading);
				Serial.printf("Delta: %d\n", delta);
				Serial.println("---------------------------------");
				count++;
			} while (delta > MAX_DELTA && count < MAX_NUM_OF_TRIES);
			if (delta > MAX_DELTA)
				Serial.println("Sensor reading failed, max number of tries exeded");
			else
				Serial.printf("[%s] [%d] OK\n", "getReadings", millis());
		}
		getAverage();
		handleWebsocket();
	}
}

void getAverage() {
	for (int sensor = 0; sensor < NUM_OF_SCT013_SENSORS; sensor++) {
		int sum = 0;
		for (int reading = 0; reading < NUM_OF_READINGS; reading++) {
			sum += readings[sensor][reading];
			//Serial.printf("readings[sensor][reading]: %d\n", readings[sensor][reading]);
		}
		sum /= NUM_OF_READINGS;
		average[sensor] = sum;
		//Serial.printf("sum: %d\n", sum);
	}
	printAverage();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	//Serial.print("Trying to open a WebSocket .");
	//while (type != WStype_CONNECTED) {
	//	Serial.print(".");
	//	delay(100);
	//}
	//Serial.println(" OK");
	switch (type) {
	case WStype_DISCONNECTED:
		Serial.printf("[WebSocket] Disconnected!\n");
		break;
	case WStype_CONNECTED:
		Serial.printf("[WebSocket] Connected to url: %s\n", payload);
		webSocketClient.sendTXT("Connected");
		break;
	case WStype_TEXT:
		Serial.printf("[WebSocket] get text: %s\n", payload);
		// webSocket.sendTXT("message here");
		break;
	}
}

void handleWebsocket() {
	for (int i = 0; i < NUM_OF_SCT013_SENSORS; i++) {
		Serial.printf("[%s] [%d] Sending sensor %d data\n", "handleWebSocket", millis(), i);
		String temp = String(i) + ':' + String(average[i]);
		webSocketClient.sendTXT(temp);
		Serial.printf("[%s] [%d] OK\n", "handleWebSocket", millis());
	}
	previousSocketSendTime = millis();
	Serial.printf("[%s] [%d] Sending OK flag\n", "handleWebSocket", millis());
	webSocketClient.sendTXT("OK");
}