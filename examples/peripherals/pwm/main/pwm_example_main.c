/* pwm example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/pwm.h"

#include "driver/hw_timer.h"
#include "rom/ets_sys.h"




#define PWM_0_OUT_IO_NUM   2
#define PWM_1_OUT_IO_NUM   13
#define PWM_2_OUT_IO_NUM   14
#define PWM_3_OUT_IO_NUM   15

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD    (1000)

static const char *TAG = "pwm_example";
os_timer_t os_timer_1;

// pwm pin number
const uint32_t pin_num[4] = {
    PWM_0_OUT_IO_NUM,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
    PWM_3_OUT_IO_NUM
};

// duties table, real_duty = duties[x]/PERIOD
uint32_t duties[4] = {
    0, 1000, 1000, 1000,
};

// phase table, delay = (phase[x]/360)*PERIOD
float phase[4] = {
    0, 0, 90.0, -90.0,
};

void OS_Timer_1_Cb(void);//定时器回调函数
void OS_Timer_Init(uint32_t ms,bool repeat_flag);//定时器初始化函数


void OS_Timer_1_Cb(void)
{
	static uint8_t direction=0;
	int span = 10;//每次的增量或减量

	if(direction==0 && duties[0]<=1000)//占空比增大
	   {
		    duties[0]+=span;
	    }
	if(direction==1 && duties[0]>=0)//占空比减小
		{
			duties[0]-=span;
		}
	if(duties[0]>1000)//改变方向
	    {
		  direction=1;
	    }
	if(duties[0]<=span)//改变方向
		{
		  direction=0;
		}
	pwm_set_duty(duties[0],0);//设置占空比
	pwm_start();
}


//软件定时器配置初始化
void OS_Timer_Init(uint32_t ms,bool repeat_flag)
{
	os_timer_disarm(&os_timer_1);//关闭软件定时器
	os_timer_setfn(&os_timer_1,(os_timer_func_t*)OS_Timer_1_Cb,NULL);//注册软件定时器回调函数
	os_timer_arm(&os_timer_1,ms,repeat_flag);//打开软件定时器，设置定时周期，设置是否自动重装
}


void app_main()
{
    pwm_init(PWM_PERIOD, duties, 4, pin_num);
    pwm_set_phases(phase);
    // pwm_start();
    int16_t count = 0;


    OS_Timer_Init(10,1);//10ms改变一次占空比,重装载


    // while (1) {
    //     if (count == 20) {
    //         // channel0, 1 output hight level.
    //         // channel2, 3 output low level.
    //         pwm_stop(0x3);
    //         ESP_LOGI(TAG, "PWM stop\n");
    //     } else if (count == 30) {
    //         pwm_start();
    //         ESP_LOGI(TAG, "PWM re-start\n");
    //         count = 0;
    //     }

    //     count++;
    //     vTaskDelay(1000 / portTICK_RATE_MS);
    // }
}

