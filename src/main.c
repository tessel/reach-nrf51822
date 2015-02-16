/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
*
* The information contained herein is property of Nordic Semiconductor ASA.
* Terms and conditions of usage are described in detail in NORDIC
* SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
*
* Licensees are granted free, non-transferable use of the information. NO
* WARRANTY of ANY KIND is provided. This heading must NOT be removed from
* the file.
*
*/

/** @file
*
* @defgroup ble_sdk_app_beacon_main main.c
* @{
* @ingroup ble_sdk_app_beacon
* @brief Beacon Transmitter Sample Application main file.
*
* This file contains the source code for an Beacon transmitter sample application.
*/

#include <stdbool.h>
#include <stdint.h>
#include "ble_advdata.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "ble_gap.h"
#include "gossip.h"
// #include "device_manager.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define APP_CFG_NON_CONN_ADV_TIMEOUT    0                                 /**< Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout. */
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define APP_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define APP_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */

#define DEAD_BEEF                       0xDEADBEEF                        /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define GOSSIP_UUID_BASE                {0x45, 0x6A, 0x11, 0x40, 0xC6, 0x9B, 0x1C, 0xA4, 0x52, 0x4F, 0x19, 0xD4, 0x00, 0x00, 0x2A, 0xD3}
#define GOSSIP_UUID_SERVICE             0x1100
#define GOSSIP_UUID_COMMAND             0x1101
#define GOSSIP_UUID_RESPONSE            0x1102

#define MAX_CHAR_VAL_LEN                32

#define CONNECTABLE_ADV_INTERVAL      MSEC_TO_UNITS(20, UNIT_0_625_MS)              /**< The advertising interval for connectable advertisement (20 ms). This value can vary between 20ms to 10.24s. */
#define NON_CONNECTABLE_ADV_INTERVAL  MSEC_TO_UNITS(100, UNIT_0_625_MS)             /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

const char*                     device_name = "Reach";
static uint16_t                 m_conn_handle = BLE_CONN_HANDLE_INVALID;
static ble_gap_adv_params_t     m_adv_params;
ble_advdata_uuid_list_t         services;
static uint8_t                  command_char_value[MAX_CHAR_VAL_LEN];
static uint8_t                  response_char_value[MAX_CHAR_VAL_LEN];
gossip_t                        gossip;

/**@brief Callback function for asserts in the SoftDevice.
*
* @details This function will be called in case of an assert in the SoftDevice.
*
* @warning This handler is an example only and does not fit a final product. You need to analyze
*          how your product is supposed to react in case of Assert.
* @warning On assert from the SoftDevice, the system can only recover on reset.
*
* @param[in]   line_num   Line number of the failing ASSERT call.
* @param[in]   file_name  File name of the failing ASSERT call.
*/
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for initializing the Advertising functionality.
*
* @details Encodes the required advertising data and passes it to the stack.
*          Also builds a structure to be passed to the stack when starting advertising.
*/
static void advertising_init(void)
{
  uint32_t      err_code;
  ble_advdata_t advdata;
  uint8_t       flags = 0x06; // BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED & BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE;

  // Build and set advertising data.
  memset(&advdata, 0, sizeof(advdata));
  
  advdata.name_type             = BLE_ADVDATA_SHORT_NAME ;
  advdata.flags.size            = sizeof (flags);
  advdata.flags.p_data          = &flags;
  // advdata.p_manuf_specific_data = &manuf_specific_data;
  
  err_code = ble_advdata_set(&advdata, NULL);
  APP_ERROR_CHECK(err_code);
  
  // Initialize advertising parameters (used when starting advertising).
  memset(&m_adv_params, 0, sizeof(m_adv_params));
  
  m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_IND;
  m_adv_params.p_peer_addr = NULL;                             // Undirected advertisement.
  m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
  m_adv_params.interval    = CONNECTABLE_ADV_INTERVAL;
  m_adv_params.timeout     = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
}


/**@brief Function for starting advertising.
*/
static void advertising_start(void)
{
  uint32_t err_code;
  
  err_code = sd_ble_gap_adv_start(&m_adv_params);
  APP_ERROR_CHECK(err_code);
  
}


static ble_uuid_t make_uuid(ble_uuid128_t base, uint16_t id)
{
  uint32_t err_code;
  
  uint8_t uuid_type;
  err_code = sd_ble_uuid_vs_add(&base, &uuid_type);
  APP_ERROR_CHECK(err_code);
  
  ble_uuid_t uuid;
  uuid.type = uuid_type;
  uuid.uuid = id;
  
  return uuid;
}

