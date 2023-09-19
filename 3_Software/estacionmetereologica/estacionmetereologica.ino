#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>

Adafruit_BMP280 bmp;

//float TEMPERATURA
float PRESION, P0;
int SENSOR = 2;
int HUMEDAD;
int TEMPERATURA;

DHT dht(SENSOR, DHT11);

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("Iniciando;");
  if ( !bmp.begin() ) {
    Serial.println("BMP280 No encontrado !");
    while (1);
  }
  P0 = bmp.readPressure()/92.25 + 3.35;

}


void loop() {
  TEMPERATURA = bmp.readTemperature();
  PRESION = bmp.readPressure()/92.25;
  HUMEDAD = dht.readHumidity();

    Serial.print("Temperaruta:  ");
    Serial.print(TEMPERATURA);
    Serial.print(" ºC ");

    Serial.print("Humedad:  ");
    Serial.print(HUMEDAD);
    Serial.print(" % ");
    
    Serial.print("Presion:  ");
    Serial.print(PRESION);
    Serial.print(" hPa ");
    
    Serial.print(" - Altitud snm:  ");
    Serial.print(bmp.readAltitude(P0));
    Serial.print(" mts ");

    delay(5000);
    
}
