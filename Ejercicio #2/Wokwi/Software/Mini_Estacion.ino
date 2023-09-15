#include <Wire.h>
#include <DHT.h>

#define DHTPIN 15      // Pin donde está conectado el sensor DHT22
#define DHTTYPE DHT22  // Tipo de sensor DHT
#define POTENTIOMETER_PIN 2 // Pin analógico al que está conectado el potenciómetro

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // Lectura de humedad y temperatura del sensor DHT22
  float humedad = dht.readHumidity();
  float temperaturaDHT22 = dht.readTemperature();

  // Lectura del valor del potenciómetro
  int potValue = analogRead(POTENTIOMETER_PIN);

  // Mapear el valor del potenciómetro al rango de 900 a 1100 hPa
  float presion = map(potValue, 0, 1023, 900, 1100) / 10.0; // Convertir a hPa

  // Verificar si la lectura de los sensores fue exitosa
  if (isnan(humedad) || isnan(temperaturaDHT22)) {
    Serial.println("Error al leer el sensor DHT22.");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.print("%\n");
    Serial.print("Temperatura (DHT22): ");
    Serial.print(temperaturaDHT22);
    Serial.println("°C");
  }

  Serial.print("Presión atmosférica: ");
  Serial.print(presion);
  Serial.println(" hPa");

  delay(2000); // Esperar 2 segundos antes de realizar la siguiente lectura
}
