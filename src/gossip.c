#include "gossip.h"

// Gets called when connect event occurs
static void on_connect(gossip_t * gossip, ble_evt_t * p_ble_evt)
{
  gossip->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
}

// Gets called when disconnect event occurs
static void on_disconnect(gossip_t * gossip, ble_evt_t * p_ble_evt)
{
  UNUSED_PARAMETER(p_ble_evt);
  gossip->conn_handle = BLE_CONN_HANDLE_INVALID;
}

// Gets called when a characteristic has new notification/indication subscription
static void on_cccd_write(gossip_t * gossip, ble_gatts_evt_write_t * p_evt_write)
{
  if (p_evt_write->len == 2)
  {
    // CCCD written, update indication state
    if (gossip->evt_handler != NULL)
    {
      ble_gossip_evt_t evt;

      if (ble_srv_is_indication_enabled(p_evt_write->data))
      {
        evt.evt_type = BLE_GOSSIP_EVT_INDICATION_ENABLED;
      }
      else
      {
        evt.evt_type = BLE_GOSSIP_EVT_INDICATION_DISABLED;
      }

      gossip->evt_handler(gossip, &evt);
    }
  }
  
  // uint16_t p_len = p_evt_write->len;
  // uint8_t const p_value[1] = { p_evt_write->len};
  // sd_ble_gatts_value_set( gossip->response_handles.value_handle, 0, &p_evt_write->len, p_evt_write->data);
}

// Gets called when handle for characteristic or CCCD has been written to
static void on_write(gossip_t * gossip, ble_evt_t * p_ble_evt)
{
  ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
  
  if (p_evt_write->handle == gossip->response_handles.cccd_handle)
  {
    on_cccd_write(gossip, p_evt_write);
  } else if (p_evt_write->handle == gossip->command_handles.value_handle)
  {
    
  }
  
  sd_ble_gatts_value_set( gossip->response_handles.value_handle, 0, &p_evt_write->len, p_evt_write->data);
}

// Gets called when a value has been confirmed by the client
static void on_hvc(gossip_t * gossip, ble_evt_t * p_ble_evt)
{
  ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;
  
  if(p_hvc->handle == gossip->response_handles.value_handle){
    ble_gossip_evt_t evt;
    
    evt.evt_type = BLE_GOSSIP_EVT_INDICATION_CONFIRMED;
    gossip->evt_handler(gossip, &evt);
  }
}

// Route various events to their proper handlers
void ble_gossip_on_ble_evt(gossip_t * gossip, ble_evt_t * p_ble_evt)
{
  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      on_connect(gossip, p_ble_evt);
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      on_disconnect(gossip, p_ble_evt);
      break;

    case BLE_GATTS_EVT_WRITE:
      on_write(gossip, p_ble_evt);
      break;

    case BLE_GATTS_EVT_HVC:
      on_hvc(gossip, p_ble_evt);
    break;

    default:
      // No implementation needed.
      break;
  }
}

uint32_t ble_gossip_init(gossip_t * gossip, const ble_gossip_init_t * p_gossip_init)
{
    // Initialize service structure
    gossip->evt_handler = p_gossip_init->evt_handler;
    gossip->conn_handle = BLE_CONN_HANDLE_INVALID;
    gossip->state = Idle;

    return NRF_SUCCESS;
}


// Outgoing buffer will have to be free'd by the calling function
uint8_t SPI_handle_buffer(gossip_t * gossip, uint8_t * incoming, uint8_t * outgoing)
{
  uint8_t command = incoming[0];
  uint8_t payload_len = incoming[1];
  switch (command){
    case SPISETCLOCKDIVISOR: {
      uint8_t payload[payload_len];
      slice_payload(incoming, payload_len, payload);
      uint8_t param = payload[0];
      spi_set_clock_speed_divisor(param);
      outgoing = (uint8_t *) malloc(sizeof(uint8_t));
      outgoing[0] = SPISETCLOCKDIVISOR;
      return 1;
    }
    case SPISETMODE: {
      uint8_t payload[payload_len];
      slice_payload(incoming, payload_len, payload);
      uint8_t param = payload[0];
      spi_set_mode(param);
      outgoing = (uint8_t *) malloc(sizeof(uint8_t));
      outgoing[0] = SPISETMODE;
      return 1;
    }
    case SPISETROLE: {
      uint8_t payload[payload_len];
      slice_payload(incoming, payload_len, payload);
      uint8_t param = payload[0];
      spi_set_role(param);
      outgoing = (uint8_t *) malloc(sizeof(uint8_t));
      outgoing[0] = SPISETROLE;
      return 1;
    }
    case SPISETFRAME: {
      uint8_t payload[payload_len];
      slice_payload(incoming, payload_len, payload);
      uint8_t param = payload[0];
      spi_set_frame(param);
      outgoing = (uint8_t *) malloc(sizeof(uint8_t));
      outgoing[0] = SPISETFRAME;
      return 1;
    }
    default:
    switch (gossip->state) {
      case Idle:
      switch (command) {
        case SPIENABLE: {
          spi_enable();
          gossip->state = SPIEnable;
          outgoing = (uint8_t *) malloc(sizeof(uint8_t));
          outgoing[0] = SPIENABLE;
          return 1;
        }
        case SPIDISABLE: {
          outgoing = (uint8_t *) malloc(sizeof(uint8_t));
          outgoing[0] = SPIDISABLE;
          return 1;
        }
        default:
        return 0;
      }
      case SPIEnable:
      switch (command){
        case SPIENABLE: {
          outgoing = (uint8_t *) malloc(sizeof(uint8_t));
          outgoing[0] = SPIENABLE;
          return 1;
        }
        case SPITRANSFER: {
          uint8_t payload[payload_len];
          slice_payload(incoming, payload_len, payload);
          outgoing[0] = SPITRANSFER;
          outgoing[1] = payload_len;
          outgoing = (uint8_t *) malloc((sizeof(uint8_t) * payload_len) + 2);
          return spi_transfer(payload, outgoing);
        }
        case SPIDISABLE: {
          spi_disable();
          gossip->state = Idle;
          outgoing = (uint8_t *) malloc(sizeof(uint8_t));
          outgoing[0] = SPIDISABLE;
          return 1;
        }
        default:
        return 0;
      }
      default: 
      return 0;
    }
  }
}
