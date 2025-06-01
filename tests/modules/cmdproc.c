/**
 * @file cmdProcessor.c
 * @brief Implements the command processor for UART communication with RTDB.
 *
 * This module handles parsing of incoming UART commands, processing requests (such as 
 * reading temperature, setting parameters, toggling verbosity), and sending structured responses
 * or acknowledgments back to the UART buffer.
 * 
 * \author Pedro Ramos, n.º 107348
 * \author Rafael Morgado, n.º 104277
 * \date 01/06/2025
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>  
#include <time.h>    
#include "cmdproc.h"
#include "rtdb.h"

/* Internal variables */
/* Used as part of the UART emulation */
static unsigned char UARTRxBuffer[UART_RX_SIZE];    /**< UART receive buffer */
static unsigned char rxBufLen = 0;                  /**< Length of received buffer */

static unsigned char UARTTxBuffer[UART_TX_SIZE];    /**< UART transmit buffer */
static unsigned char txBufLen = 0;                  /**< Length of transmit buffer */


/* === Function Implementations === */

/**
 * @brief Processes received UART commands and generates appropriate responses.
 * 
 * Supported commands:
 *  - #C...!: Get current temperature.
 *  - #D...!: Get desired temperature.
 *  - #M...!: Set desired temperature.
 *  - #S...!: Set PID parameters.
 *  - #V...!: Toggle verbose mode.
 *
 * @return int Status code:
 *         -  0: Success
 *         - -1: Empty buffer
 *         - -2: Invalid command
 *         - -3: Checksum error
 *         - -4: Framing error
 */
