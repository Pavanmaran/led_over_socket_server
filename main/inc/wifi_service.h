#ifndef __MAIN_INC_WIFI_SERVICE_H__
#define __MAIN_INC_WIFI_SERVICE_H__
//#include "nvs_flash_drive.h"

#include <stdint.h>
#define Default_SSID 0x00
#define User_SSID 0x01


extern uint8_t mac[6];

void initialise_wifi(void);
void SetSSID(uint8_t *data,uint8_t *uint8_t_SSID, uint16_t len);
void SetPSWD(uint8_t *data, uint8_t *uint8_t_SSID, uint16_t len);


#endif // __MAIN_INC_WIFI_SERVICE_H__