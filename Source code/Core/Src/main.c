/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "flash.h"
#include "rc522.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_BACKLIGHT 0x08     // Turn on light
#define LCD_NOBACKLIGHT 0x00  // Turn off light
#define addr_pcf8574 0x4E
#define address_so_luong_the_storage 0x08000000 + 61*1024 //page 61
#define address_password_storage 0x08000000 + 62*1024	//page 62
//
#define address_master_card_delete_storage 0x08000000 + 63*1024 //page 63 
#define address_master_card_add_storage 0x08000000 + 64*1024 //page 64 
//
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI2_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
void send_command(uint8_t data) {
	uint8_t buf[4] = {
        (data & 0xF0) | 0x04 | LCD_BACKLIGHT, // 4 bit hight + EN = 1 + Backlight
        (data & 0xF0) | LCD_BACKLIGHT,       // 4 bit hight + EN = 0 + Backlight
        (data << 4) | 0x04 | LCD_BACKLIGHT,  // 4 bit low + EN = 1 + Backlight
        (data << 4) | LCD_BACKLIGHT          // 4 bit low + EN = 0 + Backlight
    };
    HAL_I2C_Master_Transmit(&hi2c2, addr_pcf8574, buf, 4, HAL_MAX_DELAY);
}
//
void send_data(uint8_t data) {
    uint8_t buf[4] = {
        (data & 0xF0) | 0x05 |LCD_BACKLIGHT, // 4 bit hight + RS=1 + EN=1 + Backlight
        (data & 0xF0) | 0x01 | LCD_BACKLIGHT, // 4 bit hight + RS=1 + EN=0 + Backlight
        (data << 4) | 0x05 | LCD_BACKLIGHT,   // 4 bit low + RS=1 + EN=1 + Backlight
        (data << 4) | 0x01 | LCD_BACKLIGHT    // 4 bit low + RS=1 + EN=0 + Backlight
    };
    HAL_I2C_Master_Transmit(&hi2c2, addr_pcf8574, buf, 4, HAL_MAX_DELAY);
}
//
void LCD_Init()
{
	send_command(0x33); //lenh de khoi tao
	send_command(0x32); //lenh de khoi tao
	send_command(0x28); // 4bit,2hang, font 5x7
	send_command(0x0C); //hien thi man hinh va tat con tro
	send_command(0x06); //tang con tro
	send_command(0x01); // xoa man hinh
	HAL_Delay(1);
}
//ham xoa man hinh 
void LCD_CLEAR_SCREEN()
{
	send_command(0x01);
	HAL_Delay(1);
}
// ham chon vi tri hien thi
void LCD_LOCATION(uint8_t x,uint8_t y) // x la hang, y la cot
{
if (x==1)
	send_command(0x80+y);
else if(x==2)
	send_command(0xC0+y);

}
//ham ghi chuoi
void LCD_WRITE_STRING(char *string)
{
	for (uint8_t i=0;i<strlen(string);i++)
	{
		send_data(string[i]);
	}
}
// ham ghi so
void LCD_WRITE_NUMBER(int number)
{
	char buffer[8];
	sprintf(buffer,"%d",number);
	LCD_WRITE_STRING(buffer);
}
//HAM CHAY CHUOI TU PHAI QUA TRAI
void data_run(uint8_t a,int speed,char* chuoi)
{
	int count=strlen(chuoi); //chieu dai cua chuoi
	char destination[count]; //mang chua chuoi
	strcpy(destination, chuoi);//dua chuoi vao mang
	for (int i=0;i<=count;i++)
	 
		{
			LCD_LOCATION(a,0);
			for (int j=i;j<i+16;j++)
			{
				
				send_data(destination[j]);
				HAL_Delay(1);
				if(j==count-1)goto EXIT;
			}
			HAL_Delay(speed);
		}
		EXIT:;
}

