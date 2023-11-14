/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

typedef struct dht11_isr_arg_t
{
    uint8_t status;           // 0-未发送触发信号，1-已发送触发信号
    uint8_t num;              // 已采集的波形数量，有效数据是40bit
    uint8_t bit_status;       // 采集1个波形的状态，0-接收到上升沿开始，1-接收到下降沿结束采集
    EventGroupHandle_t event; // 事件组
    uint32_t clk;             // 用于记录波形时间
    uint64_t data;            // 最终采集到的数据
} dht11_isr_arg_t;

/**
 * @brief 定时器中断函数,5个clk=1us
 */
void timer_isr(void *arg)
{
    gpio_set_level(GPIO_NUM_2, !gpio_get_level(GPIO_NUM_2));

    dht11_isr_arg_t *isr_arg = (dht11_isr_arg_t *)arg;
    BaseType_t base;
    xEventGroupSetBitsFromISR(isr_arg->event, 0x01, &base);
}

/**
 * @brief 初始化定时器
 */
void timer_init(void *arg)
{
    hw_timer_init(timer_isr, arg);        // 注册定时器中断函数
    hw_timer_set_reload(true);            // 允许重载，即循环定时，不止一次
    hw_timer_set_clkdiv(TIMER_CLKDIV_16); // 16分频
    hw_timer_set_intr_type(TIMER_EDGE_INT);
    hw_timer_set_load_data((TIMER_BASE_CLK >> hw_timer_get_clkdiv())); // 5000000
    hw_timer_enable(true);
}

/**
 * @brief 初始化LED
 */
void led_init()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1 << GPIO_NUM_2;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

/**
 * @brief GPIO5的中断函数，上升沿和下降沿均触发
 */
void dht11_isr(void *arg)
{
    dht11_isr_arg_t *isr_arg = (dht11_isr_arg_t *)arg;
    // 未发送触发信号直接退出
    if (0 == isr_arg->status)
        return;

    // 上升沿开始计时，下降沿结束计时并将数据写入
    switch (isr_arg->bit_status)
    {
    case 0:
        if (1 == gpio_get_level(GPIO_NUM_5))
        {
            isr_arg->clk = hw_timer_get_count_data();
            isr_arg->bit_status = 1;
        }
        break;
    case 1:
        if (0 == gpio_get_level(GPIO_NUM_5))
        {
            isr_arg->data = isr_arg->data << 1;
            // 防止两次计时中间经历了定时器重载，实际上在定时器1s中断的情况下基本不存在，因为获取全部41bit数据大概只需要4.1ms
            if (isr_arg->clk < hw_timer_get_count_data())
            {
                isr_arg->clk += (TIMER_BASE_CLK >> hw_timer_get_clkdiv());
            }
            // 高电平超过40us即认定为数据1
            if (40 * 5 <= (isr_arg->clk - hw_timer_get_count_data()))
            {
                isr_arg->data |= 0x01;
            }
            isr_arg->num++;
            isr_arg->bit_status = 0;
            // 40bit数据加上触发信号的响应，共41bit信号
            if (41 <= isr_arg->num)
            {
                BaseType_t base;
                xEventGroupSetBitsFromISR(isr_arg->event, 0x02, &base);
            }
        }
        break;
    default:
        break;
    }
}

void app_main()
{
    dht11_isr_arg_t isr_arg;
    isr_arg.event = xEventGroupCreate();
    // 因data线同时需要输入输出，配置为开漏并使用内部上拉
    gpio_config_t conf;
    conf.mode = GPIO_MODE_OUTPUT_OD;
    conf.intr_type = GPIO_INTR_ANYEDGE;
    conf.pin_bit_mask = 1 << GPIO_NUM_5;
    conf.pull_down_en = 0;
    conf.pull_up_en = 1;
    gpio_config(&conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, dht11_isr, &isr_arg);

    led_init();
    timer_init(&isr_arg);

    while (1)
    {
        // 等待定时器事件产生，发送触发信号
        EventBits_t bits = xEventGroupWaitBits(isr_arg.event, 0x03, pdTRUE, pdFALSE, portMAX_DELAY);
        if (0x01 != (bits | 0x01))
            continue;
        isr_arg.data = 0;
        isr_arg.status = 0;
        isr_arg.bit_status = 0;
        isr_arg.clk = 0;
        isr_arg.num = 0;
        gpio_set_level(GPIO_NUM_5, 0);
        os_delay_us(20000);
        gpio_set_level(GPIO_NUM_5, 1);
        isr_arg.status = 1;
        os_delay_us(30);
        printf("wait dht11 init data\n");

        // 等待数据采集完毕
        bits = xEventGroupWaitBits(isr_arg.event, 0x03, pdTRUE, pdFALSE, portMAX_DELAY);
        // 防止DHT11异常，当再次接收到定时器事件时重置，间隔1s足够DH11正常响应
        if (0x02 != (bits | 0x02))
            continue;
        uint8_t *data = &(isr_arg.data);
        for (int i = 0; i < 8; i++)
        {
            printf("data[%d] = %d ", i, data[i]);
        }
        if (data[0] == (data[1] + data[2] + data[3] + data[4]))
        {
            printf("\nhumidity = %d.%d temperature = %d.%d\n", data[4], data[3], data[2], data[1]);
        }
    }
}
