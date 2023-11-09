/* PWM呼吸灯示例工程 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/pwm.h"                 //pwm头文件
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "driver/hw_timer.h"

#define PWM_PERIOD 1000                 //PWM周期（1000us）
#define CHANNLE_PWM_TOTAL 1             //通道总数
#define CHANNLE_PWM_LED1 0              //PWM通道号
#define CHANNLE_PWM_IO_NUM_LED1 2      //PWM输出引脚号(LED1)

#define GPIO_OUTPUT_PIN_SEL  ((1ULL << CHANNLE_PWM_IO_NUM_LED1) | (1ULL << CHANNLE_PWM_IO_NUM_LED1))


static uint32_t pinNum[CHANNLE_PWM_TOTAL] = {CHANNLE_PWM_IO_NUM_LED1};//PWM通道总数
static uint32_t setDuties[CHANNLE_PWM_TOTAL] = {50};                  //PWM通道周期（1000us）
static int16_t phase[CHANNLE_PWM_TOTAL] = {0};                        //相位

static uint8_t ledDutyFlag = 1;         //呼吸灯标志位
static uint8_t ledDutySteup = 5;        //呼吸灯占空比变化步长
static uint32_t duty = 1000;            //呼吸灯占空比变量
#define TEST_ONE_SHOT    false        // testing will be done without auto reload (one-shot)
#define TEST_RELOAD      true         // testing will be done with auto reload
static os_timer_t os_timer;                    //定义定时器结构体
static const char *TAG = "main";		//格式化输出前缀
static volatile int state = 0;

static void pwm_breathe_led(void *arg)
{

    if(ledDutyFlag == 1)
    {
        duty = duty + ledDutySteup;
        if(duty >= PWM_PERIOD)
            ledDutyFlag = 0;
    }
    else
    {
        duty = duty - ledDutySteup;
        if(duty <= 0)
            ledDutyFlag = 1;
    }
    
    pwm_set_duty(CHANNLE_PWM_LED1, duty);//设置周期
    pwm_start();                         //PWM启动    
}

void hw_timer_callback1(void *timer_arg)
{
    state++;
    ets_printf("test");
    // if(state == 1000000){
    //     gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 0);
    // }

    // printf("sss");
    // ESP_LOGI(TAG, "pirnt x %d",state);
}

void app_main(void)
{
    uart_set_baudrate(UART_NUM_0,115200);//初始化波特率为115200
    ESP_LOGI(TAG,esp_get_idf_version());
    ESP_LOGI(TAG, "Config gpio");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 1);
    ESP_LOGI(TAG, "LED1 blink\n");
	pwm_init(PWM_PERIOD, setDuties, CHANNLE_PWM_TOTAL, pinNum);     //PWM初始化
    pwm_set_phases(phase);               //设置相位
    // pwm_set_duty(CHANNLE_PWM_LED1, duty);//设置周期
	// pwm_start();                         //PWM启动  
	// ESP_LOGI(TAG, "started!!!");
    // os_timer_disarm(&os_timer);          //关闭定时器
	// ESP_LOGI(TAG, "disarm");
    // os_timer_setfn(&os_timer, (os_timer_func_t *)(pwm_breathe_led), NULL);//初始化定时器的回调函数(呼吸灯)
    // os_timer_arm(&os_timer, 100000, 0);      //启动定时器
	// ESP_LOGI(TAG, "start timerarm");


    ESP_LOGI(TAG, "Initialize hw_timer for callback1");
    hw_timer_init(pwm_breathe_led, NULL);
    ESP_LOGI(TAG, "Set hw_timer timing time 100us with reload");
    hw_timer_alarm_us(10000, TEST_RELOAD);
    vTaskDelay(1000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Deinitialize hw_timer for callback1");
    // hw_timer_deinit();




    for(;;)                              
    {
        vTaskDelay(1000 / portTICK_RATE_MS);                       
        ESP_LOGI(TAG,"for:%d",duty);
        // hw_timer_callback1(NULL);
    }
}


