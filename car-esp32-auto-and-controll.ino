#import<Ultrasonic.h>

#include <WiFi.h>

#include <PS4Controller.h>
#include <bitset>
#include "pitches.h" 

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_ipc.h>

#include <iostream>
#include <cstdlib>

#include <sys/socket.h> 
#include <arpa/inet.h> 

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
#define LED_WIFI        2

enum Direction {STOP=0, GO_TO_FRONT=10, GO_TO_BACK=5, GO_TO_LEFT=9, GO_TO_RIGHT=6};
enum ControllType {MANUAL_BASIC, MANUAL_ACC, AUTO_PILLOT};

ControllType controllType = MANUAL_BASIC;
//int socketServer = -1;

TaskHandle_t taskSoundHandle = NULL;

void soundStarWars(void * args){
  
  int melody[] = {
    NOTE_AS4, NOTE_AS4, NOTE_AS4,
    NOTE_F5, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5, NOTE_C5,
    NOTE_F5, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,

    NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
    NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5,
    NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
    NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C5, NOTE_C5,
    NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,

    NOTE_C6, NOTE_G5, NOTE_G5, REST, NOTE_C5,
    NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
    NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C6, NOTE_C6,
    NOTE_F6, NOTE_DS6, NOTE_CS6, NOTE_C6, NOTE_AS5, NOTE_GS5, NOTE_G5, NOTE_F5,
    NOTE_C6
  };

  int durations[] = {
    8, 8, 8,
    2, 2,
    8, 8, 8, 2, 4,
    8, 8, 8, 2, 4,
    8, 8, 8, 2, 8, 8, 8,
    2, 2,
    8, 8, 8, 2, 4,

    8, 8, 8, 2, 4,
    8, 8, 8, 2, 8, 16,
    4, 8, 8, 8, 8, 8,
    8, 8, 8, 4, 8, 4, 8, 16,
    4, 8, 8, 8, 8, 8,

    8, 16, 2, 8, 8,
    4, 8, 8, 8, 8, 8,
    8, 8, 8, 4, 8, 4, 8, 16,
    4, 8, 4, 8, 4, 8, 4, 8,
    1
  };


  int size = sizeof(durations) / sizeof(int);
  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note];
    ledcWrite(BUZZER_CHANNEL, duration);
    tone(BUZZER, melody[note], duration);
    int pauseBetweenNotes = duration * 1.30;
    
    vTaskDelay(pauseBetweenNotes / portTICK_PERIOD_MS);
    //delay(pauseBetweenNotes);
    
    ledcWrite(BUZZER_CHANNEL, 0);
  }

  taskSoundHandle = NULL;
  vTaskDelete(taskSoundHandle);
}

void soundConnectControll(void * args){
  ledcWrite(BUZZER_CHANNEL, 125);
  tone(BUZZER,NOTE_B5);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  //delay(100);
  tone(BUZZER,NOTE_E6);
  vTaskDelay(800 / portTICK_PERIOD_MS);
  //delay(800);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  taskSoundHandle = NULL;
  vTaskDelete(taskSoundHandle);
}

void soundTurnOn(void * args){
  ledcWrite(BUZZER_CHANNEL, 125);
  tone(BUZZER,NOTE_E6);
  vTaskDelay(130 / portTICK_PERIOD_MS);
  //delay(130);
  tone(BUZZER,NOTE_G6);
  vTaskDelay(130 / portTICK_PERIOD_MS);
  //delay(130);
  tone(BUZZER,NOTE_E7);
  vTaskDelay(130 / portTICK_PERIOD_MS);
  //delay(130);
  tone(BUZZER,NOTE_C7);
  vTaskDelay(130 / portTICK_PERIOD_MS);
  //delay(130);
  tone(BUZZER,NOTE_D7);
  vTaskDelay(130 / portTICK_PERIOD_MS);
  //delay(130);
  tone(BUZZER,NOTE_G7);
  vTaskDelay(125 / portTICK_PERIOD_MS);
  //delay(125);
  ledcWrite(BUZZER_CHANNEL, 0);
  
  taskSoundHandle = NULL;
  vTaskDelete(taskSoundHandle);
}

void stopSound(){
  if (taskSoundHandle != NULL){
    vTaskDelete(taskSoundHandle);
    taskSoundHandle = NULL;
  }
}

