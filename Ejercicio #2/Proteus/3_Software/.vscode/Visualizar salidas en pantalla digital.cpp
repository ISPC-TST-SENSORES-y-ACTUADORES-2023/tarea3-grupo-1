#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  dht.begin();
  if (!bmp.begin(0x76)) {
    Serial.println("No se pudo encontrar un sensor BMP280.");
    while (1);
  }

  if (!display.begin(SSD1306_I2C_ADDRESS, SCREEN_ADDRESS)) {
    Serial.println(F("Error: No se pudo inicializar la pantalla OLED"));
    while (true);
  }
  display.display(); // Inicializa con pantalla en blanco
  delay(2000); // Espera 2 segundos
  display.clearDisplay();
}

void loop() {
  delay(2000); // Espera 2 segundos entre lecturas

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pressure = bmp.readPressure() / 100.0F; // Presión en hPa

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print("°C\tHumedad: ");
  Serial.print(humidity);
  Serial.print("%\tPresión: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.print("Humedad: ");
  display.print(humidity);
  display.println(" %");
  display.print("Presion: ");
  display.print(pressure);
  display.println(" hPa");
  display.display();
}
