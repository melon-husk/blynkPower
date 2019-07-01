
#define BLYNK_USE_DIRECT_CONNECT
#define BLYNK_MAX_READBYTES 512
// You could use a spare Hardware Serial on boards that have it (like Mega)
#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2, 3); // RX, TX

#define BLYNK_PRINT DebugSerial
#include <BlynkSimpleSerialBLE.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "_I3NIgXZIvsd4En3Fe0hyfKJ6YcJUvEM";

/**************************************Variables*************************************/

float current, voltage, energy = 0, power=0,v,vbc;
float averageCurrent = 0, averageVoltage=0, averagePower=0;

void setup()
{
  // Debug console
  DebugSerial.begin(9600);

  DebugSerial.println("Waiting for connections...");

  // Blynk will work through Serial
  // 9600 is for HC-06. For HC-05 default speed is 38400
  // Do not read or write this serial manually in your sketch
  Serial.begin(38400);
  Blynk.begin(Serial, auth);
}

void loop()
{
  Blynk.run();
  sendValues();
}
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

void getCurrent()
{
  vbc = readVcc()/1000.0 ;
  float adc = analogRead(0);
  v = (adc/1024.0) * 5.1;
  current = (v-2.5)/0.011;
}
void getVoltage()
{
  //float Vcc = readVcc()/1000.0;
  voltage = (analogRead(1)/1024.0)*5.1*4.34*0.939;
}
void getPower()
{
  power = averageVoltage*averageCurrent; //in W
  energy = energy + power/1000;
}
void avgCurrent()
{
  
  int MeasurementsToAverage = 16;
  for(int i = 0; i < MeasurementsToAverage; ++i)
  {
    getCurrent();
    averageCurrent += current;
    delay(1);
  }
  averageCurrent /= MeasurementsToAverage;
}
void avgVoltage()
{
  
  int MeasurementsToAverage = 16;
  for(int i = 0; i < MeasurementsToAverage; ++i)
  {
    getVoltage();
    averageVoltage += voltage;
    delay(1);
  }
  averageVoltage /= MeasurementsToAverage;
}
void avgPower()
{
  
  int MeasurementsToAverage = 16;
  for(int i = 0; i < MeasurementsToAverage; ++i)
  {
    getPower();
    averagePower += power;
    delay(1);
  }
  averagePower /= MeasurementsToAverage;
}

void sendValues()
{
  //getVoltage();
  avgVoltage();
  Blynk.virtualWrite(V1, averageVoltage);
  //getCurrent();
  avgCurrent();
  if(averageCurrent<0.3 && averageCurrent > -0.3)
  {
    Blynk.virtualWrite(V2, 0);
  }
  else
  {
    Blynk.virtualWrite(V2, averageCurrent);
  }
  
  Blynk.virtualWrite(V5, v);
  Blynk.virtualWrite(V6,vbc);
  avgPower();
  if(averagePower<3 && averagePower > -3)
  {
    Blynk.virtualWrite(V3, 0);
  }
  else
  {
    Blynk.virtualWrite(V3, averagePower);
  }
  Blynk.virtualWrite(V4, energy);
  delay(50);
}


