#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 64

int current_temp = 23;
int max_temp = 50; // Variável para guardar a temperatura máxima

void update_random_temperature() {
    current_temp = 10 + rand() % 41; // 10 a 50 (inclusive)
}

void send_ack() {
    int sum = 'E' + 'o';    // 'E' comando erro, 'o' código OK
    int checksum = sum % 256;
    printf("#Eo%03d!\n", checksum);
    fflush(stdout);
}


void send_error(char error_type) {
    int sum = 'E' + error_type;
    int checksum = sum % 256;
    printf("#E%c%03d!\n", error_type, checksum);
    fflush(stdout);
}


void send_temperature_response(int temp) {
    char temp_str[4];
    snprintf(temp_str, sizeof(temp_str), "%03d", temp);

    int sum = 'c';
    for (int i = 0; i < 3; i++) {
        sum += temp_str[i];
    }
    int checksum = sum % 256;

    printf("#c%s%03d!\n", temp_str, checksum);
    fflush(stdout);
}

int calculate_checksum(char cmd, const char *data) {
    int sum = cmd;
    for (int i = 0; data[i] != '\0'; i++)
        sum += data[i];
    return sum % 256;
}

void process_uart_frame(const char *frame) {
    printf("DEBUG: Frame recebido: '%s'\n", frame);
    fflush(stdout);

    if (frame[0] != '#' || frame[strlen(frame) - 1] != '!') {
        send_error('f');
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

    printf("DEBUG: cmd='%c', data='%s', checksum recebido=%d, checksum calculado=%d\n",
           cmd, data, received_cs, computed_cs);
    fflush(stdout);

    if (received_cs != computed_cs) {
        send_error('s');
        return;
    }

    switch (cmd) {
        case 'C':
            update_random_temperature();
            send_temperature_response(current_temp);
            break;

        case 'M': {
            // Tenta converter data para inteiro (temperatura máxima)
            int new_max = atoi(data);
            if (new_max < 0 || new_max > 999) {
                send_error('v'); // erro de valor
                return;
            }
            max_temp = new_max;
            printf("DEBUG: Temperatura máxima definida para %d\n", max_temp);
            fflush(stdout);
            send_ack();
            break;
        }

        case 'S':
            printf("Configuração do controlador recebida: %s\n", data);
            fflush(stdout);
            send_ack();
            break;

        default:
            send_error('i');
            break;
    }
}

int main() {
    srand(time(NULL));  // inicializa semente do rand

    char buffer[BUFFER_SIZE];
    int index = 0;
    char c;

    printf("UART Simulada: Insira comandos no formato #CMD DATA CS!\n");
    fflush(stdout);

    while (1) {
        c = getchar();

        if (c == '\r' || c == ' ')
            continue;

        if (c == '\n') {
            // ENTER foi pressionado
            // Verifica se o frame está completo (termina com '!')
            if (index == 0) {
                // Nada no buffer, ignora
                continue;
            }

            if (buffer[index - 1] != '!') {
                // Erro de framing: buffer tem algo, mas não termina em '!'
                buffer[index] = '\0'; // fecha string
                send_error('f');      // envia framing error
                printf("DEBUG: Framing error - buffer não termina em '!': '%s'\n", buffer);
                fflush(stdout);
                index = 0;            // reseta buffer
                continue;
            }

            // Se termina com '!', processa normalmente
            buffer[index] = '\0';
            process_uart_frame(buffer);
            index = 0;
            continue;
        }

        // Se ainda não recebeu ENTER ou !, adiciona caractere no buffer
        buffer[index++] = c;

        // Se receber '!' processa frame imediatamente
        if (c == '!') {
            buffer[index] = '\0';
            process_uart_frame(buffer);
            index = 0;
        }

        if (index >= BUFFER_SIZE - 1) {
            printf("DEBUG: Buffer overflow, resetando\n");
            fflush(stdout);
            index = 0;
        }
    }

    return 0;
}
