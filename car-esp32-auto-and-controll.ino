#import<Ultrasonic.h>

#include <PS4Controller.h>
#include <bitset>
#include "pitches.h" 

#define ULTRA_TRIC  13
#define ULTRA_ECHO  12

#define LEFT_FRONT  27
#define LEFT_BACK   25
#define RIGHT_FRONT  33
#define RIGHT_BACK  32

#define BUZZER      21
#define BUZZER_CHANNEL 0

#define LED_MANUAL_X    4
#define LED_AUTO_PILLOT 16

enum Direction {STOP=0, GO_TO_FRONT=10, GO_TO_BACK=5, GO_TO_LEFT=9, GO_TO_RIGHT=6};
enum ControllType {MANUAL_BASIC, MANUAL_ACC, AUTO_PILLOT};

ControllType controllType = MANUAL_BASIC;

void soundStarWars(){
  int melody[] = {
    NOTE_AS4, NOTE_AS4, NOTE_AS4,
    NOTE_F5, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5
  };

  int durations[] = {
    8, 8, 8,
    2, 2,
    8, 8, 8, 2, 4,
    8, 8, 8, 2, 4,
    8, 8, 8, 2
  };

  int size = sizeof(durations) / sizeof(int);
  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note];
    ledcWrite(BUZZER_CHANNEL, duration);
    tone(BUZZER, melody[note], duration);
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);
    ledcWrite(BUZZER_CHANNEL, 0);
  }
}

