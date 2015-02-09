#ifndef __I2C__
#define __I2C__

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum
{
    I2C_100KPBS = 0x01980000, 
    I2C_250KPBS = 0x4000000,
    I2C_400KPBS = 0x06680000
} I2C_freq;

#define I2C_ENABLE 5UL
#define I2C_DELAY() nrf_delay_us(4)

void i2c_enable (int scl, int sda, I2C_freq freq, uint8_t addr);
void i2c_disable (int scl, int sda);
int i2c_master_transfer (int scl, int sda, I2C_freq freq, uint8_t addr, const uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len);
int i2c_master_send (int scl, int sda, I2C_freq freq, uint8_t addr, const uint8_t *txbuf, size_t txbuf_len);
int i2c_master_receive (int scl, int sda, I2C_freq freq, uint8_t addr, uint8_t *rxbuf, size_t rxbuf_len);

// int i2c_slave_transfer (uint32_t port, const uint8_t *txbuf, size_t txbuf_len, uint8_t *rxbuf, size_t rxbuf_len);
// int i2c_slave_send (uint32_t port, const uint8_t *txbuf, size_t txbuf_len);
// int i2c_slave_receive (uint32_t port, uint8_t *rxbuf, size_t rxbuf_len);
// void i2c_set_slave_addr (uint32_t port, uint8_t slave_addr);

#endif /** __I2C__ **/
