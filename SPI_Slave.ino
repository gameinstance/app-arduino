byte readData[32];
char writeData[] = "Me, slave!";
byte sizeWD = sizeof(writeData);
volatile byte iw = 0, ir = 0, ow = 0;

void setup() {
	// 
	Serial.begin(9600);
	// sets "slave out" as output
	pinMode(MISO, OUTPUT);
	// sets SPI in slave mode
	SPCR |= _BV(SPE);
	// turns on interrupts
	SPCR |= _BV(SPIE);
}

void loop() {
	// 
	bool bData = false;
	if (ir != iw) {
		//
		bData = true;
		Serial.print("Received data: \"");
	}
	for (; ir != iw; ir ++) {
		//
		Serial.print((char)readData[ir]);
	}
	if (bData) {
		// 
		Serial.println("\"");
	}
	delay(250);
}

ISR (SPI_STC_vect) {
	// read from the data register
	readData[iw ++] = SPDR;
	// write onto the data register
	SPDR = writeData[ow ++];
	if (ow > sizeWD) {
		//
		ow = 0;
	}
}
