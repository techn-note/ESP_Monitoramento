#ifndef LEDS_H
#define LEDS_H

#include <Adafruit_NeoPixel.h>

#define PIN_BARRA1 25
#define PIN_BARRA2 26
#define PIN_BARRA3 27
#define NUM_PIXELS 8

extern Adafruit_NeoPixel barra1;
extern Adafruit_NeoPixel barra2;
extern Adafruit_NeoPixel barra3;

void inicializarLEDs();
void atualizarLEDs(Adafruit_NeoPixel &barra, int valor);

#endif
