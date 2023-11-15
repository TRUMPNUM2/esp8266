/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "FreeRTOS.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sys/socket.h"
#include <netdb.h>
#include "driver/hw_timer.h"


#define TAG "example"


static u_int8_t isconnect = 0;

static void initialize_nvs()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

static void initialize_console()
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    uart_config_t uart_config = {
        .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
    };
    ESP_ERROR_CHECK(uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);
}

/** Arguments used by 'join' function */
static struct
{
    struct arg_str *ip_host;
    struct arg_int *port;
    struct arg_str *uri;
    struct arg_end *end;
} ip_args;

struct sockaddr_in server;
#define TEST_RELOAD      true         // testing will be done with auto reload


void establish_conect(void* args){
    static int initnum = 0;
    initnum++;
    ESP_LOGI(TAG,"%d",initnum);    
}

void internate_request(int argc, char **argv)
{

    struct hostent *entry;
    /*get addr info for hostname*/

    int nerrors = arg_parse(argc, argv, (void **)&ip_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, ip_args.end, argv[0]);
        return;
    }
    ESP_LOGI(__func__, "getHostName '%s'",
             ip_args.ip_host->sval[0]);

    // char *hostname = ip_args.ip_host->sval[0];
    char *hostname = ip_args.ip_host->sval[0];

    do
    {
        entry = gethostbyname(hostname);
        vTaskDelay(500 / portTICK_RATE_MS);
    } while (entry == NULL);

    ESP_LOGI(TAG, "get target IP is %d.%d.%d.%d", (unsigned char)((((struct in_addr *)(entry->h_addr))->s_addr & 0x000000ff) >> 0),
             (unsigned char)((((struct in_addr *)(entry->h_addr))->s_addr & 0x0000ff00) >> 8),
             (unsigned char)((((struct in_addr *)(entry->h_addr))->s_addr & 0x00ff0000) >> 16),
             (unsigned char)((((struct in_addr *)(entry->h_addr))->s_addr & 0xff000000) >> 24));
    ESP_LOGI(TAG, "建立连接");
   
}

esp_err_t register_internate()
{
    // esp_console_cmd_t command = {
    //     .command = "getip",
    //     .help = "getip",
    //     .func = &internate_request};

    ip_args.ip_host = arg_str0(NULL, NULL, "<host>", "ip or domain");
    ip_args.port = arg_int0(NULL, NULL, "<p>", "端口");
    ip_args.port->ival[0] = 8078; // set default value
    ip_args.uri = arg_str1(NULL, NULL, "<uri>", "path");
    ip_args.end = arg_end(2);
    const esp_console_cmd_t ipget_cmd = {
        .command = "getip",
        .help = "getip hostname",
        .hint = NULL,
        .func = &internate_request,
        .argtable = &ip_args};

    return esp_console_cmd_register(&ipget_cmd);
}


void app_main()
{
    // uart_set_baudrate(UART_NUM_0, 9600); // 初始化波特率为115200
    initialize_nvs();

    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
    register_system();
    register_wifi();
    register_internate();
    // bool connected = wifi_join("jail",
    //                            "12345678",
    //                            1000);
    // if (!connected)
    // {
    //     isconnect =0;
    //     ESP_LOGW(__func__, "Connection timed out");
    // }
    // isconnect = 1;
    vTaskDelay(10000 / portTICK_RATE_MS);

    // ESP_LOGI(TAG, "Initialize hw_timer for callback1");
    // hw_timer_init(establish_conect, NULL);
    // ESP_LOGI(TAG, "Set hw_timer timing time 100us with reload");
    // hw_timer_alarm_us(10000, TEST_RELOAD);
    // vTaskDelay(1000 / portTICK_RATE_MS);
    // ESP_LOGI(TAG, "Deinitialize hw_timer for callback1");
    

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char *prompt = LOG_COLOR_I "esp8266> " LOG_RESET_COLOR;

    printf("\n"
           "This is an example of ESP-IDF console component.\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n");

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status)
    { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "esp8266> ";
#endif // CONFIG_LOG_COLORS
    }

    /* Main loop */
    while (true)
    {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char *line = linenoise(prompt);
        if (line == NULL)
        { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
        {
            printf("Unrecognized command\n");
        }
        else if (err == ESP_ERR_INVALID_ARG)
        {
            // command was empty
        }
        else if (err == ESP_OK && ret != ESP_OK)
        {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        }
        else if (err != ESP_OK)
        {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}