static void char_add(uint16_t m_service_handle, ble_uuid_t char_uuid, uint8_t *m_char_value, ble_gatts_char_handles_t m_char_handles)
{
  uint32_t            err_code;
  ble_gatts_char_md_t char_md;
  ble_gatts_attr_md_t cccd_md;
  ble_gatts_attr_t    attr_char_value;
  ble_gatts_attr_md_t attr_md;
  
  memset(&cccd_md, 0, sizeof(cccd_md));
  
  cccd_md.vloc = BLE_GATTS_VLOC_STACK;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
  
  memset(&char_md, 0, sizeof(char_md));
  
  char_md.char_props.read   = 1;
  char_md.char_props.notify = 1;
  char_md.p_char_user_desc  = NULL;
  char_md.p_char_pf         = NULL;
  char_md.p_user_desc_md    = NULL;
  char_md.p_cccd_md         = &cccd_md;
  char_md.p_sccd_md         = NULL;
  
  memset(&attr_md, 0, sizeof(attr_md));
  
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
  
  attr_md.vloc    = BLE_GATTS_VLOC_STACK;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen    = 0;
  
  memset(&attr_char_value, 0, sizeof(attr_char_value));
  
  attr_char_value.p_uuid    = &char_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len  = BLE_GATTS_FIX_ATTR_LEN_MAX;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len   = BLE_GATTS_FIX_ATTR_LEN_MAX;
  attr_char_value.p_value   = m_char_value;
  
  err_code = sd_ble_gatts_characteristic_add(m_service_handle,
  &char_md,
  &attr_char_value,
  &m_char_handles);
  APP_ERROR_CHECK(err_code);
}

/**
* Initialize the gatt profile of the device.
*/
static void gatt_init(void)
{
  uint32_t err_code;
  ble_gap_conn_sec_mode_t name_security;
  name_security.lv = 0;
  name_security.sm = 0;
  err_code = sd_ble_gap_device_name_set(&name_security, (const uint8_t * const) device_name, strlen(device_name));
  APP_ERROR_CHECK(err_code);
  
  ble_uuid_t gossip_service_uuid = make_uuid( (ble_uuid128_t) {GOSSIP_UUID_BASE}, GOSSIP_UUID_SERVICE);
  ble_uuid_t command_char_uuid = make_uuid( (ble_uuid128_t) {GOSSIP_UUID_BASE}, GOSSIP_UUID_COMMAND);
  ble_uuid_t response_char_uuid = make_uuid( (ble_uuid128_t) {GOSSIP_UUID_BASE}, GOSSIP_UUID_RESPONSE);
  
  uint16_t gossip_service_handle;
  ble_gatts_char_handles_t command_char_handle;
  ble_gatts_char_handles_t response_char_handle;
  
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &gossip_service_uuid, &gossip_service_handle);
  gossip.service_handle = gossip_service_handle;
  
  char_add(gossip_service_handle, command_char_uuid, command_char_value, command_char_handle);
  gossip.command_handles = command_char_handle;
  
  char_add(gossip_service_handle, response_char_uuid, response_char_value, response_char_handle);
  gossip.response_handles = response_char_handle;
  

  // uuid_list[1] = gossip_service;
  // services.uuid_cnt = sizeof(uuid_list) / sizeof(uuid_list[0]);
  // services.p_uuids = uuid_list;
}

static void on_ble_evt(ble_evt_t * p_ble_evt)
{
  // uint32_t err_code;
  
  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
      m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
      break;

    case BLE_GAP_EVT_DISCONNECTED:
      m_conn_handle = BLE_CONN_HANDLE_INVALID;
      advertising_start();
      break;

    default:
      break;
  }
}

static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
  // dm_ble_evt_handler(p_ble_evt);
  ble_gossip_on_ble_evt(&gossip, p_ble_evt);
  on_ble_evt(p_ble_evt);
}

/**@brief Function for initializing the BLE stack.
*
* @details Initializes the SoftDevice and the BLE event interrupt.
*/
static void ble_stack_init(void)
{
  // Initialize the SoftDevice handler module.
  SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, false);
  
  // Enable BLE stack 
  uint32_t err_code;
  ble_enable_params_t ble_enable_params;
  memset(&ble_enable_params, 0, sizeof(ble_enable_params));
  ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
  err_code = sd_ble_enable(&ble_enable_params);
  APP_ERROR_CHECK(err_code);
  
  err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
  APP_ERROR_CHECK(err_code);

}


/**@brief Function for doing power management.
*/
static void power_manage(void)
{
  uint32_t err_code = sd_app_evt_wait();
  APP_ERROR_CHECK(err_code);
}

void gossip_evt_handler(gossip_t * gossip, ble_gossip_evt_t * p_evt)
{

}

static void make_gossip(void)
{
  uint32_t err_code;
  
  ble_gossip_init_t gossip_init;
  
  gossip_init.evt_handler = &gossip_evt_handler;
  
  err_code = ble_gossip_init(&gossip, &gossip_init);
  APP_ERROR_CHECK(err_code);
  
}

/**
* @brief Function for application main entry.
*/
int main(void)
{
  // Initialize.
  ble_stack_init();
  make_gossip();
  gatt_init();
  advertising_init();
  
  // Start execution.
  advertising_start();
  
  // Enter main loop.
  for (;; )
  {
    power_manage();
  }
}


/**
* @}
*/
