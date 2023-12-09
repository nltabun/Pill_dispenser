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

void lora_msg()
{

}