//HAM QUET KEYPAB 
	uint8_t SCAN()
{
uint8_t KEY=255;
	while(1)
{
	//DOC HANG 1
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
			HAL_Delay(20);
			while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
			KEY=1;
		return KEY;
		
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=2;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=3;
		return KEY;
	}
	//DOC HANG 2
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
		KEY=4;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=5;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=6;
		return KEY;
	}
	//DOC HANG 3
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
		KEY=7;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=8;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=9;
		return KEY;
	}
	// DOC HANG 4
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
		KEY=23; //MODE
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=0;
		return KEY;
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=24;//CLEAR
		return KEY;
	}
	return 25;
}
}
// TAO HAM QUET 6 KI TU, HIEN THI RA MAN HINH DAU"*" KHI CO PHIM NHAN VA TAO MANG PASS CHECK DE SO SANH VOI PASSWORD
void pass_array(uint8_t PASSCHECK[6])
{
	
		int i=0;
		int j=0;
		while(1)
		{
			
			uint8_t KEY=255;
	//DOC HANG 1
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
			HAL_Delay(100);
			while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
			KEY=1;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
		
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=2;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=3;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	//DOC HANG 2
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
		KEY=4;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=5;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=6;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	//DOC HANG 3
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4))==0){}
		KEY=7;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=8;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))==0){}
		KEY=9;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	// DOC HANG 4
	HAL_GPIO_WritePin(GPIOA,(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3),GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
	
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0)
	{
		HAL_Delay(100);
		while((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5))==0){}
		KEY=0;
		PASSCHECK[i]=KEY;
				i++;
				if(i==j+1){
				j++;
				LCD_LOCATION(2,j+4);
				LCD_WRITE_STRING("*");
				HAL_Delay(2);
				}
				if(i>5)
				{goto AA;}
	}
	
	
		}
		AA: ;
	}
// TAO HAM KIEM TRA 2 MANG CO GIONG NHAU KHONG
//NEU GIONG THI TRA VE 1, KHAC THI TRA VE 0
uint8_t CHECK_ARRAY(uint8_t PASS_W[],uint8_t b[],size_t size) 
{
		int a=1;
		for (size_t i = 0; i < size; ++i) 
		{
        if (PASS_W[i]!=b[i])  
				{
					a=0;
					break;
				}	
    }
		return a;
}

// DEFAULT PASSWORD
uint8_t PASS[6]={0,0,0,0,0,0};
//
uint8_t c[6]={0};
uint8_t A[6]={0};
uint8_t d[6]={0};
uint8_t Check[6]={0};
uint8_t str[MAX_LEN];
volatile int count=0;

