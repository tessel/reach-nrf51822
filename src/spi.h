#include <stdint.h>

void spi_enable(void);

void spi_disable(void);

void spi_set_clock_speed_divisor(uint8_t speed);

void spi_set_mode(uint8_t mode);

void spi_set_role(uint8_t role);

void spi_set_frame(uint8_t role);

uint8_t spi_transfer(uint8_t * incoming, uint8_t * outgoing);
