#include "leds.h"

Adafruit_NeoPixel barra1(NUM_PIXELS, PIN_BARRA1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel barra2(NUM_PIXELS, PIN_BARRA2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel barra3(NUM_PIXELS, PIN_BARRA3, NEO_GRB + NEO_KHZ800);

void inicializarLEDs() {
  barra1.begin();
  barra1.show();
  barra2.begin();
  barra2.show();
  barra3.begin();
  barra3.show();
}

void atualizarLEDs(Adafruit_NeoPixel &barra, int valor) {
  
  int numLedsAtivos = map(valor, 0, 100, 0, NUM_PIXELS);
  uint32_t cor = (valor > 66) ? barra.Color(255, 0, 0) : 
                  (valor > 33) ? barra.Color(255, 255, 0) : 
                                 barra.Color(0, 255, 0);

  for (int i = 0; i < NUM_PIXELS; i++) {
    barra.setPixelColor(i, i < numLedsAtivos ? cor : barra.Color(0, 0, 0));
  }
  
  barra.show();
  delay(10);
  barra.show(); // Garantir atualização
}
