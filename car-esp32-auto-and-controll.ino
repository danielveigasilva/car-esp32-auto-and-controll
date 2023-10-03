#import<Ultrasonic.h>
#include <PS4Controller.h>
#include <bitset>

#define ULTRA_TRIC  13
#define ULTRA_ECHO  12
#define LEFT_FRONT  27
#define LEFT_BACK   14
#define RIGH_FRONT  2
#define RIGHT_BACK  15

#define BUZZER      26

#define LED_CONTROLL 4
#define LED_AUTO_PILLOT 16

enum Direction {STOP=0, GO_TO_FRONT=10, GO_TO_BACK=5, GO_TO_LEFT=9, GO_TO_RIGHT=6};
bool auto_pillot = false;
bool controll_move = false;
unsigned long lastTimeStamp = 0;

void setDirection(Direction direction){
  int directionInt = (int)direction;
  std::bitset<4> directionBits(directionInt);
  digitalWrite(LEFT_FRONT,  directionBits[3]);
  digitalWrite(LEFT_BACK,   directionBits[2]);
  digitalWrite(RIGH_FRONT,  directionBits[1]);
  digitalWrite(RIGHT_BACK,  directionBits[0]);
}

void tryConnectControll(){
  PS4.attach(controllEvent);
  PS4.begin("D8:08:31:1F:F6:F5");
  bool state = HIGH;
  while(!PS4.isConnected()){
    setDirection(STOP);
    digitalWrite(LED_CONTROLL, state);
    state = !state;
    delay(250);
  }
  //soundConnect();
  digitalWrite(LED_CONTROLL, HIGH);
  PS4.setLed(100, 100, 0);
  PS4.sendToController();
}

void phrase1() {
    int k = random(1000,2000);
    for (int i = 0; i <=  random(100,2000); i++){
        tone(BUZZER, k+(-i*2));          
        delay(random(.9,2));             
    } 
    for (int i = 0; i <= random(100,1000); i++){
        tone(BUZZER, k + (i * 10));          
        delay(random(.9,2));             
    } 
}

void phrase2() {
    int k = random(1000,2000);
    for (int i = 0; i <= random(100,2000); i++){
        tone(BUZZER, k+(i*2));          
        delay(random(.9,2));             
    }    
    for (int i = 0; i <= random(100,1000); i++){  
        tone(BUZZER, k + (-i * 10));          
        delay(random(.9,2));             
    } 
}

void soundTurnOn() {
    int K = 2000;
    switch (random(1,7)) {
        case 1:phrase1(); break;
        case 2:phrase2(); break;
        case 3:phrase1(); phrase2(); break;
        case 4:phrase1(); phrase2(); phrase1();break;
        case 5:phrase1(); phrase2(); phrase1(); phrase2(); phrase1();break;
        case 6:phrase2(); phrase1(); phrase2(); break;
    }
    for (int i = 0; i <= random(3, 9); i++){
        tone(BUZZER, K + random(-1700, 2000));          
        delay(random(70, 170));   
        noTone(BUZZER);         
        delay(random(0, 30));             
    } 
    noTone(BUZZER);         
    delay(random(2000, 4000));             
}

void soundConnect(){
  tone(8,988,100);
  delay(100);
  tone(8,1319,850);
  delay(800);
  noTone(8);
}

void setup() {
  //soundTurnOn();
  delay(200);
  noTone(BUZZER);
  delay(200);
  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(LEFT_BACK, OUTPUT);
  pinMode(RIGH_FRONT, OUTPUT);
  pinMode(RIGHT_BACK, OUTPUT);
  pinMode(LED_CONTROLL, OUTPUT);
  pinMode(LED_AUTO_PILLOT, OUTPUT);
  Serial.begin(9600);

  tryConnectControll();
}

void controllEvent() {

  if (PS4.event.button_down.options){
    auto_pillot = !auto_pillot;
    if (auto_pillot){
      digitalWrite(LED_CONTROLL, LOW);
      digitalWrite(LED_AUTO_PILLOT, HIGH);
      PS4.setLed(0, 100, 0);
      PS4.sendToController();
    }
    else{
      setDirection(STOP);
      digitalWrite(LED_CONTROLL, HIGH);
      digitalWrite(LED_AUTO_PILLOT, LOW);
      PS4.setLed(100, 100, 0);
      PS4.sendToController();
    }
  }

  if (PS4.event.button_down.share && !auto_pillot){
    controll_move = !controll_move;
    if (controll_move){
      PS4.setLed(0, 0, 100);
      PS4.sendToController();
    }
    else{
      PS4.setLed(100, 100, 0);
      PS4.sendToController();
    }
  }
}

void loop() {
  if (PS4.isConnected()){
    if (auto_pillot){
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
    else{
      if (controll_move){
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
      else{
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
    }
  }
  else{
    setDirection(STOP);
    tryConnectControll();
  }

  delay(10);
}
