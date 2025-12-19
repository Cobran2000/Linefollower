#include "SerialCommand.h"
#include "BluetoothSerial.h"
#include "EEPROMAnything.h"
#include "EEPROM.h"
#include "soc/gpio_struct.h"

#define Baudrate 115200
#define EEPROM_SIZE sizeof(param_t)

BluetoothSerial SerialBT;
SerialCommand sCmd(SerialBT);

// VAR
bool debug = false, run = false, blink = false, startRead = false, readDone = false;
bool klaar[8];
int  raw[8], normalised[8], powerLeft = 0, powerRight = 0;
unsigned int kruispunt;
float position, iTerm, lastErr;
unsigned long previous, calculationTime, drivePrevious, driveHelp, start[8];

// Motorpinnen
const int MotorRightForward  = 18;
const int MotorRightBackward = 19;
const int MotorLeftForward   = 16;
const int MotorLeftBackward  = 17;

// Pin's
const int startButton = 23;
const int stopButton  = 22;
const int sensor[8]   = {32, 33, 25, 26, 27, 14, 12, 13};

// Param die in EEPROM staan
struct param_t {
  unsigned long cycleTime;
  int   black, white[8], power, maxWhite;
  float diff, kp, ki, kd;
} params;


// --------- Sensor lezen ---------
void sensorRead() {
  if (!startRead) {
    startRead = true;

    for (uint8_t i = 0; i < 8; i++) {
      pinMode(sensor[i], OUTPUT);
      if (sensor[i] < 32) GPIO.out_w1ts       = (1UL << sensor[i]);
      else                GPIO.out1_w1ts.data = (1UL << (sensor[i] - 32));
      klaar[i] = false;
    }
    delayMicroseconds(10);

    unsigned long nu = micros();
    for (uint8_t i = 0; i < 8; i++) {
      pinMode(sensor[i], INPUT);
      start[i] = nu;
    }
    return;
  }

  bool allesKlaar = true;
  unsigned long nu = micros();
  for (uint8_t i = 0; i < 8; i++) {
    if (!klaar[i]) {
      bool low = (digitalRead(sensor[i]) == LOW);
      if (low || (nu - start[i]) > (unsigned long)params.black) {
        raw[i]   = nu - start[i];
        klaar[i] = true;
      } else allesKlaar = false;
    }
  }
  if (allesKlaar) { startRead = false; readDone = true; }
}


// --------- Debug ---------
void onDebug() {
  SerialBT.println();
  SerialBT.print("cycle:"); SerialBT.println(params.cycleTime);
  SerialBT.print("black:"); SerialBT.println(params.black);
  SerialBT.print("white:");
  for (int i = 0; i < 8; i++) { SerialBT.print(params.white[i]); SerialBT.print(' '); }
  SerialBT.println();
  SerialBT.print("maxW:"); SerialBT.println(params.maxWhite);

  SerialBT.print("norm:");
  for (int i = 0; i < 8; i++) { SerialBT.print(normalised[i]); SerialBT.print(' '); }
  SerialBT.println();

  SerialBT.print("raw:");
  for (int i = 0; i < 8; i++) { SerialBT.print(raw[i]); SerialBT.print(' '); }
  SerialBT.println();

  SerialBT.print("pos:");  SerialBT.println(position, 1);
  SerialBT.print("pwr:");  SerialBT.println(params.power);
  SerialBT.print("diff:"); SerialBT.println(params.diff, 2);
  SerialBT.print("kp:");   SerialBT.println(params.kp, 2);
  float cycleSec = params.cycleTime * 1e-6f;
  SerialBT.print("ki:");   SerialBT.println(params.ki / cycleSec, 2);
  SerialBT.print("kd:");   SerialBT.println(params.kd * cycleSec, 2);
  SerialBT.print("left:"); SerialBT.print(powerLeft);
  SerialBT.print(" right:"); SerialBT.println(powerRight);
  SerialBT.print("calc:"); SerialBT.println(calculationTime);
}


