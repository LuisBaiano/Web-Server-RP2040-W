// include/rgb_led.h

#ifndef RGB_LED_H
#define RGB_LED_H

#include "pico/stdlib.h" // Para uint8_t, uint16_t
#include "config.h"  // Para LED_..._PIN e LED_PWM_WRAP

// ----- DEFINES DE CORES PARA STATUS (com ajuste de brilho 40%) -----
// Brilho máximo para cada canal individual se cor pura, considerando PWM_WRAP = 255
#define RGB_BRIGHTNESS_COMPONENT ((uint8_t)(LED_PWM_WRAP * 0.40)) // Ex: 255 * 0.40 = 102

void rgb_led_init(void);
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b); // Você já deve ter esta
void rgb_led_update_from_joystick(uint16_t adc_x, uint16_t adc_y); // Você já deve ter esta

#endif // RGB_LED_H