static uint8_t master_add[5]={0x0A, 0xFB, 0x7C ,0x00 ,0x8D};
static uint8_t master_delete[5]={0x73, 0xDF, 0x0C ,0xF5 ,0x55};
//
// HAM DOI MAT KHAU
void CHANGE_PASS()
{
	FF:;
	LCD_CLEAR_SCREEN();
	HAL_Delay(1);
	LCD_LOCATION(1,1);
	LCD_WRITE_STRING("RESET PASSWORD!");
	HAL_Delay(2000);
	LCD_CLEAR_SCREEN();
	HAL_Delay(500);
	data_run(1,200,"   PRESS ANY KEY TO CONFIRM OR PRESS '#' TO EXIT");
	while (1)
	{
		uint8_t a=SCAN();
		if(a==24)
		{
			goto exit;
		}
		if(a!=25)
		{
			break;
		}
	}
	while(1)
	{
		LCD_CLEAR_SCREEN();
		HAL_Delay(1);
		LCD_LOCATION(1,2);
		LCD_WRITE_STRING("Old Password!");
		HAL_Delay(200);
		
		uint8_t A[6];
		pass_array(A);
		HAL_Delay(200);
		uint8_t check[6];
		Flash_Read_Array(address_password_storage, check,6);
		if(CHECK_ARRAY(check,A,6)==1)
		{
			LCD_LOCATION(2,0);
			LCD_WRITE_STRING("                ");
			LCD_LOCATION(2,5);
			LCD_WRITE_STRING("CORRECT");
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_Delay(1000);
		}
		else{
			LCD_LOCATION(2,0);
			LCD_WRITE_STRING("                ");
			LCD_LOCATION(2,6);
			LCD_WRITE_STRING("FAIL");
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_Delay(1000);
			goto FF;
		}
		AA:;
		LCD_CLEAR_SCREEN();
		HAL_Delay(1);
		LCD_LOCATION(1,2);
		LCD_WRITE_STRING("New Password");
		HAL_Delay(100);
		pass_array(c);
		HAL_Delay(200);
		LCD_CLEAR_SCREEN();
		HAL_Delay(1);
		LCD_LOCATION(1,1);
		LCD_WRITE_STRING("Re_New Password");
		HAL_Delay(100);
		pass_array(d);
		HAL_Delay(200);
		///check
		if(CHECK_ARRAY(c,d,6)==0)
									{
										LCD_CLEAR_SCREEN();
										HAL_Delay(1);
										LCD_LOCATION(1,3);
										LCD_WRITE_STRING("NOT MATCH !!");
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
						HAL_Delay(500);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
										HAL_Delay(2000);
										goto AA;
									}
		if(CHECK_ARRAY(c,d,6)==1)
		{
										Flash_Erase(address_password_storage);
										Flash_Write_Array(address_password_storage, c, 6);
										LCD_CLEAR_SCREEN();
										HAL_Delay(1);
										LCD_LOCATION(1,3);
										LCD_WRITE_STRING("Successful!");
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(400);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_Delay(1000);
		count=0;
										
		}
		///
	break;
	}
		exit:;
	LCD_CLEAR_SCREEN();
	HAL_Delay(1);
	}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
	LCD_Init();
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	MFRC522_Init();
	
	

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		//
		LCD_CLEAR_SCREEN();
		HAL_Delay(1);
		LCD_LOCATION(1,5);
		LCD_WRITE_STRING("Welcome");	
		HAL_Delay(500);
		//
		while(1)
		{
			if(SCAN()!=25)
			{
				goto KEYPAB;
			}
			else{
				goto RFID;
			}
		}
		//keypab begin
		KEYPAB:;

		while(1)
				{
				LCD_LOCATION(2,1);
				LCD_WRITE_STRING(" Press Any Key");
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
				if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)
				{
					HAL_Delay(5000);
					if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)
					{
						CHANGE_PASS();
						break;
					}
				}
				//
				
				uint8_t b=SCAN();
				if(b!=25)
				{
					LCD_LOCATION(2,1);
					LCD_WRITE_STRING("                   ");
					break;
				}
				//
			}

		LCD_LOCATION(1,0);
		LCD_WRITE_STRING("                     ");
		LCD_LOCATION(1,1);
		LCD_WRITE_STRING("Enter Password");
		pass_array(A);
		Flash_Read_Array(address_password_storage, Check,6);
		if(CHECK_ARRAY(Check,A,6)==1)
		{
			
			HAL_Delay(200);
			LCD_LOCATION(2,4);
			LCD_WRITE_STRING("Correct !");
			count=0;
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_Delay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_Delay(200);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_Delay(100);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
			HAL_Delay(500);
			LCD_LOCATION(2,0);
			LCD_WRITE_STRING("                   ");
			LCD_LOCATION(2,1);
			LCD_WRITE_STRING(" Door Unlocked");
			TIM3->CCR2=(0.36*20000)/20; //so ms xung o muc cao_Duty Cycle --> CCR = (Duty_Cycle_ms*ARR)/20ms
			//HAL_Delay(1000);
			HAL_Delay(1500);
			LCD_CLEAR_SCREEN();
			HAL_Delay(1);
			LCD_LOCATION(1,1);
			LCD_WRITE_STRING("Door will lock");
			HAL_Delay(100);
			LCD_LOCATION(2,3);
			LCD_WRITE_STRING("in   second");
			HAL_Delay(100);
			for (int i=9;i>-1;i--)
			{
				LCD_LOCATION(2,6);
				LCD_WRITE_NUMBER(i);
				HAL_Delay(1000);
			}
			TIM3->CCR2=(2.35*20000)/20; //so ms xung o muc cao_Duty Cycle --> CCR = (Duty_Cycle_ms*ARR)/20ms
			//HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
			HAL_Delay(300);
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
			LCD_CLEAR_SCREEN();
			HAL_Delay(1);
		}
		else{
			count++;
			HAL_Delay(200);
			LCD_LOCATION(2,2);
			LCD_WRITE_STRING("Incorrect !!");
			HAL_Delay(500);
			if(count>3)
			{
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_Delay(100);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
				for (int j=0;j<count;j++)
				{
					LCD_CLEAR_SCREEN();
					HAL_Delay(500);
					LCD_LOCATION(1,3);
					LCD_WRITE_STRING("Warning!!!");
					HAL_Delay(500);
				}
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_Delay(count/2*10000);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
				
				goto WW;
			}
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		}
		//
		WW:;
		
		//keypab end	
		
		//////	
		RFID:;	
