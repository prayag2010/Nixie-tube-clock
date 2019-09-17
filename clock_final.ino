#include <SoftPWM.h>
#include "nixie_config.h"
#include <Adafruit_NeoPixel.h>
#include <DS3231.h>
#include <Wire.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, leds, NEO_GRB + NEO_KHZ800);

int currentDigits[6] = {9, 0, 3, 4, 5, 6}; //digits ssmmhh

long output = 0;

DS3231 Clock;
bool Century = false;
bool h12 = true;
bool PM;

int currentHour = 25, previousHour;

unsigned long currentTime;
unsigned long previousTime = 0;
unsigned long dateTimeoutMillis = 0;

unsigned long bulbFadeMillis;
int prevTime[3] = {99, 99, 99};

boolean startSlot = true;
int slotEffectDigit = 0;
int slotEffectCounter = 0;
int slotEffectPosition = 0;

boolean displayDate = false;

int touchInput, prevTouchInput;

int brightnessLevel = 2;
int trueBrightness;

void setup() {

  Serial.begin(115200);
  Wire.begin(myAddress);
  Wire.onReceive (receiveEvent);

  setTouchIC();

  pixels.begin();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 10, 0));
    pixels.show();
    delay(100);
  }

  for (int i = 0; i < 6; i++) {
    pinMode(anodes[i], OUTPUT);
  }

  Palatis::SoftPWM.begin(100);
  Palatis::SoftPWM.printInterruptLoad();

  for (int i = 0; i < 6; i++) {
    pinMode(anodes[i], OUTPUT);
    //lower limit 130, upper limit 180
    //    analogWrite(anodes[i], 180);
  }

  pinMode(hvData, OUTPUT);
  pinMode(hvClock, OUTPUT);
  pinMode(hvStrobe, OUTPUT);

  pinMode(lvData, OUTPUT);
  pinMode(lvClock, OUTPUT);
  pinMode(lvLatch, OUTPUT);

  digitalWrite(lvLatch, LOW);
  shiftOut(lvData, lvClock, LSBFIRST, 0xFF);
  digitalWrite(lvLatch, HIGH);

  dateTimeoutMillis = millis();
}

void loop() {
  //  blank();
  //  delay(5);

  writeTime();
  checkSlotMachine();

  readTouchIC();

  if (millis() - dateTimeoutMillis <= 5000)
    displayDate = true;
  else
    displayDate = false;

  if (prevTime[0] != Clock.getSecond()) {
    bulbFadeMillis = millis();
    prevTime[0] = Clock.getSecond();
  }
  if (prevTime[1] != Clock.getMinute()) {
    bulbFadeMillis = millis();
    prevTime[1] = Clock.getMinute();
  }
  if (prevTime[2] != Clock.getHour(h12, PM)) {
    bulbFadeMillis = millis();
    prevTime[2] = Clock.getHour(h12, PM);
  }

  int temp = map( millis() - bulbFadeMillis, 0, 1000, 0, trueBrightness);

  if (prevTouchInput == 0) {
    for (int i = 0; i < 5; i++) {
      trueBrightness = map(brightnessLevel, 0, 5, 0, 80);
      Palatis::SoftPWM.set(i, trueBrightness);
    }
  }
  Palatis::SoftPWM.set(5, temp);
  Serial.println(temp);

  //  delay(1);
}

void readTouchIC()
{
  // Begin transmission to the touch sensor
  Wire.beginTransmission(touchICReg);
  Wire.write(touchICKeyStatus);
  Wire.endTransmission();

  Wire.requestFrom(touchICReg, 1);

  if (Wire.available() <= 1) {
    prevTouchInput = touchInput;
    touchInput = Wire.read();
    //    Serial.println(touchInput, HEX);
  }

  if (prevTouchInput == 0) {
    if (touchInput == 0x08) {
      Serial.println("KEY S TOUCHED");
      dateTimeoutMillis = millis();
    }
    if (touchInput == 0x20) {
      Serial.println("KEY + TOUCHED");
      brightnessLevel++;
      Serial.println("Brightness Level:");
      Serial.println(brightnessLevel);
    }
    if (touchInput == 0x10) {
      Serial.println("KEY - TOUCHED");
      brightnessLevel--;
      Serial.println("Brightness Level:");
      Serial.println(brightnessLevel);
    }

    brightnessLevel = constrain(brightnessLevel, 0, 5);

    if (prevTouchInput == 0) {
      //      Serial.println("Brightness Level:");
      //      Serial.println(brightnessLevel);

      for (int i = 0; i < 5; i++) {
        trueBrightness = map(brightnessLevel, 0, 5, 0, 80);
        Palatis::SoftPWM.set(i, trueBrightness);
      }
    }
  }
}

void setTouchIC()
{
  //Writing a random value to calibrate touch IC
  Wire.beginTransmission(touchICReg);
  Wire.write(touchICCal);//cal
  Wire.write(4);//random
  Wire.endTransmission();
  delay(5);

  //IC will recal itself if the input is detected for more than val*160ms
  Wire.beginTransmission(touchICReg);
  Wire.write(touchICMaxOn);//recal
  Wire.write(10);//val * 160mS
  Wire.endTransmission();
  delay(5);

  //adjacent key supression disable
  for (int i = 0; i < 8; i++) {
    Wire.beginTransmission(touchICReg);
    Wire.write(touchICAKS0 + i);//reg
    Wire.write(8 << 2);//disable
    Wire.endTransmission();
    delay(5);
  }

  Wire.endTransmission();
}

