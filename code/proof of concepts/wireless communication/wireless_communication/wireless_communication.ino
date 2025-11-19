#include "BluetoothSerial.h"
#include "SerialCommand.h"

BluetoothSerial SerialBT;
SerialCommand sCmd(SerialBT);
unsigned long previous;

void setup() {
  // put your setup code here, to run once:
  SerialBT.begin("auto");
  sCmd.addCommand("led", led);
  sCmd.setDefaultHandler(onUnknownCommand);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  sCmd.readSerial();

  unsigned long current = micros();
  if (current - previous > 1000000)
  {
    SerialBT.println("Hello World");
    previous = current;
  } 

  
}

void onUnknownCommand (char *command)
{
  SerialBT.print("Unknown Command: \"");
  SerialBT.print(command);
  SerialBT.println("\"");
}

void led ()
{
  bool Led = !digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, Led);
}