// --------- Help ---------
void onHelp() {
  SerialBT.println();
  SerialBT.println("---------------------------------------------------------------------------");
  SerialBT.println("Possible commands:                   <Argument1>   <Argument2>");
  SerialBT.println();
  SerialBT.println("start      (starts program)          <NONE>        <NONE>");
  SerialBT.println("stop       (stops program)           <NONE>        <NONE>");
  SerialBT.println();
  SerialBT.println("set        (sets params)");
  SerialBT.println("                                     <cycle>       <value (int)>");
  SerialBT.println("                                     <power>       <value (int)>");
  SerialBT.println("                                     <diff>        <value (float)>");
  SerialBT.println("                                     <kp>          <value (float)>");
  SerialBT.println("                                     <ki>          <value (float)>");
  SerialBT.println("                                     <kd>          <value (float)>");
  SerialBT.println();
  SerialBT.println("calibrate  (sensor calibration)      <NONE>        <NONE>");
  SerialBT.println("debug      (shows EEPROM values)     <NONE>        <NONE>");
  SerialBT.println("---------------------------------------------------------------------------");
}


// --------- Calibratie ---------
void onCalibrate() {
  SerialBT.print("calibrating...");

  params.black = 2000;
  readDone  = false;
  startRead = false;
  while (!readDone) sensorRead();
  readDone = false;

  int maxW = raw[0];
  for (int i = 0; i < 8; i++) {
    params.white[i] = raw[i];
    if (raw[i] > maxW) maxW = raw[i];
  }
  params.maxWhite = maxW + 10;
  params.black    = params.maxWhite + 150;

  SerialBT.println("done");

  EEPROM.begin(EEPROM_SIZE);
  EEPROM_writeAnything(0, params);
  EEPROM.end();
}


// --------- Start / Stop ---------
void onStart() {
  run = true;
  SerialBT.println("The car is started");
  iTerm = 0;
  drivePrevious = micros();
  driveHelp     = drivePrevious;
  kruispunt     = 0;
}

void onStop() {
  run = false;
  SerialBT.println("The car is stopt");
}


// --------- Set parameters ---------
void onSet() {
  char* param = sCmd.next();
  char* value = sCmd.next();
  if (!param || !value) return;

  if (strcmp(param, "cycle") == 0) {
    long newCycle = atol(value);
    float ratio   = (float)newCycle / params.cycleTime;
    params.ki    *= ratio;
    params.kd    /= ratio;
    params.cycleTime = newCycle;
  } else if (strcmp(param, "power") == 0) {
    params.power = constrain(atoi(value), 0, 255);
  } else if (strcmp(param, "diff") == 0) {
    params.diff = atof(value);
  } else if (strcmp(param, "kp") == 0) {
    params.kp = atof(value);
  } else if (strcmp(param, "ki") == 0) {
    params.ki = atof(value) * (params.cycleTime * 1e-6f);
  } else if (strcmp(param, "kd") == 0) {
    params.kd = atof(value) / (params.cycleTime * 1e-6f);
  }

  EEPROM.begin(EEPROM_SIZE);
  EEPROM_writeAnything(0, params);
  EEPROM.end();
}


// --------- Unknown command ---------
void onUnknownCommand(char *command) {
  SerialBT.print("Unknown Command: \"");
  SerialBT.print(command);
  SerialBT.println("\"");
}


// --------- setup ---------
void setup() {
  SerialBT.begin("BrancoDD Auto");

  sCmd.addCommand("set",       onSet);
  sCmd.addCommand("start",     onStart);
  sCmd.addCommand("stop",      onStop);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("help",      onHelp);
  sCmd.addCommand("debug",     onDebug);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM_readAnything(0, params);
  EEPROM.end();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MotorRightForward,  OUTPUT);
  pinMode(MotorRightBackward, OUTPUT);
  pinMode(MotorLeftForward,   OUTPUT);
  pinMode(MotorLeftBackward,  OUTPUT);

  // NIEUWE LEDC-API: pin-based attach [web:70]
  const uint32_t pwmFreq = 20000;
  const uint8_t  pwmRes  = 8;
  ledcAttach(MotorLeftForward,   pwmFreq, pwmRes);
  ledcAttach(MotorLeftBackward,  pwmFreq, pwmRes);
  ledcAttach(MotorRightForward,  pwmFreq, pwmRes);
  ledcAttach(MotorRightBackward, pwmFreq, pwmRes);

  SerialBT.println("ready");
}


