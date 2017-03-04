static const int BUFFER_SIZE = 256;         // bytes
static const long int SAMPLE_DURATION = 55; // microseconds

byte readData[BUFFER_SIZE];
volatile int iw = 0, ir = 0, ow = 0, diff = 0;
unsigned long int tsProbe = 0, tsNow = 0;

ISR (SPI_STC_vect) {
  // read from the data register
  readData[iw ++] = SPDR;
  if (iw >= BUFFER_SIZE) iw = 0;
  diff = (iw > ir) ? iw - ir : BUFFER_SIZE - ir + iw;
  SPDR = diff;
}

void setup() {
  // 
  Serial.begin(9600);
  // sets "slave out" as output
  pinMode(MISO, OUTPUT);
  // sets SPI in slave mode
  SPCR |= _BV(SPE);
  // turns on interrupts
  SPCR |= _BV(SPIE);
  
  // sets pin 3 as PWM output
  DDRD |= (1 << DDD3);
  TCCR2A |= (1 << COM2A1) | (1 << COM2B1);  // non-inverted PWM mode
  TCCR2A |= (1 << WGM22) | (1 << WGM20);    // fast PWM mode
  TCCR2B |= (1 << CS20);                    // no clock frequency reduction
  OCR2A = 255;        // the max count value, giving PWM period
  OCR2B = 0;          // the value to be modulated
}

void loop() {
  // 
  if (ir != iw) {
    // 
    Serial.print("Read index: ");
    Serial.print(ir);
    Serial.print(", Write index: ");
    Serial.print(iw);
    Serial.print(", Diff: ");
    Serial.println(diff);
    while (ir != iw) {
      // 
      tsNow = micros();
      if (tsNow <= SAMPLE_DURATION + tsProbe) {
        // 
        continue;
      }
      // 
      tsProbe = tsNow;
      OCR2B = readData[ir];
      ir ++;
      if (ir >= BUFFER_SIZE) ir = 0;
    }
    OCR2B = 0;
  }
}
