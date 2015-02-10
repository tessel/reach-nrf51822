#ifndef __I2C__
#define __I2C__

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include <stddef.h>
#include <stdbool.h>
#include "pca10001.h"

typedef enum
{
    I2C_100KPBS = 0x01980000, 
    I2C_250KPBS = 0x4000000,
    I2C_400KPBS = 0x06680000
} I2C_freq;

typedef struct I2C_config
{
	I2C_freq freq;
	uint8_t scl;
	uint8_t sda;
	uint32_t addr;
	NRF_TWI_Type* I2C_bus;
} I2C_config_t;

#define I2C_ENABLE 5UL
#define I2C_DELAY() nrf_delay_us(4)

#define LEDS_OFF(leds_mask) do {  NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
                            NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)

#define LEDS_ON(leds_mask) do {  NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
                           NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)

#define LED_IS_ON(leds_mask) ((leds_mask) & (NRF_GPIO->OUT ^ LEDS_INV_MASK) )

#define LEDS_INVERT(leds_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
                              NRF_GPIO->OUTSET = ((leds_mask) & ~gpio_state); \
                              NRF_GPIO->OUTCLR = ((leds_mask) & gpio_state); } while (0)

#define LEDS_CONFIGURE(leds_mask) do { uint32_t pin;                  \
                                  for (pin = 0; pin < 32; pin++) \
                                      if ( (leds_mask) & (1 << pin) )   \
                                          nrf_gpio_cfg_output(pin); } while (0)

void i2c_init_config(uint8_t sda, uint8_t scl, uint32_t addr);
int i2c_enable ();
int i2c_disable ();
int i2c_master_transfer (const uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len);
int i2c_master_send (const uint8_t *txbuf, size_t txbuf_len);
int i2c_master_receive (uint8_t *rxbuf, size_t rxbuf_len);

#endif /** __I2C__ **/
