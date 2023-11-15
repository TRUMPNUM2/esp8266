 
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
#include "oled.h"
#include "bmp.h"
 

 
 
 
/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/


void app_main()
{
	u_int8_t t=' ';
 
	OLED_Init();
	OLED_ColorTurn(0);//0正常显示，1 反色显示
	delay_ms(20);
	OLED_DisplayTurn(0);//0正常显示 1 屏幕翻转显示
	delay_ms(20);
	while(1){
		OLED_ShowPicture(0,0,128,64,BMP1,1);
		OLED_Refresh();
		delay_ms(500);
		OLED_Clear();
		OLED_ShowChinese(0,0,0,16,1);//中
		OLED_ShowChinese(18,0,1,16,1);//景
		OLED_ShowChinese(36,0,2,16,1);//园
		OLED_ShowChinese(54,0,3,16,1);//电
		OLED_ShowChinese(72,0,4,16,1);//子
		OLED_ShowChinese(90,0,5,16,1);//技
		OLED_ShowChinese(108,0,6,16,1);//术
		OLED_ShowString(8,16,"ZHONGJINGYUAN",16,1);
		OLED_ShowString(20,32,"2014/05/01",16,1);
		OLED_ShowString(0,48,"ASCII:",16,1);
		OLED_ShowString(63,48,"CODE:",16,1);
		OLED_ShowChar(48,48,t,16,1);//显示ASCII字符
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,48,t,3,16,1);
		OLED_Refresh();
		delay_ms(500);
 
 
		printf("\r\n yu \r\n");
		// system_soft_wdt_feed();//喂狗
		os_delay_us(5000);
 
	}
}