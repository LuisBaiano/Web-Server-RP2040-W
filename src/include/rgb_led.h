#ifndef RGB_LED_H
#define RGB_LED_H

#include "pico/stdlib.h"
#include "config.h" 

// luz de status do sistema
#define STATUS_RED ((uint8_t)(LED_PWM_WRAP * 0.40))
#define STATUS_GREEN ((uint8_t)(LED_PWM_WRAP * 0.40))
#define STATUS_BLUE ((uint8_t)(LED_PWM_WRAP * 0.40))

#define RGB_OFF     0, 0, 0
#define RGB_ERROR   STATUS_RED, 0, 0                     // Vermelho
#define RGB_CONNECTING STATUS_RED, STATUS_GREEN/2, 0     // Laranja (R + um pouco de G)
#define RGB_SERVER_OK 0, STATUS_GREEN, 0                 // Verde
#define RGB_WEB_CMD   0, 0, STATUS_BLUE                  // Azul

void rgb_led_init(void);
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b);

#endif // RGB_LED_H