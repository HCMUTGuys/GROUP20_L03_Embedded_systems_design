#include "flash.h"
#include "stm32f1xx.h"

void Flash_Erase(uint32_t address)
{
		HAL_FLASH_Unlock();
		FLASH_EraseInitTypeDef Eraseintstruct;
		Eraseintstruct.Banks = 1; //stm32f103 chi co 1 banks
		Eraseintstruct.NbPages = 1; // chon xoa 1 page hoac xoa 2,3,...,128 page
		Eraseintstruct.PageAddress = address; // address la dia chi page muon xoa
		Eraseintstruct.TypeErase = FLASH_TYPEERASE_PAGES;
		uint32_t pageerr;
		HAL_FLASHEx_Erase(&Eraseintstruct,&pageerr);
		HAL_FLASH_Lock();	
	}

	
////
////
	void Flash_Write_uint(uint32_t address,uint8_t value)
{
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, value); // ghi 2 byte
	HAL_FLASH_Lock();	

}
////
void Flash_Write_Float(uint32_t address,float f) //kieu float se ghi 4 byte
{
	HAL_FLASH_Unlock();
	uint8_t data[4];
	*(float*)data = f;
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, *(uint32_t*)data);
	HAL_FLASH_Lock();	
}
////
void Flash_Write_Array(uint32_t address,uint8_t*arr, uint8_t size)
{
	HAL_FLASH_Unlock();
	uint16_t*ptr= (uint16_t*)arr;
	for (uint16_t i=0;i<(size+1)/2;i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address+2*i, *ptr);
		ptr++;
	}
	HAL_FLASH_Lock();	
}
////
void Flash_Write_Struct(uint32_t address,wifi_info_t dta)
{
	HAL_FLASH_Unlock();
	Flash_Write_Array(address,(uint8_t*)&dta,sizeof(dta));
	HAL_FLASH_Lock();	

}



////
////
uint8_t Flash_Read_uint(uint32_t address)
{	
		return*( __IO uint16_t* )(address);
}
////
float Flash_Read_Float(uint32_t address)
{
	uint32_t data= *( __IO uint32_t*)(address);
	return *(float*)(&data);
}
////
void Flash_Read_Array(uint32_t address, uint8_t*arr,uint8_t size)
{
	uint16_t*ptr= (uint16_t*)arr;
	for (uint16_t i=0;i<(size+1)/2;i++)
	{
		*ptr = *( __IO uint16_t* )(address+2*i);
		ptr++;
	}

}
////
void Flash_Read_Struct(uint32_t address, wifi_info_t*dta)
{
	Flash_Read_Array(address,(uint8_t*)dta,sizeof(wifi_info_t));
}

