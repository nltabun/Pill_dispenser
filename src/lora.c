#include "lora.h"
#include "uart.h"

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

void lora_msg(enum Step, char msg[100])
{
    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);
    const char CMD_OK[] = "AT\r\n";
    const char CMD_MODE[] = "AT+MODE=LWOTAA\r\n";
    const char CMD_APPKEY[] = "AT+KEY=APPKEY,\"e06716984de834e2a38a779b9cd2def3\"";
    const char CMD_CLASS[] = "AT+CLASS=A";
    const char CMD_PORT[] = "AT+PORT=8";
    const char CMD_JOIN[] = "AT+JOIN";
    const char CMD_MSG[] = "AT+MSG=\" msg \"";
    char response[RESP_LEN];
    int attempts = 0;
    int pos = 0;

    enum Step current_step;
    current_step = CONNECT;
    for (int i = 0; i < 7; ++i) {
        switch (current_step) {
            case CONNECT:
                while (attempts < MAX_ATTEMPTS && current_step == CONNECT) {
                    uart_send(UART_NR, CMD_OK);
                    sleep_ms(500);

                    pos = read_string(response);

                    if (pos > 0) {
                        response[pos] = '\0';
                        if (strstr(response, "OK") != NULL) {
                            printf("Connected to LoRa Module\n");
                            current_step = SEND_MSG;
                        }
                    }

                    attempts++;
                    if (attempts >= MAX_ATTEMPTS) {
                        printf("Module not responding\n");
                        current_step = CONNECT;
                    }
                }
                break;
            case MODE:
                uart_send(UART_NR, CMD_MODE);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    current_step = APPKEY;
                } else {
                    printf("Module stopped responding\n");
                    current_step = CONNECT;
                }
                break;
            case APPKEY:
                uart_send(UART_NR, CMD_APPKEY);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    current_step = CLASS;
                } else {
                    printf("Module stopped responding\n");
                    current_step = CONNECT;
                }
                break;
            case CLASS:
                uart_send(UART_NR, CMD_CLASS);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    current_step = PORT;
                } else {
                    printf("Module stopped responding\n");
                    current_step = PORT;
                }
                break;
            case PORT:
                uart_send(UART_NR, CMD_PORT);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    current_step = JOIN;
                } else {
                    printf("Module stopped responding\n");
                    current_step = CONNECT;
                }
                break;
            case JOIN:
                uart_send(UART_NR, CMD_JOIN);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    current_step = SEND_MSG;
                } else {
                    printf("Module stopped responding\n");
                    current_step = CONNECT;
                }
                break;
            case SEND_MSG:
                uart_send(UART_NR, &msg);
                sleep_ms(500);

                pos = read_string(response);

                if (pos > 0) {
                    response[pos] = '\0';
                    printf("The message: %s", response);
                } else {
                    printf("Module stopped responding\n");
                    current_step = 1;
                }
                break;
            default:
                current_step = CONNECT;
                break;
        }
    }
}
