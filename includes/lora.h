#ifndef LORA_H
#define LORA_H

#include "uart.h"
#include "pico/stdlib.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"

#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD_RATE 9600
#define RESP_LEN 200
#define MAX_ATTEMPTS 5

enum Step
{
    CONNECT,
    MODE,
    APPKEY,
    CLASS,
    PORT,
    JOIN,
    SEND_MSG
};

int read_string(char *response);
void lora_msg(char *msg);
int Connect(int attempts, int pos, char *response, enum Step current_step);
int mode(int pos, char *response);
int appkey(int pos, char *response);
int class(int pos, char *response);
int port(int pos, char *response);
int join(int pos, char *response);
void message(char *msg, int pos, char *response);

#endif