/*
 *  Arduino NiMH battery cell discharger.
 *  
 *  GameInstance.com
 *  2017
 */

static const byte VBAT_PROBE = A6;
static const byte VSHUNT_PROBE = A7;
static const byte DISCHARGE_PIN = 10;
static const byte LED_PIN = 13;

static const unsigned int ADC_RESOLUTION_DEFAULT = 1024;

static const float V_BAT_MIN = 1.0; // volts
static const float V_BAT_MAX = 1.45; // volts
static const float VCC = 5.00; // volts
static const float SHUNT_RESISTANCE = 1.8; // ohms

byte state = 0;
double v_bat = 0.0, v_shunt = 0.0;
bool LEDon = false;

double GetVoltage(unsigned int value) {
  // 
  return (double) value / (ADC_RESOLUTION_DEFAULT - 1) * VCC;
}

void setup() {
  // 
  digitalWrite(DISCHARGE_PIN, LOW);
  Serial.begin(9600);
}

void loop() {
  // 
  if (state == 0) {
    // waiting for battery
    v_bat = GetVoltage(analogRead(VBAT_PROBE));
    if (v_bat > V_BAT_MIN) {
      // a battery was connected
      if (v_bat > V_BAT_MAX) {
        // voltage too high, 
        // consistent with an alkaline cell
        // or a well charged NiMH - not needing discharge
        Serial.println("Invalid cell. Must be a partially discharged 1.2V NiMH cell.");
        state = 4;
      } else {
        // voltage consistent with a 1.2V NiMH cell
        state = 1;
        Serial.print("1.2V NiMH cell connected. Vbat = ");
        Serial.print(v_bat, 6);
        Serial.println(" V");
      }
    } else {
      // nothing connected yet
      Serial.print("No battery connected. Vbat = ");
      Serial.print(v_bat, 6);
      Serial.println(" V");
      delay(2000);
    }
  }
  if (state == 1) {
    // starting the discharge
    digitalWrite(DISCHARGE_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Discharge started.");
    delay(500);
    state = 2;
  }
  if (state == 2) {
    // 
    v_bat = GetVoltage(analogRead(VBAT_PROBE));
    if (v_bat < V_BAT_MIN) {
      // cell no longer needing discharge
      Serial.print("Discharge complete. Vbat=");
      Serial.print(v_bat);
      Serial.println(" V");
      state = 3;
    } else {
      // cell not yet discharged
      v_shunt = GetVoltage(analogRead(VSHUNT_PROBE));
      Serial.print(millis());
      Serial.print(" Vbat=");
      Serial.print(v_bat, 6);
      Serial.print(" V Idischarge=");
      Serial.print((v_bat - v_shunt) / SHUNT_RESISTANCE, 6);
      Serial.println(" A");
      delay(2000);
    }
  }
  if (state == 3) {
    // ending the discharge
    digitalWrite(DISCHARGE_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    state = 4;
  }
  if (state == 4) {
    // waiting for the battery to be removed
    v_bat = GetVoltage(analogRead(VBAT_PROBE));
    if (v_bat < V_BAT_MIN / 2) {
      // battery removed
      state = 0;
    } else {
      // battery not yet removed
      if (LEDon) {
        // 
        LEDon = false;
        digitalWrite(LED_PIN, LOW);
      } else {
        // 
        LEDon = true;
        digitalWrite(LED_PIN, HIGH);
      }
      delay(1000);
    }
  }
}
