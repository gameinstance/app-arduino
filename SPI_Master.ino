#include <SPI.h>

SPISettings conf(4000000, MSBFIRST, SPI_MODE0);
char readData[32];
char writeData[] = "I'm master!";

void setup() {
	// 
	SPI.begin();
	Serial.begin(9600);
}

void loop() {
	// 
	SPI.beginTransaction(conf);
	digitalWrite(10, LOW);
	// slave setup time
	delayMicroseconds(10);
	byte i = 0;
	for (; i < sizeof(writeData); i ++) {
		// data exchange
		readData[i] = SPI.transfer(writeData[i]);
		// slave interrupt extra time
		delayMicroseconds(20);
	}
	digitalWrite(10, HIGH);
	SPI.endTransaction();
	Serial.print("Received: \"");
	for (byte j = 0; j < i; j ++) {
		// 
		Serial.print((char)readData[j]);
	}
	Serial.println("\"");
	delay(2000);
}
