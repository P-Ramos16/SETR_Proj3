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
#define UART_RX_SIZE 20 	/**< Maximum size of the RX buffer */ 
#define UART_TX_SIZE 20 	/**< Maximum size of the TX buffer */ 
#define SOF_SYM '#'	        /**< Start of Frame Symbol */
#define EOF_SYM '!'         /**< End of Frame Symbol */

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
int cmdProcessor(void);

/**
 * @brief Appends a character to the receive buffer.
 * 
 * @param car Character to append.
 * @return int 0 if success, -1 if buffer full.
 */
int rxChar(unsigned char car);

/**
 * @brief Appends a character to the transmit buffer.
 * 
 * @param car Character to append.
 * @return int 0 if success, -1 if buffer full.
 */
int txChar(unsigned char car);

/**
 * @brief Resets the UART receive buffer.
 */
void resetRxBuffer(void);

/**
 * @brief Resets the UART transmit buffer.
 */
void resetTxBuffer(void);

/**
 * @brief Retrieves the contents of the transmit buffer.
 * 
 * @param buf Pointer to destination buffer.
 * @param len Pointer to variable storing transmit length.
 */
void getTxBuffer(unsigned char * buf, int * len);

/**
 * @brief Calculates 8-bit checksum for a given buffer.
 * 
 * @param buf Pointer to buffer.
 * @param nbytes Number of bytes to compute.
 * @return int Calculated checksum.
 */
int calcChecksum(unsigned char * buf, int nbytes);

/**
 * @brief Returns the size of the receive buffer.
 * 
 * @return int Receive buffer length.
 */
int getRxBufferSize(void);

/**
 * @brief Returns the size of the transmit buffer.
 * 
 * @return int Transmit buffer length.
 */
int getTxBufferSize(void);

/**
 * @brief Sends an acknowledgment message with appropriate checksum.
 * 
 * @param type Acknowledgment type:
 *             - 0: OK
 *             - 1: Framing error
 *             - 2: Checksum error
 *             - 3: Invalid command
 */
void send_ack(int type);


#endif