void play(TaskFunction_t sound){
  srand((unsigned) time(NULL));
	int randomId = rand();

  stopSound();

  xTaskCreate(
        sound,
        ("sound" + std::to_string(randomId)).c_str(),
        4096,
        NULL,
        1,
        &taskSoundHandle
      );
}

void setDirection(Direction direction, int speed = 255){
  int directionInt = (int)direction;
  std::bitset<4> directionBits(directionInt);
  analogWrite(LEFT_FRONT,  directionBits[3] == 1 ? speed : 0);
  analogWrite(LEFT_BACK,   directionBits[2] == 1 ? speed : 0);
  analogWrite(RIGHT_FRONT, directionBits[1] == 1 ? speed : 0);
  analogWrite(RIGHT_BACK,  directionBits[0] == 1 ? speed : 0);
  /*
  if (socketServer < 0)
    connectSocket();
  else
    sendInt(direction, socketServer);
  */
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
/*
void sendInt( int value, int socket ){
    send(socket, &value, sizeof(int), 0);    
}
*/
/*
void connectWifi(){
  const char* SSID = "";
  const char* PASSWORD = "";
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  bool state = HIGH;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_WIFI, state);
    state = !state;
    delay(250);
  }

  digitalWrite(LED_WIFI, HIGH);
  Serial.println(WiFi.localIP());
}
*/
/*
void connectSocket(){

  struct sockaddr_in dest;
  socketServer = socket(AF_INET, SOCK_STREAM, 0);

  memset(&dest, 0, sizeof(dest));                
  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = inet_addr("192.168.1.18"); 
  dest.sin_port = htons(12345); 

  int connectResult = connect(socketServer, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
  if( connectResult == - 1 ){
    socketServer = -1;
    Serial.println(F("Falha ao Conectado Socket"));
  }
  else
    Serial.println(F("Socket Conectado"));
}
*/
void controllEvent() {

  if (PS4.event.button_down.options)
      setControllType(controllType != AUTO_PILLOT ? AUTO_PILLOT : MANUAL_BASIC);

  if (PS4.event.button_down.share)
    setControllType(controllType != MANUAL_ACC ? MANUAL_ACC : MANUAL_BASIC);

  if (PS4.event.button_down.triangle)
    play(soundStarWars);
  
  if (PS4.event.button_down.circle)
    stopSound();
}

int convertStickValue(int value, int min, int max){

  if (value < 0)
    value *= -1;

  int x = map(value, min, max, 0, 16);
  int newValue = pow(x,2) + 20;
  if (newValue > 255)
    newValue = 255;
  if (newValue <= 30)
    newValue = 0;

  return newValue;
}

void controllLogicAutoPillot(){
  Ultrasonic ultrasonic(ULTRA_TRIC, ULTRA_ECHO);
  float distancia = ultrasonic.read(CM);
  Serial.println(distancia);

  int speed = distancia <= 100 ? map(distancia, 0, 100, 90, 255) : 255;
  if(distancia <= 30)
  {
    setDirection(STOP);
    delay(300);
    setDirection(GO_TO_BACK);
    delay(300);
    setDirection(GO_TO_RIGHT);
    delay(500);
    float rightSpace = ultrasonic.read(CM);
    setDirection(GO_TO_LEFT);
    delay(1000);
    float leftSpace = ultrasonic.read(CM);
    if (leftSpace < rightSpace){
      setDirection(GO_TO_RIGHT);
      delay(1000);
    }
  }
  else
    setDirection(GO_TO_FRONT, speed);
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
    setDirection(GO_TO_RIGHT);
  else if (PS4.AccX() <= -1000)
    setDirection(GO_TO_LEFT);
  else if (PS4.AccY() >= 6000)
    setDirection(GO_TO_FRONT);
  else if (PS4.AccY() <= -300)
    setDirection(GO_TO_BACK);
  else
    setDirection(STOP);
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

  PS4.setRumble(0, (leftAnalogValue + rightAnalogValue)/2);
  PS4.sendToController();
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
  play(soundConnectControll);
  //effectConnectControll();
}

void setup() {
  Serial.begin(9600);
  
  pinMode(LEFT_FRONT, OUTPUT);
  pinMode(LEFT_BACK, OUTPUT);
  pinMode(RIGHT_FRONT, OUTPUT);
  pinMode(RIGHT_BACK, OUTPUT);
  pinMode(LED_MANUAL_X, OUTPUT);
  pinMode(LED_AUTO_PILLOT, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER, BUZZER_CHANNEL);
  play(soundTurnOn);
  //effectTurnOn();

  //connectWifi();
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
