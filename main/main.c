/* 
Socket with LED blink Example
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "wifi_service.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "driver/gpio.h"
#include <driver/timer.h>
#define BLINK_GPIO 2 // The GPIO pin number of the LED
bool led_state = false;

#define CONFIG_EXAMPLE_PORT 6888
#define CONFIG_EXAMPLE_KEEPALIVE_IDLE 5
#define CONFIG_EXAMPLE_KEEPALIVE_INTERVAL 5
#define CONFIG_EXAMPLE_KEEPALIVE_COUNT 3

#define PORT CONFIG_EXAMPLE_PORT
#define KEEPALIVE_IDLE CONFIG_EXAMPLE_KEEPALIVE_IDLE
#define KEEPALIVE_INTERVAL CONFIG_EXAMPLE_KEEPALIVE_INTERVAL
#define KEEPALIVE_COUNT CONFIG_EXAMPLE_KEEPALIVE_COUNT

static const char *TAG = "LED_over_WIFI";



static void do_retransmit(const int sock)
{
    char led_str[2];

    while (1)
    {
        // Convert LED state to string representation
        led_str[0] = led_state ? '1' : '0';
        led_str[1] = '\0';

        // Send LED state over socket
        ssize_t num_bytes_sent = write(sock, led_str, strlen(led_str));
        if (num_bytes_sent == -1)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
        else
        {
            ESP_LOGI(TAG, "Sent LED State: %s", led_str);
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second before checking LED state again
    }
}

static void led_task(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 200 / portTICK_PERIOD_MS; // Blink at 5 Hz
    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        gpio_set_level(BLINK_GPIO, 1); // Turn on the LED
        led_state = true;
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Wait for the specified period
        gpio_set_level(BLINK_GPIO, 0);               // Turn off the LED
        led_state = false;
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // Wait for the specified period
    }
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1)
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void app_main(void)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    initialise_wifi();
    xTaskCreate(led_task, "led_task", 4096, (void *)AF_INET, 5, NULL);
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
}
