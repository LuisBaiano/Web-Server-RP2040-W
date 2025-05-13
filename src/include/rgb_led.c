#include "config.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include <stdlib.h>
#include <stdint.h>


/**
 * @brief Inicializa os pinos PWM para os LEDs RGB.
 *        Associa as funções PWM aos pinos e configura o wrap.
 *        Define os LEDs inicialmente como desligados.
 */
void rgb_led_init() {
    gpio_set_function(LED_RED_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_GREEN_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE_PIN, GPIO_FUNC_PWM);

    uint slice_red = pwm_gpio_to_slice_num(LED_RED_PIN);
    uint slice_green = pwm_gpio_to_slice_num(LED_GREEN_PIN);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, LED_PWM_WRAP);

    pwm_init(slice_red, &config, true);
    pwm_init(slice_green, &config, true);
    pwm_init(slice_blue, &config, true);

    rgb_led_set(0, 0, 0);
}

/**
 * @brief Define os níveis de PWM para os canais R, G e B do LED.
 *        Assume LED com cátodo comum: 0 = desligado, 255 = intensidade máxima.
 *
 * @param r Intensidade do canal vermelho (0–255).
 * @param g Intensidade do canal verde (0–255).
 * @param b Intensidade do canal azul (0–255).
 */
void rgb_led_set(uint8_t r, uint8_t g, uint8_t b) {
    pwm_set_gpio_level(LED_RED_PIN, r);
    pwm_set_gpio_level(LED_GREEN_PIN, g);
    pwm_set_gpio_level(LED_BLUE_PIN, b);
}

