
#ifndef __OLED_H
#define __OLED_H 
 
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/hw_timer.h"

//-----------------OLED端口定义---------------- 
 
#define OLED_SCL_Clr() gpio_set_level(GPIO_ID_PIN(5), 0);//SCL
#define OLED_SCL_Set() gpio_set_level(GPIO_ID_PIN(5), 1);
 
#define OLED_SDA_Clr() gpio_set_level(GPIO_ID_PIN(4), 0);//DIN
#define OLED_SDA_Set() gpio_set_level(GPIO_ID_PIN(4), 1);
 
//#define OLED_RES_Clr() GPIO_ResetBits(GPIOA,GPIO_Pin_2)//RES
//#define OLED_RES_Set() GPIO_SetBits(GPIOA,GPIO_Pin_2)
 
 
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
 
void delay_ms(u_int16_t ms);
void OLED_ClearPoint(u_int8_t x,u_int8_t y);
void OLED_ColorTurn(u_int8_t i);
void OLED_DisplayTurn(u_int8_t i);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u_int8_t dat);
void OLED_WR_Byte(u_int8_t dat,u_int8_t mode);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_Refresh(void);
void OLED_Clear(void);
void OLED_DrawPoint(u_int8_t x,u_int8_t y,u_int8_t t);
void OLED_DrawLine(u_int8_t x1,u_int8_t y1,u_int8_t x2,u_int8_t y2,u_int8_t mode);
void OLED_DrawCircle(u_int8_t x,u_int8_t y,u_int8_t r);
void OLED_ShowChar(u_int8_t x,u_int8_t y,u_int8_t chr,u_int8_t size1,u_int8_t mode);
void OLED_ShowChar6x8(u_int8_t x,u_int8_t y,u_int8_t chr,u_int8_t mode);
void OLED_ShowString(u_int8_t x,u_int8_t y,char *chr,u_int8_t size1,u_int8_t mode);
void OLED_ShowNum(u_int8_t x,u_int8_t y,u_int32_t num,u_int8_t len,u_int8_t size1,u_int8_t mode);
void OLED_ShowChinese(u_int8_t x,u_int8_t y,u_int8_t num,u_int8_t size1,u_int8_t mode);
void OLED_ScrollDisplay(u_int8_t num,u_int8_t space,u_int8_t mode);
void OLED_ShowPicture(u_int8_t x,u_int8_t y,u_int8_t sizex,u_int8_t sizey,unsigned char BMP[],u_int8_t mode);
void OLED_Init(void);
 
#endif
