#include "i2c.h"

/* most of this is copied from the official SDK, 
 * with a couple of changes to make things
 * more usable.
 * Original files are located in nRF51_SDK_7.1.0/components/drivers_nrf/twi_master
*/

I2C_config_t i2c_config = {I2C_400KPBS, 0, 0, 0x00, NRF_TWI0};

#define MAX_WAIT 20000UL // max i2c hang time. should eventually replace with timer...?

static void I2C_PIN_LOW(int pin) {
	NRF_GPIO->OUTCLR = (1UL << pin);
}

static void I2C_PIN_HIGH(int pin) {
	NRF_GPIO->OUTSET = (1UL << pin);  
}

static unsigned long I2C_PIN_READ(int pin){
	return (NRF_GPIO->IN >> pin) & 0x1UL;
}

/**
 * @brief Function for detecting stuck slaves (SDA = 0 and SCL = 1) and tries to clear the bus.
 *
 * @return
 * @retval false Bus is stuck.
 * @retval true Bus is clear.
 */
static bool twi_master_clear_bus(int scl, int sda)
{
    uint32_t twi_state;
    bool     bus_clear;
    uint32_t clk_pin_config;
    uint32_t data_pin_config;

    // Save and disable TWI hardware so software can take control over the pins.
    twi_state        = i2c_config.I2C_bus->ENABLE;
    i2c_config.I2C_bus->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;

    clk_pin_config = \
        NRF_GPIO->PIN_CNF[scl];
    NRF_GPIO->PIN_CNF[scl] =      \
        (GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1    << GPIO_PIN_CNF_DRIVE_Pos)   \
      | (GPIO_PIN_CNF_PULL_Pullup   << GPIO_PIN_CNF_PULL_Pos)    \
      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)   \
      | (GPIO_PIN_CNF_DIR_Output    << GPIO_PIN_CNF_DIR_Pos);

    data_pin_config = \
        NRF_GPIO->PIN_CNF[sda];
    NRF_GPIO->PIN_CNF[sda] =       \
        (GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1    << GPIO_PIN_CNF_DRIVE_Pos)   \
      | (GPIO_PIN_CNF_PULL_Pullup   << GPIO_PIN_CNF_PULL_Pos)    \
      | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)   \
      | (GPIO_PIN_CNF_DIR_Output    << GPIO_PIN_CNF_DIR_Pos);

    I2C_PIN_HIGH(sda);
    I2C_PIN_HIGH(scl);
    I2C_DELAY();

    if ((I2C_PIN_READ(sda) == 1) && (I2C_PIN_READ(scl) == 1))
    {
        bus_clear = true;
    }
    else
    {
        uint_fast8_t i;
        bus_clear = false;

        // Clock max 18 pulses worst case scenario(9 for master to send the rest of command and 9
        // for slave to respond) to SCL line and wait for SDA come high.
        for (i=18; i--;)
        {
            I2C_PIN_LOW(scl);
    		I2C_DELAY();
            I2C_PIN_HIGH(scl);
    		I2C_DELAY();

            if (I2C_PIN_READ(sda) == 1)
            {
                bus_clear = true;
                break;
            }
        }
    }

    NRF_GPIO->PIN_CNF[scl] = clk_pin_config;
    NRF_GPIO->PIN_CNF[sda]  = data_pin_config;

    i2c_config.I2C_bus->ENABLE = twi_state;

    return bus_clear;
}

// void SPI0_TWI0_IRQHandler(void) {
//     // check if we're sending or receiving
//     // sending

//     // recv

// }

void i2c_init_config(uint8_t scl, uint8_t sda, uint32_t addr) {
    i2c_config.scl = scl;
    i2c_config.sda = sda;
    i2c_config.addr = addr;
}

// returns 0 when enabled, 1 on error
int i2c_enable () {

    if (!i2c_config.scl || !i2c_config.sda) return 1;

	NRF_GPIO->PIN_CNF[i2c_config.scl] =     \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[i2c_config.sda] =      \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    i2c_config.I2C_bus->ADDRESS = i2c_config.addr;
    i2c_config.I2C_bus->EVENTS_RXDREADY = 0;
    i2c_config.I2C_bus->EVENTS_TXDSENT  = 0;
    i2c_config.I2C_bus->PSELSCL         = i2c_config.scl;
    i2c_config.I2C_bus->PSELSDA         = i2c_config.sda;
    i2c_config.I2C_bus->FREQUENCY       = i2c_config.freq;
    i2c_config.I2C_bus->ENABLE          = I2C_ENABLE;

    // these three lines appear to do the following:
    // set up peripherial interconnect such that whenever i2c 
    // crosses a byte boundry (end of a byte), the i2c task 
    // will be suspended.
	NRF_PPI->CH[0].EEP        = (uint32_t)&(i2c_config.I2C_bus)->EVENTS_BB;
    NRF_PPI->CH[0].TEP        = (uint32_t)&(i2c_config.I2C_bus)->TASKS_SUSPEND;
    NRF_PPI->CHENCLR          = PPI_CHENCLR_CH0_Msk;
    
    twi_master_clear_bus(i2c_config.scl, i2c_config.sda);

    return 0;
}

// returns 0 when disabled, 1 on error
int i2c_disable () {

    if (!i2c_config.scl || !i2c_config.sda) return 1;

	// turn scl & sda back into normal pins
	NRF_GPIO->PIN_CNF[i2c_config.scl] =     \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Disconnect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

    NRF_GPIO->PIN_CNF[i2c_config.sda] =      \
        (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Disconnect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos);

	// disable twi0
    i2c_config.I2C_bus->ENABLE = 0;

    return 0;
}

