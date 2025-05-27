#include <zephyr/kernel.h>
#include <zephyr/console/console.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdlib.h>
#include "rtdb.h"

#define UART_STACK_SIZE 1024
#define UART_PRIORITY 7

static void send_ack(void);
static void send_error(char code);
static void send_temperature_response(int temp);
static int calculate_checksum(char cmd, const char *data);

/**
 * Processa uma string de comando UART no formato: #CMDDATA###!
 */
static void process_uart_frame(const char *frame)
{
    if (frame[0] != '#' || frame[strlen(frame) - 1] != '!') {
        send_error('f');  // framing error
        return;
    }

    char cmd = frame[1];
    char data[32];
    char cs_str[4];

    int data_len = strlen(frame) - 5; // exclui #, CMD, 3 dígitos CS, !
    if (data_len < 0 || data_len > 30) {
        send_error('f');
        return;
    }

    strncpy(data, &frame[2], data_len);
    data[data_len] = '\0';

    strncpy(cs_str, &frame[2 + data_len], 3);
    cs_str[3] = '\0';
    int received_cs = atoi(cs_str);

    int computed_cs = calculate_checksum(cmd, data);
    if (received_cs != computed_cs) {
        send_error('s'); // checksum error
        return;
    }

    switch (cmd) {
        case 'C': {
            int temp = rtdb_get_current_temp();
            send_temperature_response(temp);
            break;
        }
        case 'M': {
            int max_temp = atoi(data); // Assumes 3 digits
            // TODO: adicionar max_temp na RTDB se quiseres controlar
            send_ack();
            break;
        }
        case 'S': {
            // Exemplo: S100050025 => Kp=100, Ti=50, Td=25
            // Aqui só imprimimos. Podes guardar na RTDB.
            int kp = atoi(&data[0]);
            int ti = atoi(&data[3]);
            int td = atoi(&data[6]);
            printk("Controller params set: Kp=%d, Ti=%d, Td=%d\n", kp, ti, td);
            send_ack();
            break;
        }
        default:
            send_error('i'); // invalid command
            break;
    }
}

static int calculate_checksum(char cmd, const char *data)
{
    int sum = cmd;
    for (int i = 0; data[i] != '\0'; i++)
        sum += data[i];
    return sum % 256;
}

static void send_ack(void)
{
    int cs = ('E' + 'o') % 256;
    printk("#Eo%03d!\n", cs);
}

static void send_error(char code)
{
    int cs = ('E' + code) % 256;
    printk("#E%c%03d!\n", code, cs);
}

static void send_temperature_response(int temp)
{
    int cs = ('c' + (temp / 100 % 10) + (temp / 10 % 10) + (temp % 10)) % 256;
    printk("#c%03d%03d!\n", temp, cs);
}

/**
 * Task UART: lê comandos em ASCII via console e processa frames
 */
void uart_task(void)
{
    char buffer[64];
    int index = 0;
    char c;

    console_init();
    printk("UART task started. Awaiting commands...\n");

    while (1) {
        c = console_getchar();

        if (index == 0 && c != '#')
            continue;

        buffer[index++] = c;

        if (c == '!' || index >= sizeof(buffer) - 1) {
            buffer[index] = '\0';
            process_uart_frame(buffer);
            index = 0;
        }
    }
}

// Criação da thread da UART
K_THREAD_DEFINE(uart_tid, UART_STACK_SIZE, uart_task, NULL, NULL, NULL, UART_PRIORITY, 0, 0);