int cmdProcessor(void) {
    int i;

    char sensorStr[12];
    unsigned char checksumBuffer[256];
    int chksumIdx = 0;
    int checksum = 0;
    char checksumStr[5];

    
    /* Detect empty cmd string */
    if(rxBufLen == 0)
        return -1;

    /* Find index of SOF */
    for(i = 0; i < rxBufLen; i++) {
        if(UARTRxBuffer[i] == SOF_SYM) {
            break;
        }
    }

    /* If a SOF was found, look for commands */
    if(i < rxBufLen) {
        

        int msgCheckSum = atoi(UARTRxBuffer + strlen(UARTRxBuffer) - 4);

        if(UARTRxBuffer[rxBufLen-1] != EOF_SYM) {
            return -4;
        }

        switch(UARTRxBuffer[i+1]) { 
            
            //  Responds as #cxxxyyy!
            case 'C':
                if(UARTRxBuffer[i+5] != EOF_SYM) {
                    //  Send bad framing ACK
                    send_ack((int)1);
                    return -4;
                }

                // Checksum de entrada: apenas sobre CMD ('C')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    //  Send bad checksum ACK
                    send_ack(2);
                    return -3;
                }

                checksumBuffer[chksumIdx++] = 'c';


                int current = rtdb_get_current_temp();

                sprintf(sensorStr, "%02d", abs(current));

                checksumBuffer[chksumIdx++] = 't';
                checksumBuffer[chksumIdx++] = (current >= 0) ? '+' : '-';

                for (int k = 0; sensorStr[k] != '\0'; k++) {
                    checksumBuffer[chksumIdx++] = sensorStr[k];
                }

                for (int k = 0; k < chksumIdx; k++) {
                    checksum += checksumBuffer[k];
                }
                checksum = checksum % 256;
                
                snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);

                txChar('#');
                for (int k = 0; k < chksumIdx; k++) {
                    txChar(checksumBuffer[k]);
                }
                txChar(checksumStr[0]);
                txChar(checksumStr[1]);
                txChar(checksumStr[2]);
                txChar('!');

                rxBufLen = 0;
                return 0;

            //  Responds as #dxxxyyy!
            case 'D':
                if(UARTRxBuffer[i+5] != EOF_SYM) {
                    //  Send bad framing ACK
                    send_ack(1);
                    return -4;
                }

                // Checksum de entrada: apenas sobre CMD ('C')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    //  Send bad checksum ACK
                    send_ack(2);
                    return -3;
                }

                checksumBuffer[chksumIdx++] = 'd';


                int desired = rtdb_get_desired_temp();

                sprintf(sensorStr, "%02d", abs(desired));

                checksumBuffer[chksumIdx++] = 't';
                checksumBuffer[chksumIdx++] = (desired >= 0) ? '+' : '-';

                for (int k = 0; sensorStr[k] != '\0'; k++) {
                    checksumBuffer[chksumIdx++] = sensorStr[k];
                }

                for (int k = 0; k < chksumIdx; k++) {
                    checksum += checksumBuffer[k];
                }
                checksum = checksum % 256;
                
                snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);

                txChar('#');
                for (int k = 0; k < chksumIdx; k++) {
                    txChar(checksumBuffer[k]);
                }
                txChar(checksumStr[0]);
                txChar(checksumStr[1]);
                txChar(checksumStr[2]);
                txChar('!');

                rxBufLen = 0;
                return 0;


            //  Responds like #Mxxxyyy!
            case 'M':  
                if(UARTRxBuffer[i+8] != EOF_SYM) {
                    //  Send bad framing ACK
                    send_ack(1);
                    return -4;
                }

                // Checksum de entrada: apenas sobre CMD ('M') + DATA ('xxx')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    //  Send bad checksum ACK
                    send_ack(2);
                    return -3;
                }

                // Ler o valor recebido
                char setTempStr[2] = {0}; // Buffer for 2 digits
                setTempStr[0] = UARTRxBuffer[i+3];
                setTempStr[1] = UARTRxBuffer[i+4];

                int intendedTemperature = atoi(setTempStr);

                if (UARTRxBuffer[i+2] == '-') {
                    intendedTemperature = -intendedTemperature;
                }

                rtdb_set_desired_temp(intendedTemperature);

                //  Send good ACK
                send_ack(0);

                rxBufLen = 0;  // clean buffer
                return 0;


            //  Sets PID parameters as #Spx.xxyyy!
            case 'S':
                if(UARTRxBuffer[i+10] != EOF_SYM) {
                    //  Send bad framing ACK
                    send_ack(1);
                    return -4;
                }

                // Checksum de entrada: apenas sobre CMD ('M') + DATA ('px.xix.xxdx.xx')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    //  Send bad checksum ACK
                    send_ack(2);
                    return -3;
                }

                //  Get the current PID parameters
                float Kp, Ki, Kd;
                rtdb_get_PID_params(&Kp, &Ki, &Kd);

                char setPIDStr[4]; // Buffer for 4 digits
                setPIDStr[0] = UARTRxBuffer[i+2];
                setPIDStr[1] = UARTRxBuffer[i+3];
                setPIDStr[2] = UARTRxBuffer[i+4];
                setPIDStr[3] = UARTRxBuffer[i+5];

                float newVal = atof(setPIDStr);

                switch (UARTRxBuffer[i+2]) {
                    // Kp
                    case 'p':
                        rtdb_set_PID_params(newVal, Ki, Kd);
                    // Ki
                    case 'i':
                        rtdb_set_PID_params(Kp, newVal, Kd);
                    // Kd
                    case 'd':
                        rtdb_set_PID_params(Kp, Ki, newVal);
                }


                //  Send good ACK
                send_ack(0);

                rxBufLen = 0;  // clean buffer
                return 0;

            //  Sets PID parameters as #Vyyy!
            case 'V':
                if(UARTRxBuffer[i+5] != EOF_SYM) {
                    //  Send bad framing ACK
                    send_ack(1);
                    return -4;
                }

                // Checksum de entrada: apenas sobre CMD ('M') + DATA ('px.xix.xxdx.xx')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    //  Send bad checksum ACK
                    send_ack(2);
                    return -3;
                }

                rtdb_set_verbose(!rtdb_get_verbose());

                //  Send good ACK
                send_ack(0);

                rxBufLen = 0;  // clean buffer
                return 0;

            default:
                //  Send bad command ACK
                send_ack(3);
                return -2;
        }
    }

    return -4;
}


