// Contains macros for common hardware defines
#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

// Debug LED
#define DEBUG_LED     PIN_LED1

// Button defines
#define BUTTON_RESET  PIN_BUTTON1
#define BUTTON_OFF    PIN_BUTTON2
#define BUTTON_ON     PIN_BUTTON3

///////////////////////////////////////////////////////////////////
// Matrix Defines
///////////////////////////////////////////////////////////////////
#define NRF_GPIO_REGISTER_SIZE 32

// Use all port 1 pins, some port 0 pins are dedicated so lets
// not play around with those...
#define PIN_R1   (NRF_GPIO_REGISTER_SIZE + 1)
#define PIN_R2   (NRF_GPIO_REGISTER_SIZE + 2)
#define PIN_G1   (NRF_GPIO_REGISTER_SIZE + 3)
#define PIN_G2   (NRF_GPIO_REGISTER_SIZE + 4)
#define PIN_B1   (NRF_GPIO_REGISTER_SIZE + 5)
#define PIN_B2   (NRF_GPIO_REGISTER_SIZE + 6)
#define PIN_A    (NRF_GPIO_REGISTER_SIZE + 7)
#define PIN_B    (NRF_GPIO_REGISTER_SIZE + 8)
#define PIN_C    (NRF_GPIO_REGISTER_SIZE + 10)
#define PIN_D    (NRF_GPIO_REGISTER_SIZE + 11)
#define PIN_CLK  (NRF_GPIO_REGISTER_SIZE + 12)
#define PIN_LAT  (NRF_GPIO_REGISTER_SIZE + 13)
#define PIN_OE   (NRF_GPIO_REGISTER_SIZE + 14)

#define PIXEL_HEIGHT 32
#define PIXEL_WIDTH 64

#endif // HARDWARECONFIG_H