if(!MFRC522_Request(PICC_REQIDL,str))
		{
			if(!MFRC522_Anticoll(str)){
			/// Do ....
					uint8_t sl=	Flash_Read_uint(address_so_luong_the_storage);
				/// add card
					if(CHECK_ARRAY(master_add,str,5)==1)
					{
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
						HAL_Delay(100);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
						LCD_CLEAR_SCREEN();
						LCD_LOCATION(1,4);
						LCD_WRITE_STRING("Add card");
						HAL_Delay(2000);
						LCD_CLEAR_SCREEN();
						LCD_LOCATION(1,2);
						LCD_WRITE_STRING("Put your card");
						HAL_Delay(500);
						while(1)
							{
									if(!MFRC522_Request(PICC_REQIDL,str))
										{
											if(!MFRC522_Anticoll(str))
												{
												/// Do ....
														
														
													if((CHECK_ARRAY(master_add,str,5)==1)|(CHECK_ARRAY(master_delete,str,5)==1))	
													{
														LCD_CLEAR_SCREEN();
														HAL_Delay(1000);
														goto TT;
													}
													uint8_t ID_check[5];
													
													for(int i=0;i<sl;i++)
													{
															Flash_Read_Array(0x08000000 + ((65+i)*1024), ID_check,5);		
															if(CHECK_ARRAY(ID_check,str,5)==1)
															{
																LCD_LOCATION(2,1);
																LCD_WRITE_STRING("Card was added!");
																HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
																HAL_Delay(300);
																HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
																HAL_Delay(2000);
																LCD_LOCATION(2,0);
																LCD_WRITE_STRING("                 ");		
																	goto GG;
															}
													}
													
														Flash_Erase(0x08000000 + (65+sl)*1024);
														Flash_Write_Array(0x08000000 + (65+sl)*1024,str, 5);
													
														
														Flash_Erase(address_so_luong_the_storage);
														Flash_Write_uint(address_so_luong_the_storage,sl+1);
														LCD_LOCATION(2,2);
														LCD_WRITE_STRING("Successfull!");
														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
														HAL_Delay(100);
														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
														HAL_Delay(100);
														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
														HAL_Delay(100);
														HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
														HAL_Delay(2000);
														LCD_CLEAR_SCREEN();
														HAL_Delay(1);
													
													goto TT;
													GG:;
												///
												}
										}	
							}
					}
					///
					
					// delete_card
						if(CHECK_ARRAY(master_delete,str,5)==1)
					{
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
						HAL_Delay(100);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
						LCD_CLEAR_SCREEN();
						LCD_LOCATION(1,3);
						LCD_WRITE_STRING("Delete card");
						HAL_Delay(2000);
						LCD_CLEAR_SCREEN();
						LCD_LOCATION(1,2);
						LCD_WRITE_STRING("Put your card");
						HAL_Delay(500);
						while(1)
						{
							if(!MFRC522_Request(PICC_REQIDL,str))
							{
								if(!MFRC522_Anticoll(str))
								{
									//
										
											if((CHECK_ARRAY(master_add,str,5)==1)|(CHECK_ARRAY(master_delete,str,5)==1))	
													{
														LCD_CLEAR_SCREEN();
														HAL_Delay(1000);
														goto TT;
													}
											uint8_t ID_check[5];
													for(int i=0;i<sl;i++)
													{
														Flash_Read_Array(0x08000000 + ((65+i)*1024), ID_check,5);
														if(CHECK_ARRAY(str,ID_check,5)==1)
														{
															goto UU;
														}
													}
													LCD_LOCATION(2,1);
													LCD_WRITE_STRING("Card not found!");
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
													HAL_Delay(300);
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
													HAL_Delay(2000);
													LCD_LOCATION(2,1);
													LCD_WRITE_STRING("                   ");
													goto PP;
													UU:;	
													for (int i=0;i<sl;i++)
													{
														Flash_Read_Array(0x08000000 + ((65+i)*1024), ID_check,5);
														if(CHECK_ARRAY(str,ID_check,5)==1)
														{
															Flash_Erase(0x08000000 + ((65+i)*1024));
															for (int j=i;j<sl-1;j++)
															{
																Flash_Read_Array(0x08000000 + ((65+j+1)*1024), ID_check,5);
																Flash_Write_Array(0x08000000 + ((65+j)*1024),ID_check,5);
																Flash_Erase(0x08000000 + ((65+j+1)*1024));
															}
															Flash_Erase(address_so_luong_the_storage);
															Flash_Write_uint(address_so_luong_the_storage,sl-1);
															break;
														}
													}
													LCD_LOCATION(2,0);
													LCD_WRITE_STRING("                ");
													LCD_LOCATION(2,3);
													LCD_WRITE_STRING("Deleted !!");
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
													HAL_Delay(100);
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
													HAL_Delay(100);
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
													HAL_Delay(100);
													HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
													HAL_Delay(2000);
													LCD_CLEAR_SCREEN();
													HAL_Delay(1);
													goto TT;
									//
								}
							}
							PP:;
						}
					}
					//
					// CHECK CARD
						if((CHECK_ARRAY(master_add,str,5)==0)&(CHECK_ARRAY(master_delete,str,5)==0))
					{
						uint8_t ID_check[5];
							for (int i=0;i<sl;i++)
							{
								Flash_Read_Array(0x08000000 + ((65+i)*1024), ID_check,5);		
								if(CHECK_ARRAY(ID_check,str,5)==1)
								{
									LCD_LOCATION(2,3);
									LCD_WRITE_STRING("Card found");
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
									HAL_Delay(100);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
									count=0;
									HAL_Delay(1500);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
									HAL_Delay(100);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
									HAL_Delay(200);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
									HAL_Delay(100);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
									LCD_LOCATION(2,0);
									LCD_WRITE_STRING("                   ");
									LCD_LOCATION(2,1);
									LCD_WRITE_STRING(" Door Unlocked");					
									TIM3->CCR2=(0.36*20000)/20; //so ms xung o muc cao_Duty Cycle --> CCR = (Duty_Cycle_ms*ARR)/20ms
									//HAL_Delay(1000);
									HAL_Delay(1500);
									LCD_CLEAR_SCREEN();
									HAL_Delay(1);
									LCD_LOCATION(1,1);
									LCD_WRITE_STRING("Door will lock");
									HAL_Delay(100);
									LCD_LOCATION(2,3);
									LCD_WRITE_STRING("in   second");
									HAL_Delay(100);
									for (int i=9;i>-1;i--)
									{
											LCD_LOCATION(2,6);
											LCD_WRITE_NUMBER(i);
											HAL_Delay(1000);
									}
									TIM3->CCR2=(2.35*20000)/20; //so ms xung o muc cao_Duty Cycle --> CCR = (Duty_Cycle_ms*ARR)/20ms
									//HAL_Delay(1000);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
									HAL_Delay(300);
									HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
									LCD_CLEAR_SCREEN();
									HAL_Delay(1);
									goto TT;
								}
							}
							LCD_LOCATION(2,1);
							LCD_WRITE_STRING("Card not found");
							HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
							HAL_Delay(300);
							HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
							HAL_Delay(2000);
							LCD_CLEAR_SCREEN();
							HAL_Delay(1);
							TT:;
							
					}
						
			///
			}
		}
/////
				
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