/**
 * @brief Calculates 8-bit checksum for a given buffer.
 * 
 * @param buf Pointer to buffer.
 * @param nbytes Number of bytes to compute.
 * @return int Calculated checksum.
 */
int calcChecksum(unsigned char *buf, int nbytes) {
    unsigned int checksum = 0;

    // Soma dos valores dos bytes do buffer
    for (int i = 0; i < nbytes; i++) {
        checksum += buf[i];
    }

    // Garantir que o checksum seja um valor de 8 bits (0-255)
    checksum = checksum % 256;

    // Retorna o checksum calculado
    return checksum;
}


/**
 * @brief Appends a character to the receive buffer.
 * 
 * @param car Character to append.
 * @return int 0 if success, -1 if buffer full.
 */
int rxChar(unsigned char car)
{
    if (rxBufLen < UART_RX_SIZE) {
        UARTRxBuffer[rxBufLen] = car;
        rxBufLen += 1;
        return 0;
    }
    return -1;
}

/**
 * @brief Appends a character to the transmit buffer.
 * 
 * @param car Character to append.
 * @return int 0 if success, -1 if buffer full.
 */
int txChar(unsigned char car)
{
    if (txBufLen < UART_TX_SIZE) {
        UARTTxBuffer[txBufLen] = car;
        txBufLen += 1;
        return 0;
    }
    return -1;
}


/**
 * @brief Resets the UART receive buffer.
 */
void resetRxBuffer(void) {
    rxBufLen = 0;
    memset(UARTRxBuffer, 0, sizeof(UARTRxBuffer));
}


/**
 * @brief Resets the UART transmit buffer.
 */
void resetTxBuffer(void) {
    txBufLen = 0;
    memset(UARTTxBuffer, 0, sizeof(UARTTxBuffer));
}


/**
 * @brief Retrieves the contents of the transmit buffer.
 * 
 * @param buf Pointer to destination buffer.
 * @param len Pointer to variable storing transmit length.
 */
void getTxBuffer(unsigned char * buf, int * len)
{
    *len = txBufLen;
    if(txBufLen > 0) {
        memcpy(buf,UARTTxBuffer,*len);
    }        
    return;
}


/**
 * @brief Returns the size of the receive buffer.
 * 
 * @return int Receive buffer length.
 */
int getRxBufferSize(void){
    return rxBufLen;
}

/**
 * @brief Returns the size of the transmit buffer.
 * 
 * @return int Transmit buffer length.
 */
int getTxBufferSize(void){
    return txBufLen;
}


/**
 * @brief Sends an acknowledgment message with appropriate checksum.
 * 
 * @param type Acknowledgment type:
 *             - 0: OK
 *             - 1: Framing error
 *             - 2: Checksum error
 *             - 3: Invalid command
 */
void send_ack(int type) {
    unsigned char checksumBuffer[2];
    int checksum = 0;
    char checksumStr[5];

    checksumBuffer[0] = 'E';

    switch (type) {
        case 0:
            checksumBuffer[1] = 'o';
            break;
        case 1:
            checksumBuffer[1] = 'f';
            break;
        case 2:
            checksumBuffer[1] = 's';
            break;
        case 3:
        default:
            checksumBuffer[1] = 'i';
            break;
    }

    for (int k = 0; k < 2; k++) {
        checksum += checksumBuffer[k];
    }
    checksum = checksum % 256;
    snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);

    txChar('#');
    for (int k = 0; k < 2; k++) {
        txChar(checksumBuffer[k]);
    }
    txChar(checksumStr[0]);
    txChar(checksumStr[1]);
    txChar(checksumStr[2]);
    txChar('!');

    rxBufLen = 0;
}