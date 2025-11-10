volatile bool toggle0 = false;
volatile bool toggle1 = false;

void setup() 
{
  pinMode(23,INPUT);
  pinMode(22,INPUT);
  attachInterrupt(23,ISR0,RISING)
  attachInterrupt(22,ISR1,RISING);
  Serial.begin(9600);
}

void loop() 
{
  Serial.println(toggle);
  delay(10000);
}

void ISR0()
{
  toggle0 = !toggle0;
}
void ISR1()
{
  toggle1 = !toggle1;
}