void checkSlotMachine()
{
  previousHour = currentHour;
  currentHour = Clock.getHour(h12, PM);
  if (previousHour != currentHour) {
    startSlot = true; //slot machine effect
  }

  currentTime = millis();

  if (currentTime - previousTime >= 100 && startSlot) {
    slotEffectDigit++;
    slotEffectCounter++;
    previousTime = currentTime;
  }

  if (slotEffectDigit > 9)
    slotEffectDigit = 0;

  if (slotEffectCounter > 30) {
    slotEffectPosition++;
    slotEffectCounter = 0;
  }

  if (slotEffectPosition > 2) {
    startSlot = false;
    slotEffectDigit = 0;
    slotEffectPosition = 0;
  }

  if (startSlot) {
    //    Serial.println("SlotEffectDigit:");
    //    Serial.println(slotEffectDigit);
    //    Serial.println("SlotEffectCounter:");
    Serial.println(slotEffectCounter);
    //    Serial.println("SlotEffectPosition:");
    //    Serial.println(slotEffectPosition);
  }
}

void writeTime()
{
  if (startSlot && slotEffectPosition == 0) {
    currentDigits[4] = currentDigits[5] = slotEffectDigit;
  } else {
    if (!displayDate) {
      currentDigits[4] = Clock.getHour(h12, PM) / 10;
      currentDigits[5] = Clock.getHour(h12, PM) % 10;
    } else {
      currentDigits[4] = Clock.getDate() / 10;
      currentDigits[5] = Clock.getDate() % 10;
    }
  }
  if (startSlot && slotEffectPosition == 1) {
    currentDigits[2] = currentDigits[3] = slotEffectDigit;
  } else {
    if (!displayDate) {
      currentDigits[2] = Clock.getMinute() / 10;
      currentDigits[3] = Clock.getMinute() % 10;
    } else {
      currentDigits[2] = Clock.getMonth(Century) / 10;
      currentDigits[3] = Clock.getMonth(Century) % 10;
    }
  }
  if (startSlot && slotEffectPosition == 2) {
    currentDigits[0] = currentDigits[1] = slotEffectDigit;
  } else {
    if (!displayDate) {
      currentDigits[0] = Clock.getSecond() / 10;
      currentDigits[1] = Clock.getSecond() % 10;
    } else {
      currentDigits[0] = Clock.getYear() / 10;
      currentDigits[1] = Clock.getYear() % 10;
    }
  }

  int m = 0;
  while (m <= 5) {
    prepareOutput(m, m + 1);
    displayDigits();
    m += 2;
  }
  digitalWrite(hvStrobe, HIGH);
  //  delay(5);
  digitalWrite(hvStrobe, LOW);
}


void prepareOutput(int a, int b)
{
  long output1 = 0;
  long output2 = 0;
  output = 0;
  int oddDigit = currentDigits[a];
  int evenDigit = currentDigits[b];

  int trueOddDigit = oddDigitMap[oddDigit][0];
  int trueEvenDigit = evenDigitMap[evenDigit][0];
  long num1 = numbers[trueOddDigit];
  long num2 = numbers[trueEvenDigit];

  //  Serial.print("Odd digit ");
  //  Serial.println(oddDigit, DEC);
  //  Serial.print("Even digit ");
  //  Serial.println(evenDigit, DEC);
  //  Serial.print("True odd digit ");
  //  Serial.println(trueOddDigit, DEC);
  //  Serial.print("True even digit ");
  //  Serial.println(trueEvenDigit, DEC);

  //  Serial.println("Num1 OP:");
  //  Serial.println(num1, BIN);
  if (oddDigitMap[oddDigit][1]) {
    //    Serial.println("Will Shift");
    output1 |= (num1 << 10) + 0x3FF;
  }
  else
    output1 |= num1 + 1047552;

  //  Serial.println("Odd OP:");
  //  Serial.println(output1, BIN);

  //  Serial.println("Num2 OP:");
  //  Serial.println(num2, BIN);
  if (evenDigitMap[evenDigit][1]) {
    //    Serial.println("Will Shift");
    output2 |= (num2 << 10) + 0x3FF;
  }
  else
    output2 |= num2 + 1047552; //10 1s with 10 0s

  output = output1 & output2;
  //  Serial.println("Odd and even OP:");
  //  Serial.println(output, BIN);
  //
  //  Serial.println();
  //  Serial.println("Final OUTPUT:");
  //  Serial.println(output, BIN);

  //  Serial.println(output & 0x3FF, BIN);
  //  Serial.println((output >> 10) & 0x3FF, BIN);
}


void displayDigits()
{
  for (int i = 0; i < 20; i++) {
    digitalWrite(hvData, (output >> i) & 1);
    //    delay(1);
    digitalWrite(hvClock, HIGH);
    //    delay(1);
    digitalWrite(hvClock, LOW);
    //    delay(1);
  }
}

void blank()
{
  for (int y = 0; y < 3; y++)
    for (int q = 0; q < 20; q++) {
      digitalWrite(hvData, (0xFFFFF >> q) & 1);
      delay(1);
      digitalWrite(hvClock, HIGH);
      delay(1);
      digitalWrite(hvClock, LOW);
      delay(1);
    }
}

void slotMachineEffectOld()
{
  int tempNumber = 0;
  for (int k = 0; k < 300; k++)
  {
    for (int l = 0; l < 6; l++)
      currentDigits[l] = tempNumber;

    int m = 0;
    while (m <= 5) {
      prepareOutput(m, m + 1);
      displayDigits();
      m += 2;
    }
    digitalWrite(hvStrobe, HIGH);
    delay(5);
    digitalWrite(hvStrobe, LOW);

    delay(100);

    tempNumber++;
    if (tempNumber > 9)
      tempNumber = 0;
  }
}

void receiveEvent (int howMany)
{
  for (int i = 0; i < howMany; i++)
  {
    char b = Wire.read ();
  }
}
