#include <Arduino.h>
#include "PS2X_lib.h"  //for v1.6

#define PS2_DAT        A3  //14    
#define PS2_CMD        A2  //15
#define PS2_SEL        A1  //16
#define PS2_CLK        A0  //17

/*
Motor Mapping:

  M1     M2
    ROBOT
  M4     M3

*/
#define M1_A 5
#define M1_B 6
#define M2_A 9
#define M2_B 10
#define M3_A 7
#define M3_B 8
#define M4_A 12
#define M4_B 13

PS2X ps2x; // create PS2 Controller Class
int error = 0;
byte type = 0;
byte vibrate = 0;

unsigned long time = 0;
int counter = 0;
int spdM1, spdM2, spdM3, spdM4;
int mtspd = 140;

#define pressures   true
//#define pressures   false
#define rumble      true
//#define rumble      false

int configJoystick(){
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
	if (pressures)
	  Serial.println("true ");
	else
	  Serial.println("false");
	Serial.print("rumble = ");
	if (rumble)
	  Serial.println("true)");
	else
	  Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
//  Serial.print(ps2x.Analog(1), HEX);
  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
	case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }
   return error;
}

void motorSpeed(int s1, int s2, int s3, int s4) {
  
  spdM1 = s1;
  spdM2 = s2 ;
  spdM3 = s3;
  spdM4 = s4;
  
  // limit max and min value for each motor
  spdM1 = constrain(spdM1,-255,255);
  spdM2 = constrain(spdM2,-255,255);
  spdM3 = constrain(spdM3,-255,255);
  spdM4 = constrain(spdM4,-255,255);
  
  /*
  Serial.print(" ,M1=");  Serial.print(spdM1);
  Serial.print(" ,M2=");  Serial.print(spdM2);
  Serial.print(" ,M3=");  Serial.print(spdM3);
  Serial.print(" ,M4=");  Serial.println(spdM4);
  */
}

void setup() {
  pinMode(M1_A, OUTPUT);
  pinMode(M1_B, OUTPUT);
  pinMode(M2_A, OUTPUT);
  pinMode(M2_B, OUTPUT);
  pinMode(M3_A, OUTPUT);
  pinMode(M3_B, OUTPUT);
  pinMode(M4_A, OUTPUT);
  pinMode(M4_B, OUTPUT);

  Serial.begin(115200);
  Serial.println("KittenBot Omini Robot");
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = 1;
  pinMode(13,OUTPUT);
  while(error){
    delay(100);
    error = configJoystick();
    digitalWrite(13, !digitalRead(13));  
  }
  
}

char buf[64];
int8_t bufindex;
int lx,ly,rx,ry;

void readJoystick(){
  uint8_t dir = 0;
  ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
  vibrate = ps2x.Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button
  
  lx = ps2x.Analog(PSS_LX)-128;
  ly = -(ps2x.Analog(PSS_LY)-127);
  rx = ps2x.Analog(PSS_RX)-128;
  ry = -(ps2x.Analog(PSS_RY)-127);
  
  if(abs(lx) < 100)  lx = 0;
  if(abs(ly) < 100)  ly = 0;
  if(abs(rx) < 100)  rx = 0;
  if(abs(ry) < 100)  ry = 0;
  
  if(ly > 100)  dir|=(1<<0);
  if(ly < -100)  dir|=(1<<1);
  if(lx < -100)  dir|=(1<<2);
  if(lx > 100)  dir|=(1<<3);
  
  if(rx < -100)  dir=(1<<4);
  if(rx > 100)  dir=(1<<5);
  if(ry < -100)  mtspd+=10;
  if(ry > 100)  mtspd-=10;
  
  if(dir == 0)
  {
    if(ps2x.Button(PSB_PAD_UP))  dir|=(1<<0);
    if(ps2x.Button(PSB_PAD_DOWN))  dir|=(1<<1);
    if(ps2x.Button(PSB_PAD_LEFT))  dir|=(1<<2);
    if(ps2x.Button(PSB_PAD_RIGHT))  dir|=(1<<3);
  }
  if(ps2x.Button(PSB_PINK))  dir=(1<<4);
  if(ps2x.Button(PSB_RED))  dir=(1<<5);
  if(ps2x.Button(PSB_GREEN) && mtspd < 250)  mtspd+=10;
  if(ps2x.Button(PSB_BLUE) && mtspd > 50)  mtspd-=10;
  
  //Serial.print(dir);  Serial.print(", ");  Serial.println(mtspd);
  
  switch(dir)
  {
    case 1:
    motorSpeed(mtspd, -mtspd, -mtspd, mtspd);
    break;
    
    case 2:
    motorSpeed(-mtspd, mtspd, mtspd, -mtspd);
    break;
    
    case 4:
    motorSpeed(-mtspd, -mtspd, mtspd, mtspd);
    break;
    
    case 8:
    motorSpeed(mtspd, mtspd, -mtspd, -mtspd);
    break;
    
    case 5:
    motorSpeed(0, -mtspd, 0, mtspd);
    break;
    
    case 6:
    motorSpeed(-mtspd, 0, mtspd, 0);
    break;
    
    case 9:
    motorSpeed(mtspd, 0, -mtspd, 0);
    break;
    
    case 10:
    motorSpeed(0, mtspd, 0, -mtspd);
    break;
    
    case 16:
    motorSpeed(-mtspd, -mtspd, -mtspd, -mtspd);
    break;
    
    case 32:
    motorSpeed(mtspd, mtspd, mtspd, mtspd);
    break;
    
    default:
    motorSpeed(0, 0, 0, 0);
    break;
  }
}

void loop() {
  if (micros() - time > 200) {
    time = micros();
    if (counter == abs(spdM1)) {
      digitalWrite(M1_A, 0);
      digitalWrite(M1_B, 0);
    }
    if (counter == abs(spdM2)) {
      digitalWrite(M2_A, 0);
      digitalWrite(M2_B, 0);
    }
    if (counter == abs(spdM3)) {
      digitalWrite(M3_A, 0);
      digitalWrite(M3_B, 0);
    }
    if (counter == abs(spdM4)) {
      digitalWrite(M4_A, 0);
      digitalWrite(M4_B, 0);
    }
    counter++;
    if (counter >= 255) {
      readJoystick();
      digitalWrite(M1_A, 0);
      digitalWrite(M1_B, 0);
      digitalWrite(M2_A, 0);
      digitalWrite(M2_B, 0);
      digitalWrite(M3_A, 0);
      digitalWrite(M3_B, 0);
      digitalWrite(M4_A, 0);
      digitalWrite(M4_B, 0);
      counter = 0;
      if (spdM1 > 0) {
        digitalWrite(M1_A, 1);
      } else if (spdM1 < 0) {
        digitalWrite(M1_B, 1);
      }
      if (spdM2 > 0) {
        digitalWrite(M2_A, 1);
      } else if (spdM2 < 0) {
        digitalWrite(M2_B, 1);
      }
      if (spdM3 > 0) {
        digitalWrite(M3_A, 1);
      } else if (spdM3 < 0) {
        digitalWrite(M3_B, 1);
      }
      if (spdM4 > 0) {
        digitalWrite(M4_A, 1);
      } else if (spdM4 < 0) {
        digitalWrite(M4_B, 1);
      }
    }

  }

}
