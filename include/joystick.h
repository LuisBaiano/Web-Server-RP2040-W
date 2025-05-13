#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h" // Para uint16_t
// #include "config.h" // Se JOYSTICK_..._PIN/_CHANNEL são definidos aqui

void joystick_init(void);
// Se joystick_read não existe, use read_adc diretamente no main
// void joystick_read(uint16_t *x, uint16_t *y);
uint16_t read_adc(uint adc_channel); // A função que você já tem em joystick.c

#endif // JOYSTICK_H