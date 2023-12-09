#include "lora.h"

int read_string(char *response)
{
    int pos = uart_read(UART_NR, (uint8_t *)response, RESP_LEN);
    if (pos > 0)
    {
        if (strchr(response, '\n') != NULL)
            return pos;
    }

    return 0;
}

void trim_response(char *response, int prefix_len, char *destination, char ignore_c)
{
    int dest_idx = 0;
    for (int resp_idx = 0; resp_idx < DEVEUI_LEN; resp_idx++)
    {
        if (response[resp_idx + prefix_len] != ignore_c)
        {
            destination[dest_idx] = tolower(response[resp_idx + prefix_len]);
            dest_idx++;
        }
    }
    destination[dest_idx] = '\0';
}

void lora_msg(enum Step)
{
    enum Step current_step;
    current_step = WAIT;
    switch (current_step) {
        case WAIT:
            while (gpio_get(BUTTON_PIN))
                sleep_ms(10);

            while (!gpio_get(BUTTON_PIN))
                sleep_ms(10);
            sleep_ms(100);

            attempts = 0;
            pos = 0;
            current_step = CONNECT;
            break;
        case CONNECT:
            while (attempts < MAX_ATTEMPTS && current_step == CONNECT) {
                uart_send(UART_NR, CMD_OK);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    if (strstr(response, "OK") != NULL) {
                        printf("Connected to LoRa Module\n");
                        current_step = READ_FIRMWARE;
                    }
                }

                attempts++;
                if (attempts >= MAX_ATTEMPTS) {
                    printf("Module not responding\n");
                    current_step = WAIT;
                }
            }
            break;
        case READ_FIRMWARE:
            uart_send(UART_NR, CMD_VER);
            sleep_ms(500);

            pos = read_string(response);

            if (pos > 0) {
                response[pos] = '\0';
                trim_response(response, FIRMWARE_PREFIX_LEN, fw_version, ' ');
                printf("Firmware version: %s", fw_version);
                current_step = READ_DEVEUI;
            } else {
                printf("Module stopped responding\n");
                current_step = WAIT;
            }
            break;
    }
}
