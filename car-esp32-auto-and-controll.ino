#import<Ultrasonic.h>

#include <PS4Controller.h>
#include <bitset>

#define ULTRA_TRIC  13
#define ULTRA_ECHO  12

#define LEFT_FRONT  27
#define LEFT_BACK   25
#define RIGHT_FRONT  33
#define RIGHT_BACK  32

#define LEFT_SPEED   -1
#define RIGHT_SPEED  -1

#define BUZZER      21

#define LED_MANUAL_X    4
#define LED_AUTO_PILLOT 16

enum Direction {STOP=0, GO_TO_FRONT=10, GO_TO_BACK=5, GO_TO_LEFT=9, GO_TO_RIGHT=6};
enum ControllType {MANUAL_BASIC, MANUAL_ACC, AUTO_PILLOT};

ControllType controllType = MANUAL_BASIC;

void setDirection(Direction direction){
  int directionInt = (int)direction;
  std::bitset<4> directionBits(directionInt);
  digitalWrite(LEFT_FRONT,  directionBits[3]);
  digitalWrite(LEFT_BACK,   directionBits[2]);
  digitalWrite(RIGHT_FRONT,  directionBits[1]);
  digitalWrite(RIGHT_BACK,  directionBits[0]);
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
}

void controllEvent() {

  if (PS4.event.button_down.options)
      setControllType(controllType != AUTO_PILLOT ? AUTO_PILLOT : MANUAL_BASIC);

  if (PS4.event.button_down.share && !auto_pillot)
    setControllType(controllType != MANUAL_ACC ? MANUAL_ACC : MANUAL_BASIC);
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

  int newValue = map(value, min, max, 0, 255);
  if (newValue > 255)
    newValue = 255;
  if (newValue < 0)
    newValue = 0;

  return newValue;
}

void controllLogicManualBasicAng(){

  int yLeft = PS4.LStickY();
  int yRight = PS4.RStickY();

  int leftAnalogValue = convertStickValue(yLeft, 0, 127);
  int rightAnalogValue = convertStickValue(yRight, 0, 127);

  digitalWrite(LEFT_FRONT,  yLeft > 0 ? HIGH : LOW);
  digitalWrite(LEFT_BACK,   yLeft < 0 ? HIGH : LOW);
  digitalWrite(RIGHT_FRONT,  yRight > 0 ? HIGH : LOW);
  digitalWrite(RIGHT_BACK,  yRight < 0 ? HIGH : LOW);

  //TODO: Ajuste Analogico
  //analogWrite(LEFT_SPEED,leftAnalogValue);
  //analogWrite(RIGHT_SPEED,leftAnalogValue);
}

void setup() {
  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(LEFT_BACK, OUTPUT);
  pinMode(RIGHT_FRONT, OUTPUT);
  pinMode(RIGHT_BACK, OUTPUT);
  pinMode(LED_MANUAL_X, OUTPUT);
  pinMode(LED_AUTO_PILLOT, OUTPUT);
  Serial.begin(9600);

  connectControll();
}

void loop() {
  if (PS4.isConnected()){
    switch(controllType){
      case AUTO_PILLOT:
        controllLogicAutoPillot();
        break;
      case MANUAL_BASIC:
        //controllLogicManualBasicAng();
        controllLogicManualBasicDigital();
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
