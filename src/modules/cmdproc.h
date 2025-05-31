/* ******************************************************/
/* SETR 23/24, Paulo Pedreiras                          */
/*	Base code for Unit Testing                          */
/*  	Simple example of command processor             */
/*    	for smart sensor node with 3 sensors			*/
/*     													*/
/*	Code is just for illustrative effects. E.g. error 	*/ 
/*		codes are "magic numbers" in the middle of the	*/
/*    	code instead of being (defined) text literals, 	*/
/* 		sensor data is not properly emulated, missing 	*/
/* 		commands, Checksum not implemented, ...			*/
/*														*/
/* ******************************************************/


/** \file cmdproc.h
*   \brief Base code for Unit Testing
*
*        Simple example of command processor 
*       for a smart sensor node with 3 sensors.
*
* \author Pedro Ramos, n.º 107348
* \author Rafael Morgado, n.º 104277
* \date 06/04/2025
*/

#ifndef CMD_PROC_H_
#define CMD_PROC_H_

/* Some defines */
/* Other defines should be return codes of the functions */
/* E.g. #define CMD_EMPTY_STRING -1                      */
#define UART_RX_SIZE 20 	/* Maximum size of the RX buffer */ 
#define UART_TX_SIZE 20 	/* Maximum size of the TX buffer */ 
#define SOF_SYM '#'	        /* Start of Frame Symbol */
#define EOF_SYM '!'          /* End of Frame Symbol */

/* Function prototypes */

/* ************************************************************ */
/* Processes the chars in the RX buffer looking for commands 	*/
/* Returns:                                                     */
/*  	 0: if a valid command was found and executed           */
/* 		-1: if empty string or incomplete command found         */
/* 		-2: if an invalid command was found                     */
/* 		-3: if a CS error is detected (command not executed)    */
/* 		-4: if string format is wrong                           */
/* ************************************************************ */
/**
 * @brief Processes received commands and generates responses.
 * @return Status code (0 on success, negative on failure)
 */
int cmdProcessor(void);

/* ******************************** */
/* Adds a char to the RX buffer 	*/
/* I.e., the reception of commands 	*/
/* Returns: 				        */
/*  	 0: if success 		        */
/* 		-1: if buffer full	 	    */
/* ******************************** */
/**
 * @brief Receives a character and stores it in the receive buffer.
 * @param car Character to receive
 * @return 0 on success, -1 if buffer is full
 */
int rxChar(unsigned char car);

/* ************************************ */
/* Adds a char to the TX buffer 		*/
/* I.e., the tranmsisison of answers 	*/
/* Returns: 				        	*/
/*  	 0: if success 		        	*/
/* 		-1: if buffer full	 	    	*/
/* ************************************ */
/**
 * @brief Transmits a character and stores it in the transmit buffer.
 * @param car Character to transmit
 * @return 0 on success, -1 if buffer is full
 */
int txChar(unsigned char car);

/* ************************* */
/* Resets the RX buffer		 */  
/* ************************* */
/**
 * @brief Resets the UART receive buffer.
 */
void resetRxBuffer(void);

/* ************************* */
/* Resets the TX buffer		 */  
/* ************************* */
/**
 * @brief Resets the UART transmit buffer.
 */
void resetTxBuffer(void);

/* ************************************************ */
/* Return the data that would be sent by the sensor */  
/* ************************************************ */
/**
 * @brief Gets the current size of the UART transmit buffer.
 * @return Number of bytes in the transmit buffer
 */
void getTxBuffer(unsigned char * buf, int * len);

/* ************************************************ */
/* Computes the checksum of a given number of chars */
/* ************************************************ */ 
/**
 * @brief Calculates a checksum for a given data buffer.
 * @param buf Pointer to data buffer
 * @param nbytes Number of bytes to process
 * @return Computed checksum value
 */
int calcChecksum(unsigned char * buf, int nbytes);

/* ************************************************ */
/* Simula a leitura de temperatura do sensor        */
/* ************************************************ */
/**
 * @brief Reads a simulated temperature sensor value.
 * @return Current temperature reading
 */
int readTempSensor(void);

/* ************************************************ */
/* Simula a leitura da humidade do sensor           */
/* ************************************************ */
/**
 * @brief Reads a simulated humidity sensor value.
 * @return Current humidity reading
 */
int readHumidSensor(void);

/* ************************************************ */
/* Simula a leitura de CO2 do sensor                */
/* ************************************************ */
/**
 * @brief Reads a simulated CO2 sensor value.
 * @return Current CO2 reading
 */
int readCO2Sensor(void);

/* ************************************************ */
/* Devolve size do buffer rx                        */
/* ************************************************ */
/**
 * @brief Gets the current size of the UART receive buffer.
 * @return Number of bytes in the receive buffer
 */
int getRxBufferSize(void);

/* ************************************************ */
/* Devolve size do buffer tx                        */
/* ************************************************ */
/**
 * @brief Gets the current size of the UART transmit buffer.
 * @return Number of bytes in the transmit buffer
 */
int getTxBufferSize(void);
#endif
