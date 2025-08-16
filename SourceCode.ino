
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x20, 16, 2);

// Pins
const int PIN_TMP36 = A0;   // Temperature sensor
const int PIN_TRIG  = 9;    // Ultrasonic Trigger
const int PIN_ECHO  = 8;    // Ultrasonic Echo
const int PIN_PIR   = 7;    // PIR sensor
const int PIN_BUZZ  = 6;    // Buzzer
const int PIN_BP    = A2;   // Potentiometer for BP simulation

// Thresholds
float FEVER_C = 37.5;     // fever threshold
int   MIN_DIST_CM = 60;   // social distance threshold

// ----- Functions -----
long measureDistanceCm() {
  digitalWrite(PIN_TRIG, LOW);  
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); 
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000); 
  if (duration == 0) return -1; // out of range/no echo
  return duration * 0.0343 / 2; // cm
}

float readTempC_TMP36() {
  int raw = analogRead(PIN_TMP36);
  float voltage = raw * (5.0 / 1023.0);
  return (voltage - 0.5) * 100.0;
}

void buzz(int ms) {
  tone(PIN_BUZZ, 2000);   
  delay(ms);
  noTone(PIN_BUZZ);
}

// ----- Setup -----
void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);  
  pinMode(PIN_BUZZ, OUTPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  // Random seed for heart rate simulation
  randomSeed(millis());

  // Welcome screen
  lcd.setCursor(0,0); lcd.print(" Smart Health Sys ");
  lcd.setCursor(0,1); lcd.print(" Initializing...  ");
  delay(2000);
  lcd.clear();
}

// ----- Loop -----
void loop() {
  float tC = readTempC_TMP36();
  long dcm = measureDistanceCm();
  int motion = digitalRead(PIN_PIR);

  // --- Heart Rate Simulation ---
  int heartRate = random(70, 101); // BPM between 70–100

  // --- Blood Pressure from Potentiometer ---
  int potVal = analogRead(PIN_BP);
  int systolic = map(potVal, 0, 1023, 90, 140);
  int diastolic = map(potVal, 0, 1023, 60, 90);

  bool fever = (tC >= FEVER_C);
  bool tooClose = (dcm != -1 && dcm < MIN_DIST_CM);
  bool alert = fever || tooClose || (motion == HIGH);

  // ----- LCD Display -----
  // First row: Temp + Distance
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(tC,1);
  lcd.print((char)223); // degree symbol
  lcd.print("C ");

  if (dcm == -1) { 
    lcd.print("D:--cm"); 
  } else { 
    lcd.print("D:");
    lcd.print(dcm);
    lcd.print("cm"); 
  }

  // Second row: HR + BP + Motion
  lcd.setCursor(0,1);
  lcd.print("HR:");
  lcd.print(heartRate);
  lcd.print(" BP:");
  lcd.print(systolic);
  lcd.print("/");
  lcd.print(diastolic);

  // Motion indicator at last position
  lcd.setCursor(15,1);
  if (motion) lcd.print("M");
  else lcd.print(" ");

  // ----- Buzzer -----
  if (alert) buzz(200);

  // ----- Serial Debug -----
  Serial.print("Temp: "); Serial.print(tC); Serial.print("C | ");
  Serial.print("Dist: ");
  if (dcm == -1) Serial.print("--");
  else Serial.print(dcm);
  Serial.print("cm | PIR: ");
  Serial.print(motion ? "MOTION" : "NO");
  Serial.print(" | HR: "); Serial.print(heartRate);
  Serial.print(" | BP: ");
  Serial.print(systolic); Serial.print("/"); Serial.println(diastolic);

  delay(1000); 
}
