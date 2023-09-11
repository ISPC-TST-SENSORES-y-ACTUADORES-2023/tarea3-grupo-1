#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DHT22.h" // Biblioteca para el sensor DHT22
#include "bmp180.h" // Biblioteca para el sensor BMP180

#define F_CPU 16000000UL // Frecuencia del reloj en Hz
#define BAUD 9600
#define UBRR (F_CPU / 16 / BAUD - 1)

// Definir los pines utilizados para el sensor DHT22
#define DHT22_PIN PC0

// Función para inicializar el UART
void uart_init() {
    UBRR0H = (UBRR >> 8);
    UBRR0L = UBRR;
    UCSR0B |= (1 << TXEN0) | (1 << RXEN0); // Habilitar transmisor y receptor
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); // Modo asíncrono, 8 bits de datos, sin paridad, 1 bit de parada
}

// Función para enviar un carácter a través del UART
void uart_transmit(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

// Función para enviar una cadena a través del UART
void uart_transmit_string(const char* data) {
    for (int i = 0; i < strlen(data); i++) {
        uart_transmit(data[i]);
    }
}

int main() {
    // Inicializar el UART
    uart_init();

    // Inicializar el sensor DHT22
    DHT22_init(DHT22_PIN);

    // Inicializar el sensor BMP180
    bmp180_init();

    // Variables para almacenar los datos
    float temperature, humidity, pressure;

    while (1) {
        // Leer datos del sensor DHT22
        if (DHT22_read(&temperature, &humidity) == DHT22_OK) {
            // Leer datos del sensor BMP180
            if (bmp180_read(&pressure) == BMP180_OK) {
                // Mostrar los datos en el UART
                char buffer[50];
                sprintf(buffer, "Temp: %.2f C\nHum: %.2f%%\nPress: %.2f hPa\n", temperature, humidity, pressure);
                uart_transmit_string(buffer);
            } else {
                uart_transmit_string("Error al leer el sensor BMP180\n");
            }
        } else {
            uart_transmit_string("Error al leer el sensor DHT22\n");
        }

        _delay_ms(5000); // Esperar 5 segundos antes de tomar otra lectura
    }

    return 0;
}