void soundConnectControll(){
  ledcWrite(BUZZER_CHANNEL, 125);
  tone(BUZZER,NOTE_B5);
  delay(100);
  tone(BUZZER,NOTE_E6);
  delay(800);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void soundTurnOn(){
  ledcWrite(BUZZER_CHANNEL, 125);
  tone(BUZZER,NOTE_E6);
  delay(130);
  tone(BUZZER,NOTE_G6);
  delay(130);
  tone(BUZZER,NOTE_E7);
  delay(130);
  tone(BUZZER,NOTE_C7);
  delay(130);
  tone(BUZZER,NOTE_D7);
  delay(130);
  tone(BUZZER,NOTE_G7);
  delay(125);
  ledcWrite(BUZZER_CHANNEL, 0);
}

void setDirection(Direction direction){
  int directionInt = (int)direction;
  std::bitset<4> directionBits(directionInt);
  analogWrite(LEFT_FRONT,  directionBits[3] == 1 ? 255 : 0);
  analogWrite(LEFT_BACK,   directionBits[2] == 1 ? 255 : 0);
  analogWrite(RIGHT_FRONT,  directionBits[1] == 1 ? 255 : 0);
  analogWrite(RIGHT_BACK,  directionBits[0] == 1 ? 255 : 0);
}

void setControllType(ControllType newType){
  
  controllType = newType;

  switch(controllType){
    case AUTO_PILLOT:
      digitalWrite(LED_MANUAL_X, LOW);
      digitalWrite(LED_AUTO_PILLOT, HIGH);
      PS4.setLed(0, 100, 0);
      PS4.sendToController();
      break;
    case MANUAL_BASIC:
      setDirection(STOP);
      digitalWrite(LED_MANUAL_X, HIGH);
      digitalWrite(LED_AUTO_PILLOT, LOW);
      PS4.setLed(100, 100, 0);
      PS4.sendToController();
      break;
    case MANUAL_ACC:
      PS4.setLed(0, 0, 100);
      PS4.sendToController();
      break;
  }
}

void connectControll(){
  
  PS4.attach(controllEvent);
  PS4.begin("D8:08:31:1F:F6:F5");
  
  bool state = HIGH;
  while(!PS4.isConnected()){
    setDirection(STOP);
    digitalWrite(LED_MANUAL_X, state);
    state = !state;
    delay(250);
  }
  setControllType(MANUAL_BASIC);
  soundConnectControll();
}

void controllEvent() {

  if (PS4.event.button_down.options)
      setControllType(controllType != AUTO_PILLOT ? AUTO_PILLOT : MANUAL_BASIC);

  if (PS4.event.button_down.share)
    setControllType(controllType != MANUAL_ACC ? MANUAL_ACC : MANUAL_BASIC);
  
  if (PS4.event.button_down.square)
    soundTurnOn();
  
  if (PS4.event.button_down.triangle)
    soundStarWars();
}

void controllLogicAutoPillot(){
  Ultrasonic ultrasonic(ULTRA_TRIC, ULTRA_ECHO);
  float distancia = ultrasonic.read(CM);
  Serial.println(distancia);

  if(distancia <= 30)
  {
    setDirection(STOP);
    delay(300);
    setDirection(GO_TO_RIGHT);
    delay(100);
    setDirection(GO_TO_LEFT);
    delay(100);
    setDirection(GO_TO_BACK);
    delay(300);
    setDirection(GO_TO_LEFT);
    delay(200);
  }
  else
    setDirection(GO_TO_FRONT);
}

void controllLogicManualBasicDigital(){
  if (PS4.LStickX() >= 30)
    setDirection(GO_TO_RIGHT);
  else if (PS4.LStickX() <= -30)
    setDirection(GO_TO_LEFT);
  else if (PS4.L2() || PS4.LStickY() >= 30)
    setDirection(GO_TO_BACK);
  else if (PS4.R2() || PS4.LStickY() <= -30)
    setDirection(GO_TO_FRONT);
  else
    setDirection(STOP);
}

void controllLogicManualAcc(){
  if (PS4.L2() && PS4.R2())
    setDirection(STOP);
  else if (PS4.AccX() >= 1000)
    setDirection(GO_TO_LEFT);
  else if (PS4.AccX() <= -1000)
    setDirection(GO_TO_RIGHT);
  else if (PS4.AccY() >= 6000)
    setDirection(GO_TO_FRONT);
  else if (PS4.AccY() <= -300)
    setDirection(GO_TO_BACK);
  else
    setDirection(STOP);
}

int convertStickValue(int value, int min, int max){

  if (value < 0)
    value *= -1;

  int newValue = map(value, min, max, 50, 255);
  if (newValue > 255)
    newValue = 255;
  if (newValue < 80)
    newValue = 0;

  return newValue;
}

void controllLogicManualBasicAng(){

  int yLeft = PS4.LStickY();
  int yRight = PS4.RStickY();

  int leftAnalogValue = convertStickValue(yLeft, 0, 127);
  int rightAnalogValue = convertStickValue(yRight, 0, 127);

  analogWrite(LEFT_FRONT,   yLeft > 0   ? leftAnalogValue   : 0);
  analogWrite(LEFT_BACK,    yLeft < 0   ? leftAnalogValue   : 0);
  analogWrite(RIGHT_FRONT,  yRight > 0  ? rightAnalogValue  : 0);
  analogWrite(RIGHT_BACK,   yRight < 0  ? rightAnalogValue  : 0);
}

void setup() {
  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(LEFT_BACK, OUTPUT);
  pinMode(RIGHT_FRONT, OUTPUT);
  pinMode(RIGHT_BACK, OUTPUT);
  pinMode(LED_MANUAL_X, OUTPUT);
  pinMode(LED_AUTO_PILLOT, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER, BUZZER_CHANNEL);
  soundTurnOn();

  connectControll();
}

void loop() {
  if (PS4.isConnected()){
    switch(controllType){
      case AUTO_PILLOT:
        controllLogicAutoPillot();
        break;
      case MANUAL_BASIC:
        controllLogicManualBasicAng();
        //controllLogicManualBasicDigital();
        break;
      case MANUAL_ACC:
        controllLogicManualAcc();
        break;
    }
  }
  else{
    setDirection(STOP);
    connectControll();
  }

  delay(10);
}
