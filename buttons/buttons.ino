#include <EEPROM.h>
#include <LiquidCrystal.h>
#define VALUES_COUNT 100
uint8_t values[VALUES_COUNT];
byte score = 0;
int eepromAddress = 0;

// Pins
//////////////////////////////////////////////
// Button pins
const int button1Pin = 9; // Change pins later
const int button2Pin = 10;
const int button3Pin = 11;
const int button4Pin = 12;
const int buttonsPin = 2;

// led pins
const int led1Pin = 3;
const int led2Pin = 4;
const int led3Pin = 5;
const int led4Pin = 6;

enum State { IDLE, READY, GAME };
State state = IDLE;
volatile byte ledState = HIGH;

// LCD pins
const int rs = A1, en = A2, d4 = A3, d5 = A4, d6 = A5, d7 = A6;
//////////////////////////////////////////////
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int currentValue = 0;
int currentLed = 0;
int idleanimation = 0;

void saveScore() {

  byte savedScore;
  EEPROM.get(eepromAddress, savedScore);
  if (score > savedScore){
  EEPROM.update(eepromAddress, score);
  }
}

void retrieveScore() {
  byte savedScore;
  EEPROM.get(eepromAddress, savedScore);
  Serial.println("current record: " + savedScore);
}

void scoreUp() {
  ++score;
  printScore();
  //saveScore();
}

void printScore() {
  Serial.print("Score: ");
  Serial.println(score);
  lcd.setCursor(0, 0);
  lcd.print("Score: ");
  lcd.setCursor(7, 0);
  lcd.print(score);
}

void initTimer(){  
  // initialize timer1 
  pinMode(LED_BUILTIN, OUTPUT);
  cli();           // disable all interrupts
  TCCR1A = 0x00;
  TCCR1B = 0x00;

  TCNT1 = 0x0000;            
  OCR1A = 16000;
  TCCR1B |= (1 << WGM12);   //CTC-toimintamoodin valinta eli TCNT nollataan, kun 16000 löytyy
  TCCR1B |= (1 << CS12) | (1 << CS10);    // 1024 prescaler   
  TIFR1 |= (1 << OCF1A);
  TIMSK1 |= (1 << OCIE1A);   // enable interrupt
  sei();             // enable all interrupts
}

bool checkCorrect() {
  /*switch (expected) {
    case 0: return b1 && !(b2 || b3 || b4);
    case 1: return b2 && !(b1 || b3 || b4);
    case 2: return b3 && !(b1 || b2 || b4);
    case 3: return b4 && !(b1 || b2 || b3);
  }*/
  uint8_t value = values[currentValue];
  int expectedButton;

  switch (value) {
    case 0: expectedButton = button1Pin; break;
    case 1: expectedButton = button2Pin; break;
    case 2: expectedButton = button3Pin; break;
    case 3: expectedButton = button4Pin; break;
  }
  return digitalRead(expectedButton);
}
ISR(TIMER1_COMPA_vect)
{
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState);
  updateLedISR();
}

void updateLedISR(){
  if (state != GAME) {
    return;
  }
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);
  digitalWrite(led4Pin, LOW);
  uint8_t nextLed = values[currentLed];
  switch (nextLed) {
    case 0: 
    digitalWrite(led1Pin, HIGH);
    break;
    case 1: 
    digitalWrite(led2Pin, HIGH);
    break;
    case 2: 
    digitalWrite(led3Pin, HIGH);
    break;
    case 3: 
    digitalWrite(led4Pin, HIGH);
    break;
  }
  ++currentLed;
  if (currentLed > VALUES_COUNT - 1) currentLed = 0;
}

void buttonPressISR() {
  if (state == IDLE) {
    state=READY;
    return;
  }
  if (state == READY) {
    state=GAME;
    return;
  }
  if (checkCorrect()) {
    scoreUp();
    Serial.println("Correct Button pressed");
  } else {
    Serial.println("peli ohi");
    printScore();
    state = IDLE;
  }
}

void setup() {
  pinMode(buttonsPin, INPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  pinMode(button4Pin, INPUT);

  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);
  score = 0;
  // Initialize random values
  for (int i = 0; i < VALUES_COUNT; ++i) {
    uint8_t val = random(0, 4);
    if (i > 0) {
      do {
      val = random(0, 4);
      } while (val == values[i - 1]);
    }
    values[i] = val;
  }
  retrieveScore();

  attachInterrupt(digitalPinToInterrupt(buttonsPin), buttonPressISR, RISING);
  Serial.begin(9600);
  lcd.begin(16, 2);
  printScore();
  initTimer();
}

void loop() {

}
