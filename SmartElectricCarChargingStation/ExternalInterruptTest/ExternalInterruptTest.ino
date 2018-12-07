/*
 Name:		ExternalInterruptTest.ino
 Created:	12/6/2018 7:54:29 PM
 Author:	Denis
*/
#define INTERRUPT_PIN 2

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	Serial.println("Monitoring interrupts: ");
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), increaseCounter, RISING);
}

// the loop function runs over and over again until power down or reset
void loop() {
	//Serial.println(digitalRead(INTERRUPT_PIN));
	//delay(10);
}

void increaseCounter() {
	Serial.println("INTERRUPT detected");
}