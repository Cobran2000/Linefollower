#include "SerialCommand.h"
#include "BluetoothSerial.h"
#include "EEPROMAnything.h"
#include "EEPROM.h"

#define Baudrate 115200
#define EEPROM_SIZE sizeof(param_t)

BluetoothSerial SerialBT;
SerialCommand sCmd(SerialBT);

//VAR
bool debug = false;
bool running = false;
bool blink = false;


unsigned long previous;

//Param that are save in the EEPROM
struct param_t
{
  unsigned long cycleTime;
} params;


void setup()
{
  SerialBT.begin("BrancoDD Auto");
  
  //Commands
  sCmd.addCommand("set", onSet);
  sCmd.addCommand("start", start);
  sCmd.addCommand("stop", stop);
  sCmd.setDefaultHandler(onUnknownCommand);

  //Read form EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM_readAnything(0, params);
  EEPROM.end();

  //Show that de programme is startert
  SerialBT.println("ready");

  //PinMode
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  //Read serialpoort
  sCmd.readSerial();

  unsigned long current = micros();
  if (current - previous > params.cycleTime)
  {
    previous = current;
    
    //Show cycle
    digitalWrite(LED_BUILTIN, blink);
    blink = !blink;

    //Debug the program
    if (debug)
    { 
      SerialBT.println("Loop running");
    }
  }
}

//Start of the car
void start()
{
  running = true;
  if (debug) SerialBT.println("The cars is started");
}

//Stop of the car
void stop()
{
  running = false;
  if (debug) SerialBT.println("The cars is stopt");
}

//Set of the parameters
void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();

  if (strcmp(param, "debug") == 0) debug = (strcmp(value, "on") ==  0);
  else if (strcmp(param, "cycle") == 0) params.cycleTime = atol(value);             //'atol()' = String to unsigned long

  //Write to EEPROM
  EEPROM.begin(EEPROM_SIZE);
  EEPROM_writeAnything(0, params);
  EEPROM.end();
}

//Unknown command handeler
void onUnknownCommand (char *command)
{
  SerialBT.print("Unknown Command: \"");
  SerialBT.print(command);
  SerialBT.println("\"");
}