int i2c_master_transfer (const uint8_t *txbuf, size_t txbuf_len,
 uint8_t *rxbuf, size_t rxbuf_len)
{
	bool transfer_succeeded = false;
	if (txbuf_len > 0 && twi_master_clear_bus(i2c_config.scl, i2c_config.sda)) {
        transfer_succeeded = i2c_master_send(txbuf, txbuf_len);
	}
	
	if (rxbuf_len > 0) {
        transfer_succeeded = transfer_succeeded & i2c_master_receive(rxbuf, rxbuf_len);
	}

	return transfer_succeeded;
}

int i2c_power_cycle() {
	// Recover the peripheral as indicated by PAN 56: "TWI: TWI module lock-up." found at
    // Product Anomaly Notification document found at 
    // https://www.nordicsemi.com/eng/Products/Bluetooth-R-low-energy/nRF51822/#Downloads
    i2c_config.I2C_bus->EVENTS_ERROR = 0;
    i2c_config.I2C_bus->ENABLE       = 0; //TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos; 
    i2c_config.I2C_bus->POWER        = 0;
    nrf_delay_us(5);
    i2c_config.I2C_bus->POWER        = 1;
    i2c_config.I2C_bus->ENABLE       = I2C_ENABLE;//TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;

    (void)(i2c_enable());

    return false;
}

int i2c_master_send (const uint8_t *data, size_t txbuf_len) {
	uint32_t timeout = MAX_WAIT; /* max loops to wait for EVENTS_TXDSENT event*/

    if (txbuf_len == 0)
    {
        /* Return false for requesting data of size 0 */
        return false;
    }

    i2c_config.I2C_bus->TXD           = *data++;
    i2c_config.I2C_bus->TASKS_STARTTX = 1;

    /** @snippet [TWI HW master write] */
    while (true)
    {
        while (i2c_config.I2C_bus->EVENTS_TXDSENT == 0 && i2c_config.I2C_bus->EVENTS_ERROR == 0 && (--timeout))
        {
            // Do nothing.
        }

        if (timeout == 0 || i2c_config.I2C_bus->EVENTS_ERROR != 0)
        {
            return i2c_power_cycle();
        }
        i2c_config.I2C_bus->EVENTS_TXDSENT = 0;
        if (--txbuf_len == 0)
        {
            break;
        }

        i2c_config.I2C_bus->TXD = *data++;
    }
    /** @snippet [TWI HW master write] */

    // if (issue_stop_condition)
    // {
    // always issue stop condition?
        // NRF_TWI0->EVENTS_STOPPED = 0;
        // NRF_TWI0->TASKS_STOP     = 1;
        // /* Wait until stop sequence is sent */ 
        // while(NRF_TWI0->EVENTS_STOPPED == 0) 
        // {
        //     // Do nothing.
        // }
    // }
    return true;
}

int i2c_master_receive (uint8_t *rxbuf, size_t rxbuf_len) {
	uint32_t timeout = MAX_WAIT; /* max loops to wait for RXDREADY event*/

    if (rxbuf_len == 0)
    {
        /* Return false for requesting data of size 0 */
        return false;
    }
    else if (rxbuf_len == 1)
    {
        NRF_PPI->CH[0].TEP = (uint32_t)&(i2c_config.I2C_bus)->TASKS_STOP;
    }
    else
    {
        NRF_PPI->CH[0].TEP = (uint32_t)&(i2c_config.I2C_bus)->TASKS_SUSPEND;
    }

    NRF_PPI->CHENSET          = PPI_CHENSET_CH0_Msk;
    i2c_config.I2C_bus->EVENTS_RXDREADY = 0;
    i2c_config.I2C_bus->TASKS_STARTRX   = 1;

    /** @snippet [TWI HW master read] */
    while (true)
    {
        while (i2c_config.I2C_bus->EVENTS_RXDREADY == 0 && i2c_config.I2C_bus->EVENTS_ERROR == 0 && (--timeout))
        {
            // Do nothing.
        }
        i2c_config.I2C_bus->EVENTS_RXDREADY = 0;

        if (timeout == 0 || i2c_config.I2C_bus->EVENTS_ERROR != 0)
        {
            return i2c_power_cycle();
        }

        *rxbuf++ = i2c_config.I2C_bus->RXD;

        /* Configure PPI to stop TWI master before we get last BB event */
        if (--rxbuf_len == 1)
        {
            NRF_PPI->CH[0].TEP = (uint32_t)&(i2c_config.I2C_bus)->TASKS_STOP;
        }

        if (rxbuf_len == 0)
        {
            break;
        }

        // Recover the peripheral as indicated by PAN 56: "TWI: TWI module lock-up." found at
        // Product Anomaly Notification document found at
        // https://www.nordicsemi.com/eng/Products/Bluetooth-R-low-energy/nRF51822/#Downloads
        nrf_delay_us(20);
        i2c_config.I2C_bus->TASKS_RESUME = 1;
    }
    /** @snippet [TWI HW master read] */

    /* Wait until stop sequence is sent */
    while(i2c_config.I2C_bus->EVENTS_STOPPED == 0)
    {
        // Do nothing.
    }
    i2c_config.I2C_bus->EVENTS_STOPPED = 0;

    NRF_PPI->CHENCLR = PPI_CHENCLR_CH0_Msk;
    return true;
}
