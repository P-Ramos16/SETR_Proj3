#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>  
#include <time.h>    
#include "cmdproc.h"

/* Internal variables */
/* Used as part of the UART emulation */
static unsigned char UARTRxBuffer[UART_RX_SIZE];
static unsigned char rxBufLen = 0; 

static unsigned char UARTTxBuffer[UART_TX_SIZE];
static unsigned char txBufLen = 0; 

static int possTempArray[] = {0, 10, -10, 30, -25, -50, 60};
static unsigned int currentTempIndex = 0;
static int lastTempArray[20];
static unsigned int sizeTempArray = 0;

static unsigned int possHumidArray[] = {0, 10, 20, 40, 60, 80, 100};
static unsigned int currentHumidIndex = 0;
static unsigned int lastHumidArray[20];
static unsigned int sizeHumidArray = 0;

static unsigned int possCO2Array[] = {0, 400, 1000, 2500, 5000, 10000, 20000};
static unsigned int currentCO2Index = 0;
static unsigned int lastCO2Array[20];
static unsigned int sizeCO2Array = 0;

static unsigned char historyLBuffer[256];
static unsigned char historyLIdx = 0;

/* Function implementation */

int cmdProcessor(void) {
    int i;
    unsigned char sid;


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
            
            //  Responds as #at+22h020c01000CKS!
            case 'A':

                // Checksum de entrada: apenas sobre CMD ('L')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    return -3;
                }


                if(UARTRxBuffer[i+5] != EOF_SYM) {
                    return -4;
                }

                checksumBuffer[chksumIdx++] = 'a';


                int sensorValueT = readTempSensor();
                int sensorValueH = readHumidSensor();
                int sensorValueC = readCO2Sensor();

                sprintf(sensorStr, "%02d", abs(sensorValueT));

                checksumBuffer[chksumIdx++] = 't';
                checksumBuffer[chksumIdx++] = (sensorValueT >= 0) ? '+' : '-';

                for (int k = 0; sensorStr[k] != '\0'; k++) {
                    checksumBuffer[chksumIdx++] = sensorStr[k];
                }

                sprintf(sensorStr, "%03d", abs(sensorValueH));

                checksumBuffer[chksumIdx++] = 'h';

                for (int k = 0; sensorStr[k] != '\0'; k++) {
                    checksumBuffer[chksumIdx++] = sensorStr[k];
                }

                sprintf(sensorStr, "%05d", abs(sensorValueC));

                checksumBuffer[chksumIdx++] = 'c';

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


            case 'L':

                // Checksum de entrada: apenas sobre CMD ('L')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    return -3;
                }

                //  Smaller message that asks how many messages are needed to transmit the data
                if(UARTRxBuffer[i+5] == EOF_SYM) {
                    memset(historyLBuffer, 0, sizeof(historyLBuffer));
                    historyLIdx = 0;

                    //  Iterate over lastTempArray and print values
                    historyLBuffer[historyLIdx++] = 't';
                    for (unsigned int i = 0; i < sizeTempArray; i++) {
                        sprintf(sensorStr, "%02d", abs(lastTempArray[i]));
                        historyLBuffer[historyLIdx++] = (lastTempArray[i] >= 0) ? '+' : '-';

                        for (int k = 0; sensorStr[k] != '\0'; k++) {
                            historyLBuffer[historyLIdx++] = sensorStr[k];
                        }
                    }

                    //  Iterate over lastHumidArray and print values
                    historyLBuffer[historyLIdx++] = 'h';
                    for (unsigned int i = 0; i < sizeHumidArray; i++) {
                        sprintf(sensorStr, "%03d", abs(lastHumidArray[i]));

                        for (int k = 0; sensorStr[k] != '\0'; k++) {
                            historyLBuffer[historyLIdx++] = sensorStr[k];
                        }
                    }

                    //  Iterate over lastCO2Array and print values
                    historyLBuffer[historyLIdx++] = 'c';
                    for (unsigned int i = 0; i < sizeCO2Array; i++) {
                        sprintf(sensorStr, "%05d", abs(lastCO2Array[i]));

                        for (int k = 0; sensorStr[k] != '\0'; k++) {
                            historyLBuffer[historyLIdx++] = sensorStr[k];
                        }
                    }

                    checksumBuffer[chksumIdx++] = 'l';

                    //  Send num of messages needed to get all the data
                    char sizeStr[5];

                    int messageNumber;

                    if (historyLIdx <= 3) {
                        messageNumber = 0;
                    }
                    else {
                        messageNumber = (historyLIdx/15)+1;
                    }

                    snprintf(sizeStr, sizeof(sizeStr), "%02d", messageNumber);
                    for (int k = 0; sizeStr[k] != '\0'; k++) {
                        checksumBuffer[chksumIdx++] = sizeStr[k];
                    }
                }


                //  Larger message that asks for extra message number X
                else if (UARTRxBuffer[i+7] == EOF_SYM) {
                    int messageNumber = (UARTRxBuffer[i+2] - '0') * 10 + (UARTRxBuffer[i+3] - '0');

                    checksumBuffer[chksumIdx++] = 'l';

                    for (int k = messageNumber*14; k < historyLIdx && k < (messageNumber + 1)*14 ; k++) {
                        checksumBuffer[chksumIdx++] = historyLBuffer[k];
                    }
                }


                else {
                    return -4;
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



            case 'P':  
                sid = UARTRxBuffer[i+2];

                if(sid != 't' && sid != 'h' && sid != 'c') {
                    return -2;
                }


                // Checksum de entrada: apenas sobre CMD ('P') + DATA ('t')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    return -3;
                }

                if(UARTRxBuffer[i+6] != EOF_SYM) {
                    return -4;
                }

                // Adicionar o valor do sensor (formatado como string)
                char sensorStr[12];

                checksumBuffer[chksumIdx++] = 'p';
                checksumBuffer[chksumIdx++] = sid;

                // Simular leitura de sensor
                int sensorValue = 0;
                if (sid == 't') {
                    sensorValue = readTempSensor();
                    checksumBuffer[chksumIdx++] = (sensorValue >= 0) ? '+' : '-';
                    sprintf(sensorStr, "%02d", abs(sensorValue));
                }
                else if (sid == 'h') {
                    sensorValue = readHumidSensor();
                    sprintf(sensorStr, "%03d", abs(sensorValue));
                }
                else if (sid == 'c') {
                    sensorValue = readCO2Sensor();
                    sprintf(sensorStr, "%05d", abs(sensorValue));
                }

                // Adicionar o valor do sensor (formatado como string)
                for (int k = 0; sensorStr[k] != '\0'; k++) {
                    checksumBuffer[chksumIdx++] = sensorStr[k];
                }

                // Calcular o cheksum com base no buffer completo
                checksum = calcChecksum(checksumBuffer, chksumIdx);

                // Formatar o checksum como uma string de 3 caracteres
                snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);

                // Enviar a  resposta
                txChar('#');
                for (int k = 0; k < chksumIdx; k++) {
                    txChar(checksumBuffer[k]);
                }
                txChar(checksumStr[0]);
                txChar(checksumStr[1]);
                txChar(checksumStr[2]);
                txChar('!');

                rxBufLen = 0;  // clean buffer
                return 0;



            case 'R':
                // Checksum de entrada: apenas sobre CMD ('R')
                if(calcChecksum(&(UARTRxBuffer[i+1]), rxBufLen - 5) != msgCheckSum) {
                    return -3;
                }

                if(UARTRxBuffer[i+5] != EOF_SYM) {
                    return -4;
                }

                checksumBuffer[chksumIdx++] = 'r';

                //  Clear the Temp array
                memset(lastTempArray, 0, sizeof(lastTempArray));
                sizeTempArray = 0;
                //  Clear the Humid array
                memset(lastHumidArray, 0, sizeof(lastHumidArray));
                sizeHumidArray = 0;
                //  Clear the CO2 array
                memset(lastCO2Array, 0, sizeof(lastCO2Array));
                sizeCO2Array = 0;

                txChar('#');
                txChar('r');
                txChar('1');
                txChar('1');
                txChar('4');
                txChar('!');

                rxBufLen = 0;
                return 0;
            default:
                return -2;
        }
    }

    return -4;
}

