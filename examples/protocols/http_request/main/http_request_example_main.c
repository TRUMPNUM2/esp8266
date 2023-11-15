/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <netdb.h>
#include <sys/socket.h>
#define CHANNLE_PWM_IO_NUM_LED1 2 // PWM输出引脚号(LED1)

#define GPIO_OUTPUT_PIN_SEL ((1ULL << CHANNLE_PWM_IO_NUM_LED1) | (1ULL << CHANNLE_PWM_IO_NUM_LED1))
/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "106.52.163.25"
#define WEB_PORT 8182
#define WEB_URL "http://106.52.163.25:8182/"

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
                             "Host: " WEB_SERVER "\r\n"
                             "User-Agent: esp-idf/1.0 esp32\r\n"
                             "\r\n";
void printCharLigth(char *s, int size);

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];
    // char *log = "init";
    // printCharLigth(&log, strlen(log));
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    while (1)
    {
        int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
        {
            char log[] = {0XFF, 0XFF, 0X00, 0XFF};
            printCharLigth(log, 4);
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");

        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                       sizeof(receiving_timeout)) < 0)
        {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        static flag = 0;
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            for (int i = 0; i < r; i++)
            {

                putchar(recv_buf[i]);
            }
            
            gpio_set_level(CHANNLE_PWM_IO_NUM_LED1,0);
        } while (r > 0);

        // printCharLigth(recv_buf, 64);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d\r\n", r, errno);
        close(s);
        for (int countdown = 10; countdown >= 0; countdown--)
        {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void printBinary(char c)
{
    // 从最高位到最低位逐位打印字符c的二进制表示
    for (int i = 7; i >= 0; --i)
    {
        // 使用位运算检查每一位是否为1
        if ((c >> i) & 1)
        {
            // printf("1");
            gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            // printf("0");
            gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 1);
}

void printCharLigth(char *s, int size)
{
    for (int i = 0; i < size; i++)
    {
        char data = *(s + i);
        printBinary(data);
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    //
    gpio_set_level(CHANNLE_PWM_IO_NUM_LED1, 1);
    // char *log = "stessssss";
    // printCharLigth(&log, strlen(log));

    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(&http_get_task, "http_get_task", 16384, NULL, 5, NULL);
}
