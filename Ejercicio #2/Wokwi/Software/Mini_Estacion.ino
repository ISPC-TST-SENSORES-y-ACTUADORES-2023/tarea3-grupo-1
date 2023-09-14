// Prueba de placa personalizada gy-63 (chip BMP280)
// con un ESP32.
// Sin bibliotecas aparte de arduino.h y wire.h estándar
#include <Wire.h>
#include <Preferences.h>
#include "DHTesp.h" // Agregamos la biblioteca DHTesp

int pinDHT = 15; // Este es el pin al que está conectado el sensor DHT22

#define GYRO_ID 0x68
// El ID del dispositivo BMP-280 es 0x77 de manera predeterminada, si CSB está conectado a Vcc el ID del dispositivo es 0x76.
#define BAROMETRO_ID 0x77
#define SDA 21
#define SCL 22

Preferences preferences;
DHTesp dht; // Declaración de la instancia DHTesp

// Copia del registro de estado del BMP280
union
{
  struct
  {
    uint8_t im_update : 1;
    uint8_t : 2;
    uint8_t midiendo : 1;
  } bit;
  uint8_t reg;
} estado = {.reg = 0};

// Parámetros de ajuste de compensación del BMP280 (coeficientes)
struct
{
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;
  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;
} parametros;

uint8_t anterior_midiendo;             // Estado de medición anterior
uint32_t presion_cruda, temperatura_cruda; // Valores de medición no compensados. El valor siempre es positivo, aunque las funciones de compensación esperan una entrada como entero con signo.
int64_t t_fino;                         // t_fino lleva la temperatura fina como variable global.
void configurar_barometro(void);
void leer_barometro(void);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Bienvenidos a la Mini Estacion Meteorologica");

  preferences.begin("mi-aplicacion", false);
  Wire.begin(SDA, SCL, 400000U);
  delay(250);

  // Comprueba si el BMP280 está respondiendo.
  Wire.beginTransmission(BAROMETRO_ID);
  if (Wire.endTransmission() == 0)
  {
    Serial.println("Conexión BMP280 exitosa.");
    configurar_barometro();
  }
  else
  {
    Serial.println("Fallo de conexión BMP280.");
  }
  
    // Inicializa el sensor DHT22
  dht.setup(pinDHT, DHTesp::DHT22);

  // Comprueba si el MPU650 está respondiendo.
  Wire.beginTransmission(GYRO_ID);
  if (Wire.endTransmission() == 0)
  {
    Serial.println("Conexión MPU6050 exitosa.");
  }
  else
  {
    Serial.println("Fallo de conexión MPU6050.");
  }
}

void loop()
{
  leer_barometro();
  delay(500); // esto acelera la simulación
// Leer y mostrar los datos del sensor DHT22
  TempAndHumidity data = dht.getTempAndHumidity();
  Serial.println("Temperatura DHT22: " + String(data.temperature, 2) + "°C");
  Serial.println("Humedad DHT22: " + String(data.humidity, 1) + "%");
//  Serial.println("---");
// delay(500);
// Espera hasta que se presione una tecla
  Serial.println("Presione una tecla para continuar...");
  while (Serial.available() == 0) {
    // Espera activa hasta que se presione una tecla
  }
  // Limpia el búfer del puerto serie
  while (Serial.available() > 0) {
    Serial.read();
  }
}

