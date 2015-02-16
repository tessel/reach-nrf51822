#include <stdint.h>
#include <stdlib.h>
#include "spi.h"
#include "ble.h"
#include "nordic_common.h"
#include "ble_srv_common.h"

#define BASE                  0x80
#define NOP                   0x00
#define SLEEP                 0x10

#define SPICMDBASE            0x20
#define SPIENABLE             0x20
#define SPITRANSFER           0x21
#define SPIDISABLE            0x22
#define SPISETCLOCKDIVISOR    0x23
#define SPISETMODE            0x24
#define SPISETROLE            0x25
#define SPISETFRAME           0x26

#define I2CCMDBASE            0x30
#define I2CENABLE             0x30
#define I2CWRITE              0x31
#define I2CREAD               0x32
#define I2CDISABLE            0x33
#define I2CSETMODE            0x34
#define I2CSETSLAVEADDRESS    0x35

#define UARTCMDBASE           0x40
#define UARTENABLE            0x40
#define UARTTRANSFER          0x41
#define UARTRECEIVE           0x42
#define UARTDISABLE           0x43
#define UARTSETBAUDRATE       0x44
#define UARTSETDATABITS       0x45
#define UARTSETPARITY         0x46
#define UARTSETSTIPBITS       0x47

#define GPIOCMDBASE           0x50
#define GPIOSETPULL           0x50
#define GPIOSETSTATE          0x51
#define GPIOWRITEPWMVALUE     0x52
#define GPIOGETPULL           0x53
#define GPIOGETSTATE          0x54
#define GPIOREADPULSELENGTH   0x55
#define GPIOSETINTERRUPTMODE  0x56
#define GPIOGETINTERRUPTMODE  0x57

#define NO_CHANGE             0xFF

typedef enum 
{
  BLE_GOSSIP_EVT_NOTIFICATION_ENABLED,
  BLE_GOSSIP_EVT_NOTIFICATION_DISABLED,
  BLE_GOSSIP_EVT_INDICATION_ENABLED,
  BLE_GOSSIP_EVT_INDICATION_DISABLED,
  BLE_GOSSIP_EVT_INDICATION_CONFIRMED
} ble_gossip_evt_type_t;

typedef enum 
{
  SPIEnable,
  I2CEnable,
  UARTEnable,
  Idle
} state_t;

typedef struct 
{
  ble_gossip_evt_type_t evt_type;
} ble_gossip_evt_t;

typedef struct gossip_s gossip_t;

typedef void (*ble_gossip_evt_handler_t) (gossip_t * gossip, ble_gossip_evt_t * p_evt);

typedef struct 
{
  ble_gossip_evt_handler_t evt_handler;
} ble_gossip_init_t;

typedef struct gossip_s 
{
  uint16_t service_handle;
  ble_gossip_evt_handler_t evt_handler;
  ble_gatts_char_handles_t command_handles;
  ble_gatts_char_handles_t response_handles;
  uint16_t conn_handle;
  state_t state;
} gossip_t;

void ble_gossip_on_ble_evt(gossip_t * gossip, ble_evt_t * p_ble_evt);

uint32_t ble_gossip_init(gossip_t * gossip, const ble_gossip_init_t * p_gossip_init);

uint8_t SPI_handle_buffer(gossip_t * gossip, uint8_t * incoming, uint8_t * outgoing);

void slice_payload(uint8_t * incoming, uint8_t length, uint8_t * payload);
