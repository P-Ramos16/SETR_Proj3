#include <zephyr/kernel.h>
#include <zephyr/console/console.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdlib.h>

#define UART_STACK_SIZE 1024
#define UART_PRIORITY 7
#define BUFFER_SIZE 64

// Função fictícia que obtém a temperatura real do sensor (deves implementar)
extern int rtdb_get_current_temp(void);

// Enviar ACK com checksum correto
static void send_ack(void) {
    int cs = ('E' + 'o') % 256;
    printk("#Eo%03d!\n", cs);
}

// Enviar erro com código e checksum correto
static void send_error(char code) {
    int cs = ('E' + code) % 256;
    printk("#E%c%03d!\n", code, cs);
}

// Enviar temperatura atual com checksum correto
static void send_temperature_response(int temp) {
    char temp_str[4];
    snprintf(temp_str, sizeof(temp_str), "%03d", temp);

    int sum = 'c';
    for (int i = 0; i < 3; i++) {
        sum += temp_str[i];
    }
    int cs = sum % 256;

    printk("#c%s%03d!\n", temp_str, cs);
}

// Calcula checksum do comando
static int calculate_checksum(char cmd, const char *data) {
    int sum = cmd;
    for (int i = 0; data[i] != '\0'; i++) {
        sum += data[i];
    }
    return sum % 256;
}

// Processa uma string de comando UART no formato #CMD DATA CS!
static void process_uart_frame(const char *frame) {
    if (frame[0] != '#' || frame[strlen(frame) - 1] != '!') {
        send_error('f');  // framing error
        return;
    }

    char cmd = frame[1];
    char data[32] = {0};
    char cs_str[4] = {0};
    int data_len;

    if (cmd == 'C') {
        // Comando #Cyyy! sem dados
        data_len = 0;
        strncpy(cs_str, &frame[2], 3);
        cs_str[3] = '\0';
    } else {
        data_len = strlen(frame) - 6;
        if (data_len < 0 || data_len > 30) {
            send_error('f');
            return;
        }
        strncpy(data, &frame[2], data_len);
        data[data_len] = '\0';
        strncpy(cs_str, &frame[2 + data_len], 3);
        cs_str[3] = '\0';
    }

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
            // Exemplo: define temperatura máxima — adaptar se quiseres guardar
            int max_temp = atoi(data);
            // TODO: armazenar max_temp se necessário
            send_ack();
            break;
        }

        case 'S': {
            // Configura parâmetros do controlador, exemplo: Kp, Ti, Td
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

// Task UART: lê comandos ASCII via console e processa frames
void uart_task(void)
{
    char buffer[BUFFER_SIZE];
    int index = 0;
    char c;

    console_init();
    printk("UART task started. Awaiting commands...\n");

    while (1) {
        c = console_getchar();

        if (index == 0 && c != '#') {
            continue;
        }

        buffer[index++] = c;

        if (c == '!') {
            buffer[index] = '\0';
            process_uart_frame(buffer);
            index = 0;
        }

        if (index >= BUFFER_SIZE - 1) {
            printk("Buffer overflow, resetting buffer\n");
            index = 0;
        }
    }
}

// Cria thread UART
K_THREAD_DEFINE(uart_tid, UART_STACK_SIZE, uart_task, NULL, NULL, NULL, UART_PRIORITY, 0, 0);
