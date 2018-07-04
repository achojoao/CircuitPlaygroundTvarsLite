#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>
#include <TimeLib.h>
#include "Notes.h"

#define NO_BUTTON -1
#define UP_BUTTON 9
#define DOWN_BUTTON 2
#define NO_CONFIGURATION -1
#define TAP_THRESHOLD   100
#define SNOOZE_MINUTES  5

int tapButton[] = {UP_BUTTON,DOWN_BUTTON};
int button = NO_BUTTON;
int bright = 255;
bool alarmEnabled = true;
bool alarmRinging = false;
bool alarmStopped = false;
int alarmNotes = 61; 
int alarmMelody[] = {NOTE_E5,0,NOTE_F5,NOTE_G5,0,NOTE_C6,0,NOTE_D5,0,NOTE_E5,NOTE_F5,0,NOTE_G5,0,NOTE_A5,NOTE_B5,0,NOTE_F6,0,NOTE_A5,0,NOTE_B5,NOTE_C6,NOTE_D6,NOTE_E6,NOTE_E5,0,NOTE_F5,NOTE_G5,0,NOTE_C6,0,NOTE_D6,0,NOTE_E6,NOTE_F6,0,NOTE_G5,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_E6,0,NOTE_D6,0,NOTE_G5,NOTE_F6,0,NOTE_E6,0,NOTE_D6,NOTE_C6 }; 
int alarmNoteDurations[] = {8,16,16,16,16,2,8,8,16,16,2,4,8,16,16,16,16,2,8,8,16,16,4,4,4,8,16,16,16,16,2,8,8,16,16,2,4,8,16,16,8,8,8,16,16,8,8,8,16,16,8,8,8,16,16,8,8,8,16,16,2};
int alarmHour = 7;
int alarmMinute = 0;
int snoozeHour;
int snoozeMinute;

void setup() {
  Serial.begin(115200);
  Serial.println("Circuit Playground Tvärs Lamp!");  
  CircuitPlayground.begin();
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
  CircuitPlayground.setAccelTap(2, TAP_THRESHOLD);    
  setTime(0,0,0,1,1,2018);
  rainbowCycle();
  delay(1000);
  dimmLamp();
}

void loop() {

  button = getButtonPress();
  delay(200);
  switch (button) {
    case UP_BUTTON:      
      changeAlarm();
      break;
    case DOWN_BUTTON: 
      dimmLamp();
      break;
    default:      
      checkAlarm();
  }
  
}

void rainbowCycle() {
  for (int i=0; i<1500; i++) {
    for(int j=0; j<10; j++) {
      CircuitPlayground.strip.setPixelColor(j, CircuitPlayground.colorWheel(((j * 256 / 10) + i) & 255));
    }
    CircuitPlayground.strip.show();
    delay(2);
  }
}

int getButtonPress() {
  for (int b=0; b<2; b++) {
    if (CircuitPlayground.readCap(tapButton[b]) > TAP_THRESHOLD) {
      return tapButton[b];
    }
  }
  return NO_BUTTON;
}

void dimmLamp() {  
  CircuitPlayground.setBrightness(bright);
  for(int i=0; i<10; i++) {
    CircuitPlayground.strip.setPixelColor(i, 255, 255, 255);  
  }
  CircuitPlayground.strip.show();
  bright -= 40;
  if (bright < 40 && bright > 0) {
    bright = 0;
  } else if (bright < 0) {
    bright = 255;
  }
}

void changeAlarm() {
  if (alarmEnabled) {
    if (alarmMinute == 0) {
      alarmMinute = 30;
    } else {
      alarmMinute = 0;
      alarmHour++;
    }
    if (alarmHour == 10) {
      alarmEnabled = false;
      playAlarmOff();
    }
  } else {    
    alarmMinute = 0;
    alarmHour = 5;
    alarmEnabled = true;
    playAlarmOn();
  }
  displayAlarm();
}

void playAlarmOn() {
  CircuitPlayground.playTone(NOTE_B6,50);
  CircuitPlayground.playTone(0,50);
  CircuitPlayground.playTone(NOTE_E7,50);
}

void playAlarmOff() {  
  CircuitPlayground.playTone(NOTE_E7,50);
  CircuitPlayground.playTone(0,50);
  CircuitPlayground.playTone(NOTE_B6,50);
}

void displayAlarm() {
  CircuitPlayground.strip.clear();
  if (alarmEnabled) {
    CircuitPlayground.strip.setBrightness(64);
    for (int i = 0; i < 5; i++) {
      if (i <= (alarmHour - 5)) {
        CircuitPlayground.strip.setPixelColor(i, 0, 255, 0);  
      }
    }  
    for (int i = 5; i < 10; i++) {
      CircuitPlayground.strip.setPixelColor(i, alarmMinute == 0 ? 0 : 255, 0, alarmMinute == 0 ? 255 : 0);  
    }
  }
  CircuitPlayground.strip.show(); 
}

void checkAlarm() {    
  if (alarmEnabled) {    
    if (alarmRinging) {      
      if (!alarmStopped) {    
        playAlarm();
      }
    } else {
      calculateSnooze();
      if ((alarmHour == hour() && alarmMinute == minute()) || 
          (snoozeHour == hour() && snoozeMinute == minute())) {
        if (!alarmStopped) {           
          alarmRinging = true;
        }  
      } else {
        alarmRinging = false;
        alarmStopped = false;
      }
    }
  }
}

void calculateSnooze() {
  snoozeHour = alarmHour;
  snoozeMinute = alarmMinute + SNOOZE_MINUTES;
  if (snoozeMinute>=60) {
    snoozeHour++;
    snoozeMinute = snoozeMinute - 60;
  }
}

void stopAlarm() {
  alarmRinging = false;
  alarmStopped = true;
}

void playAlarm() {    
  for (int thisNote = 0; thisNote < alarmNotes && !alarmStopped; thisNote++) {
    int noteDuration = 750 / alarmNoteDurations[thisNote];
    CircuitPlayground.playTone(alarmMelody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;      
    checkTapForAlarm();
    button = getButtonPress();
    if (button == UP_BUTTON || button == DOWN_BUTTON) {
      stopAlarm();
    }
    delay(pauseBetweenNotes);
  }
}

void checkTapForAlarm() {
  if (alarmRinging) {
    float x = 0;
    float y = 0;
    float z = 0;
    for (int i=0; i<10; i++) {
      x += CircuitPlayground.motionX();
      y += CircuitPlayground.motionY();
      z += CircuitPlayground.motionZ();
      delay(1);
    }
    x /= 10;
    y /= 10;
    z /= 10;    
    float totalAccel = sqrt(y*y);
    if (totalAccel > 0.5) {
      stopAlarm();
    }
  }
}