int readTempSensor() {
    if (currentTempIndex >= sizeof(possTempArray) / sizeof(int)) {
        currentTempIndex = 0;
    }
    int currentTemp = possTempArray[currentTempIndex++];
    
    //  If there is space in the history array
    if (sizeTempArray < 20) {
        lastTempArray[sizeTempArray++] = currentTemp;
    } 
    //  Shift values in lastTempArray to make space for the new value
    else {
        for (int i = 1; i < 20; i++) {
            lastTempArray[i - 1] = lastTempArray[i];
        }
        lastTempArray[19] = currentTemp;
    }

    return currentTemp;
}

int readHumidSensor() {
    if (currentHumidIndex >= sizeof(possHumidArray) / sizeof(int)) {
        currentHumidIndex = 0;
    }
    int currentHumid = possHumidArray[currentHumidIndex++];
    
    //  If there is space in the history array
    if (sizeHumidArray < 20) {
        lastHumidArray[sizeHumidArray++] = currentHumid;
    } 
    //  Shift values in lastHumidArray to make space for the new value
    else {
        for (int i = 1; i < 20; i++) {
            lastHumidArray[i - 1] = lastHumidArray[i];
        }
        lastHumidArray[19] = currentHumid;
    }

    return currentHumid;
}

int readCO2Sensor() {
    if (currentCO2Index >= sizeof(possCO2Array) / sizeof(int)) {
        currentCO2Index = 0;
    }
    int currentCO2 = possCO2Array[currentCO2Index++];
    
    //  If there is space in the history array
    if (sizeCO2Array < 20) {
        lastCO2Array[sizeCO2Array++] = currentCO2;
    } 
    //  Shift values in lastCO2Array to make space for the new value
    else {
        for (int i = 1; i < 20; i++) {
            lastCO2Array[i - 1] = lastCO2Array[i];
        }
        lastCO2Array[19] = currentCO2;
    }

    return currentCO2;
}

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

int rxChar(unsigned char car)
{
    if (rxBufLen < UART_RX_SIZE) {
        UARTRxBuffer[rxBufLen] = car;
        rxBufLen += 1;
        return 0;
    }
    return -1;
}

int txChar(unsigned char car)
{
    if (txBufLen < UART_TX_SIZE) {
        UARTTxBuffer[txBufLen] = car;
        txBufLen += 1;
        return 0;
    }
    return -1;
}

void resetRxBuffer(void) {
    rxBufLen = 0;
    memset(UARTRxBuffer, 0, sizeof(UARTRxBuffer));
}

void resetTxBuffer(void) {
    txBufLen = 0;
    memset(UARTTxBuffer, 0, sizeof(UARTTxBuffer));
}

void getTxBuffer(unsigned char * buf, int * len)
{
    *len = txBufLen;
    if(txBufLen > 0) {
        memcpy(buf,UARTTxBuffer,*len);
    }        
    return;
}

int getRxBufferSize(void){
    return rxBufLen;
}

int getTxBufferSize(void){
    return txBufLen;
}