#ifndef __FLASH_H

#define __FLASH_H
#include <string.h>
#include <stdint.h>


#pragma pack(1)
typedef struct{
	uint8_t no;
	uint8_t ssid[30];
	uint8_t pass[30];
} wifi_info_t;
#pragma pack()

void Flash_Erase(uint32_t address);

void Flash_Write_uint(uint32_t address,uint8_t value);
void Flash_Write_Float(uint32_t address,float f);
void Flash_Write_Array(uint32_t address,uint8_t*arr, uint8_t size);
void Flash_Write_Struct(uint32_t address,wifi_info_t dta);

uint8_t Flash_Read_uint(uint32_t address);
float Flash_Read_Float(uint32_t address);
void Flash_Read_Array(uint32_t address, uint8_t*arr,uint8_t size);
void Flash_Read_Struct(uint32_t address, wifi_info_t*dta);

#endif

