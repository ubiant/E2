#include <E2.h>

E2_Device hum_sensor(4,5); //SDA=4 SCL=5

float HR;
float temp;
unsigned char state;

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  HR = hum_sensor.RH_read();
  temp = hum_sensor.Temp_read();
  state = hum_sensor.Status();
  Serial.print("Humidity reading: ");
  Serial.println(HR);
  Serial.print("Temp reading: ");
  Serial.println(temp);
  Serial.print("State reading: ");
  Serial.println(state);
  delay(2000);
}