// --------- loop ---------
void loop() {
  sCmd.readSerial();

  unsigned long current = micros();
  if (current - previous < params.cycleTime) return;
  previous = current;

  readDone  = false;
  startRead = false;
  while (!readDone) sensorRead();

  // normalisatie
  normalised[0] = constrain(map(raw[0], params.white[0], params.black, 0, 1000), 0, 1000);
  normalised[1] = constrain(map(raw[1], params.white[1], params.black, 0, 1000), 0, 1000);
  normalised[2] = constrain(map(raw[2], params.white[2], params.black, 0, 1000), 0, 1000);
  normalised[3] = constrain(map(raw[3], params.white[3], params.black, 0, 1000), 0, 1000);
  normalised[4] = constrain(map(raw[4], params.white[4], params.black, 0, 1000), 0, 1000);
  normalised[5] = constrain(map(raw[5], params.white[5], params.black, 0, 1000), 0, 1000);
  normalised[6] = constrain(map(raw[6], params.white[6], params.black, 0, 1000), 0, 1000);
  normalised[7] = constrain(map(raw[7], params.white[7], params.black, 0, 1000), 0, 1000);

  uint8_t index = 0;
  int maxVal = normalised[0];
  int zwartTeller = (normalised[0] >= 750) ? 1 : 0;
  for (uint8_t i = 1; i < 8; i++) {
    if (normalised[i] > maxVal) { maxVal = normalised[i]; index = i; }
    if (normalised[i] >= 750) zwartTeller++;
  }

  if (maxVal < params.maxWhite && run) {
    position = (position < 0) ? -70 : 70;
  } else if (zwartTeller >= 7) {
    unsigned long drivetime = current;
    if (kruispunt % 6 == 0 && run && kruispunt) {
      float time = (drivetime - drivePrevious) * 1e-6f;
      drivePrevious = drivetime;
      SerialBT.print("lap:"); SerialBT.print(time, 3); SerialBT.println("s");
    } else if (!kruispunt) drivePrevious = drivetime;
    if (drivetime - driveHelp > 1000000UL) { kruispunt++; driveHelp = drivetime; }
  } else if (index == 0) {
    position = -50;
  } else if (index == 7) {
    position = 50;
  } else {
    int sNul    = normalised[index];
    int sLinks  = (index > 0) ? normalised[index - 1] : 0;
    int sRechts = (index < 7) ? normalised[index + 1] : 0;
    float b = (sRechts - sLinks) * 0.5f;
    float a = sRechts - b - sNul;
    position = -b / (2 * a) + index - 3.5f;
    position *= 15;
  }

  float error  = -position;
  float output = error * params.kp;
  iTerm += params.ki * error;
  iTerm = constrain(iTerm, -510, 510);
  output += iTerm + params.kd * (error - lastErr);
  lastErr = error;
  output  = constrain(output, -510, 510);

  powerLeft = powerRight = 0;
  if (run) {
    if (output >= 0) {
      powerLeft  = constrain(params.power + params.diff * output, -255, 255);
      powerRight = constrain(powerLeft - output, -255, 255);
    } else {
      powerRight = constrain(params.power - params.diff * output, -255, 255);
      powerLeft  = constrain(powerRight + output, -255, 255);
    }
  }

  // NIEUWE LEDC-API: write per pin
  ledcWrite(MotorLeftForward,   powerLeft  > 0 ?  powerLeft  : 0);
  ledcWrite(MotorLeftBackward,  powerLeft  < 0 ? -powerLeft  : 0);
  ledcWrite(MotorRightForward,  powerRight > 0 ?  powerRight : 0);
  ledcWrite(MotorRightBackward, powerRight < 0 ? -powerRight : 0);

  unsigned long diff = micros() - current;
  if (diff > calculationTime) calculationTime = diff;
}