void configurar_barometro(void)
{
  // Restablecer el BMP280
  Wire.beginTransmission(BAROMETRO_ID); // Inicia la comunicación con el BMP280.
  Wire.write(0xE0);                     // Subdirección del registro de restablecimiento.
  Wire.write(0xB6);                     // Establece el código de restablecimiento.
  Wire.endTransmission();               // Finaliza la transmisión con el BMP280.
  delay(10);

  // Obtén parámetros de compensación desde la memoria no volátil del BMP280.
  Wire.beginTransmission(BAROMETRO_ID); // Inicia la comunicación con el BMP280.
  Wire.write(0x88);                     // Subdirección del registro de parámetros de compensación.
  Wire.endTransmission();               // Finaliza la transmisión con el BMP280.
  Wire.requestFrom(BAROMETRO_ID, 24U);  // Obtiene 24 bytes desde el registro de parámetros de compensación.

  parametros.dig_T1 = Wire.read() | Wire.read() << 8;
  parametros.dig_T2 = Wire.read() | Wire.read() << 8;
  parametros.dig_T3 = Wire.read() | Wire.read() << 8;
  parametros.dig_P1 = Wire.read() | Wire.read() << 8;
  parametros.dig_P2 = Wire.read() | Wire.read() << 8;
  parametros.dig_P3 = Wire.read() | Wire.read() << 8;
  parametros.dig_P4 = Wire.read() | Wire.read() << 8;
  parametros.dig_P5 = Wire.read() | Wire.read() << 8;
  parametros.dig_P6 = Wire.read() | Wire.read() << 8;
  parametros.dig_P7 = Wire.read() | Wire.read() << 8;
  parametros.dig_P8 = Wire.read() | Wire.read() << 8;
  parametros.dig_P9 = Wire.read() | Wire.read() << 8;

  Serial.printf("T1: %d\n", parametros.dig_T1);
  Serial.printf("T2: %d\n", parametros.dig_T2);
  Serial.printf("T3: %d\n", parametros.dig_T3);
  Serial.printf("P1: %d\n", parametros.dig_P1);
  Serial.printf("P2: %d\n", parametros.dig_P2);
  Serial.printf("P3: %d\n", parametros.dig_P3);
  Serial.printf("P4: %d\n", parametros.dig_P4);
  Serial.printf("P5: %d\n", parametros.dig_P5);
  Serial.printf("P6: %d\n", parametros.dig_P6);
  Serial.printf("P7: %d\n", parametros.dig_P7);
  Serial.printf("P8: %d\n", parametros.dig_P8);
  Serial.printf("P9: %d\n", parametros.dig_P9);

  // Tasa de salida de datos utilizando el modo normal, resolución estándar (4x de sobremuestreo de presión, 1x de sobremuestreo de temperatura),
  // Filtro IIR @ 16 y tiempo de espera de 0.5ms es de 83.3Hz o aproximadamente cada 12ms.

  // Establecer el Registro de Configuración
  // SPI o I2C        IIRFILTER        Tiempo de Espera
  // I2C   0x00        OFF   0x00        0.5MS   0x00
  // SPI   0x01         2    0x01       62.5MS   0x01
  //                    4    0x02        125MS   0x02
  //                    8    0x03        250MS   0x03
  //                   16    0x04         500MS   0x04
  //                                    1000MS   0x05
  //                                    2000MS   0x06
  //                                    3000MS   0x07
  Wire.beginTransmission(BAROMETRO_ID);     // Inicia la comunicación con el BMP280.
  Wire.write(0xF5);                         // Subdirección del registro de configuración.
  Wire.write(0x00 << 5 | 0x04 << 2 | 0x00); // Establece spi3w_en, filtro, t_sb
  Wire.endTransmission();

  // Establecer el Registro de Control de Medida
  // Sobremuestreo      Modo
  // NINGUNO   0x00       SLEEP   0X00
  // X1     0X01       FORZADO  0X01
  // X2     0X02       NORMAL  0X03
  // X4     0X03
  // X8     0X04
  // X16    0X05
  Wire.beginTransmission(BAROMETRO_ID);     // Inicia la comunicación con el BMP280.
  Wire.write(0xF4);                         // Subdirección del registro de Control de Medida.
  Wire.write(0x01 << 5 | 0x03 << 2 | 0x03); // Establece osrs_t, osrs_p, modo
  Wire.endTransmission();
  // Realiza 1000 lecturas para que el barómetro se estabilice.
  // for (int i = 0; i < 1000; i++)
  //{
  leer_barometro();
  delay(4);
  //}
}

void leer_barometro(void)
{
  // Comprueba el bit de estado para ver si los datos están listos.
  // Si no lo están, regresa.
  // Si lo están, obtén los datos y procesa.
  Wire.beginTransmission(BAROMETRO_ID);
  Wire.write(0xF3);
  Wire.endTransmission();
  Wire.requestFrom(BAROMETRO_ID, 1U);
  estado.reg = Wire.read();
  if (estado.bit.midiendo ^ anterior_midiendo)
    anterior_midiendo = estado.bit.midiendo;
  else
  {
    Wire.beginTransmission(BAROMETRO_ID);
    Wire.write(0xF7);
    Wire.endTransmission();
    Wire.requestFrom(BAROMETRO_ID, 6U);
    presion_cruda = Wire.read() << 12 | Wire.read() << 4 | Wire.read() >> 4;
    temperatura_cruda = Wire.read() << 12 | Wire.read() << 4 | Wire.read() >> 4;
    // Serial.printf("Presión cruda: %d\n", presion_cruda);
    // Serial.printf("Temperatura cruda: %d\n", temperatura_cruda);

    int64_t var1, var2, T, P;
    var1 = ((((temperatura_cruda >> 3) - ((int64_t)parametros.dig_T1 << 1))) * ((int64_t)parametros.dig_T2)) >> 11;
    var2 = (((((temperatura_cruda >> 4) - ((int64_t)parametros.dig_T1)) * ((temperatura_cruda >> 4) - ((int64_t)parametros.dig_T1))) >> 12) * ((int64_t)parametros.dig_T3)) >> 14;
    t_fino = var1 + var2;
    T = (t_fino * 5 + 128) >> 8; // Corregir t_fine a t_fino aquí

    var1 = ((int64_t)t_fino) - 128000;
    var2 = var1 * var1 * (int64_t)parametros.dig_P6;
    var2 = var2 + ((var1 * (int64_t)parametros.dig_P5) << 17);
    var2 = var2 + (((int64_t)parametros.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)parametros.dig_P3) >> 8) + ((var1 * (int64_t)parametros.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)parametros.dig_P1) >> 33;
    if (var1 != 0)
    {
      P = 1048576 - presion_cruda;
      P = (((P << 31) - var2) * 3125) / var1;
      var1 = (((int64_t)parametros.dig_P9) * (P >> 13) * (P >> 13)) >> 25;
      var2 = (((int64_t)parametros.dig_P8) * P) >> 19;
      P = (((P + var1 + var2) >> 8) + (((int64_t)parametros.dig_P7) << 4)) / 256;
    }
    // Serial.printf("Presión Compensada: %4.2f\n", P / 100.0);
    // Serial.printf("Temperatura Compensada: %4.2f\n", T / 100.0 * 1.8 + 32.0);
    // Serial.printf("%4.2f, %4.2f\n", P / 100.0, T / 100.0 * 1.8 + 32.0);
    Serial.print("Presión Atmosférica BMP280: ");
    //Serial.printf("%4.2f\n", pressure);
    Serial.printf("%4.2f\n", T / 100.0 * 1.8 + 32.0);
  }
}
