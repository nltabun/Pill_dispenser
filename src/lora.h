#ifndef LORA_H
#define LORA_H

#include "uart.h"
#include "pico/stdlib.h"
#include "string.h"
#include "ctype.h"

#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD_RATE 9600

#define RESP_LEN 100
#define FW_VER_LEN 10
#define DEVEUI_LEN 23
#define DEVEUI_OFFSET 12
#define MAX_ATTEMPTS 5
#define TIMEOUT_MS 500

enum Step
{
    WAIT,
    CONNECT,
    SEND_MSG
} Step;

int read_string(char *response);
void trim_response(char *response, int prefix_len, char *destination, char ignore_c);
void lora_msg(enum Step step);
#endif