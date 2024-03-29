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

void lora_connect()
{
    uart_setup(UART_NR, UART_TX_PIN, UART_RX_PIN, UART_BAUD_RATE);
    enum Step current_step;
    char response[RESP_LEN];
    int attempts = 0;
    int pos;
    int function_done = 0;
    int result;
    current_step = CONNECT;

    do {
        pos = 0;
        switch (current_step)
        {
            case CONNECT:
                result = connect(attempts, pos, response, current_step);
                if (result == 1)
                    current_step = MODE;
                else
                    function_done = 1;
                break;
            case MODE:
                result = mode(pos, response);
                if (result == 1)
                    current_step = APPKEY;
                else
                    function_done = 1;
                break;
            case APPKEY:
                result = appkey(pos, response);
                if (result == 1)
                    current_step = CLASS;
                else
                    function_done = 1;
                break;
            case CLASS:
                result = class(pos, response);
                if (result == 1)
                    current_step = PORT;
                else
                    function_done = 1;
                break;
            case PORT:
                result = port(pos, response);
                if (result == 1)
                    current_step = JOIN;
                else
                    function_done = 1;
                break;
            case JOIN:
                join(pos, response);
                function_done = 1;
                break;
            default:
                current_step = CONNECT;
                break;
        }
    } while(function_done != 1);
}

int connect(int attempts, int pos, char *response, enum Step current_step)
{
    while (attempts < MAX_ATTEMPTS && current_step == CONNECT)
    {
        uart_send(UART_NR, "AT\r\n");
        sleep_ms(500);

        pos = read_string(response);

        if (pos > 0)
        {
            response[pos] = '\0';
            if (strstr(response, "OK") != NULL)
            {
                printf("%s \n", response);
                return 1;
            }
        }

        attempts++;
        if (attempts >= MAX_ATTEMPTS)
        {
            printf("Module stopped responding\n");
            return 2;
        }
        return 2;
    }
    return 2;
}

int mode(int pos, char *response)
{
    uart_send(UART_NR, "AT+MODE=LWOTAA\r\n");
    sleep_ms(500);

    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        if (strstr(response, "LWOTAA") != NULL)
        {
            printf("%s \n", response);
            return 1;
        }
    }
    else
    {
        printf("Module stopped responding\n");
        return 2;
    }
    return 2;
}

int appkey(int pos, char *response)
{
    uart_send(UART_NR, "AT+KEY=APPKEY,\"e06716984de834e2a38a779b9cd2def3\"\r\n");
    sleep_ms(500);

    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        if (strstr(response, "APPKEY") != NULL)
        {
            printf("%s \n", response);
            return 1;
        }
    }
    else
    {
        printf("Module stopped responding\n");
        return 2;
    }
    return 2;
}

int class(int pos, char *response)
{
    uart_send(UART_NR, "AT+CLASS=A\r\n");
    sleep_ms(500);

    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        printf("%s \n", response);
        return 1;
    }
    else
    {
        printf("Module stopped responding\n");
        return 2;
    }
    return 2;
}

int port(int pos, char *response)
{
    uart_send(UART_NR, "AT+PORT=8\r\n");
    sleep_ms(500);

    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        if (strstr(response, "PORT") != NULL)
        {
            printf("%s \n", response);
            return 1;
        }
        else
        {
            printf("Module stopped responding\n");
            return 2;
        }
    }
    return 2;
}

int join(int pos, char *response)
{
    uart_send(UART_NR, "AT+JOIN\r\n");
    sleep_ms(10000);

    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        printf("Response: %s\n", response);

        if (strstr(response, "Done") != NULL)
        {
            printf("%s \n", response);
            return 1;
        }
        else if (strstr(response, "failed") != NULL)
        {
            printf("Failed to connect to LoRa Module\n");
            return 2;
        }
    }
    else
    {
        printf("Module stopped responding\n");
        return 2;
    }
    return 2;
}

void lora_msg(char *msg)
{
    char response[RESP_LEN];
    int pos;
    uart_send(UART_NR, msg);
    sleep_ms(5000);
    pos = read_string(response);

    if (pos > 0)
    {
        response[pos] = '\0';
        printf("%s \n", response);
    }
    else
    {
        printf("Module stopped responding\n");
    }
}
