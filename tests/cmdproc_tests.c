//gcc tests.c modules/cmdproc.c Unity/src/unity.c -o test
#include "Unity/src/unity.h"
#include "modules/cmdproc.h"


/** \file tests.c
*   \brief Unit tests for Assignment 3
**
*        This file tests all the funtions on the 
*       CMD_Proc module developped for this assignment
**
* \author Pedro Ramos, n.º 107348
* \author Rafael Morgado, n.º 104277
* \date 01/06/2025
*/

/**
 * @brief Set up function executed before each test.
 */
void setUp(void) {
    resetTxBuffer();
    resetRxBuffer();
}

/**
 * @brief Tear down function executed after each test.
 */
void tearDown(void) {
}  

/**
 * @brief Test the temperature command processor.
 */
void test_ReadCurrentTemp(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - == === Read Current Temperature  === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    resetTxBuffer();
    resetRxBuffer();

    rxChar('#');
    rxChar('C');
    rxChar('0');
    rxChar('6');
    rxChar('7');
    rxChar('!');

    // chama cmdProcessor()
    int result = cmdProcessor();
    
    // verifica que a função cmdProcessor() retorna 0 (sucesso)
    TEST_ASSERT_EQUAL(0, result);

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#cXXXYYY!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #cXXXYYY!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#c", ans, 2);
}

/**
 * @brief Test the humidity command processor.
 */
void test_ReadDesiredTemp(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - == === Read Desired Temperature  === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    resetTxBuffer();
    resetRxBuffer();

    rxChar('#');
    rxChar('D');
    rxChar('0');
    rxChar('6');
    rxChar('8');
    rxChar('!');

    // chama cmdProcessor()
    int result = cmdProcessor();
    
    // verifica que a função cmdProcessor() retorna 0 (sucesso)
    TEST_ASSERT_EQUAL(0, result);

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#dXXXYYY!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #dXXXYYY!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#d", ans, 2);
}

/**
 * @brief Test the "all" command processor.
 */
void test_SetDesiredTemp(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - == === Write Desired Temperature === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('M');
    rxChar('+');
    rxChar('3');
    rxChar('0');
    rxChar('2');
    rxChar('1');
    rxChar('9');
    rxChar('!');

    // chama cmdProcessor()
    int result = cmdProcessor();
    
    // verifica que a função cmdProcessor() retorna 0 (sucesso)
    TEST_ASSERT_EQUAL(0, result);

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Eo180!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Eo180!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Eo180!", ans, len);
}

/**
 * @brief Test the history command processor.
 */
void test_SetPIDparams(void) {

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - == ===   Write PID Parameters    === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    int i = 0;

    uint8_t values[] = {'p', 'i', 'd'};

    for (i = 0; i < 3; i++) {
        resetTxBuffer();
        resetRxBuffer();
        rxChar('#');
        rxChar('S');
        rxChar(values[i]);
        rxChar('1');
        rxChar('.');
        rxChar('2');
        rxChar('3');
        switch (values[i]) {
            case 'p':
                rxChar('1');
                rxChar('3');
                rxChar('5');
                break;
            case 'i':
                rxChar('1');
                rxChar('2');
                rxChar('8');
                break;
            case 'd':
                rxChar('1');
                rxChar('2');
                rxChar('3');
                break;
        }
        rxChar('!');

        // chama cmdProcessor()
        int result = cmdProcessor();
        
        // verifica que a função cmdProcessor() retorna 0 (sucesso)
        TEST_ASSERT_EQUAL(0, result);

        // obter a resposta gerada
        unsigned char ans[32];
        int len;
        getTxBuffer(ans, &len);

        // resposta esperada: '#Eo180!'
        // Imprimir a resposta esperada e gerada
        printf("   Change K%c\n", values[i]);
        printf("   ─> Expected response:  #Eo180!");
        printf("\n   ─> Generated response: ");
        for (int i = 0; i < len; i++) {
            printf("%c", ans[i]); 
        }
        printf("\n");

        // verifica se a resposta gerada é igual à resposta esperada
        TEST_ASSERT_EQUAL_MEMORY("#Eo180!", ans, len);
    }
    printf("\n");
}

/**
 * @brief Test the reset command processor.
 */
void test_ToggleVerbose(void) {

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - == ===      Toggle Verbose       === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('V');
    rxChar('0');
    rxChar('8');
    rxChar('6');
    rxChar('!');

    // chama cmdProcessor()
    int result = cmdProcessor();
    
    // verifica que a função cmdProcessor() retorna 0 (sucesso)
    TEST_ASSERT_EQUAL(0, result);

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Eo180!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Eo180!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Eo180!", ans, len);
}

/**
 * @brief Test an invalid command.
 */
void test_invalidcommand(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===  Test invalid command  === == -   │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('X'); // invalid command
    rxChar('t');
    rxChar('1');
    rxChar('9');
    rxChar('6');
    rxChar('!');

    int result = cmdProcessor();
    printf("cmdProcessor returned -> %d\n\n", result);
    TEST_ASSERT_EQUAL(-2, result); // checks if returns -2 (invalid command)

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Ei174!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Ei174!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Ei174!", ans, len);
}


void test_invalidchecksum(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===  Test invalid checksum  === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('V');
    rxChar('0');
    rxChar('8'); // invalid checksum
    rxChar('5');
    rxChar('!');

    int result = cmdProcessor();
    printf("cmdProcessor returned -> %d\n\n", result);
    TEST_ASSERT_EQUAL(-3, result); // checks if returns -2 (invalid checksum)

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Es184!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Es184!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Es184!", ans, len);
}


void test_invalidframe(void){

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test invalid frame   === == -   │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('V');
    rxChar('8'); // invalid frame
    rxChar('!');

    int result = cmdProcessor();
    printf("cmdProcessor returned -> %d\n\n", result);
    TEST_ASSERT_EQUAL(-2, result); // checks if returns -2 (invalid command)

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Ef171!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Ef171!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Ef171!", ans, len);
}

/**
 * @brief Test checksum calculation with valid data.
 */
void test_calcChecksum_valid(void) {

    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == === Test calculate checksum === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    unsigned char buf[] = {'P', 't'};
    
    int nbytes = sizeof(buf);

    int result = calcChecksum(buf, nbytes);

    printf("Result of calcChecksum: %d\n", result);  
    
    TEST_ASSERT_EQUAL(196, result);  
}


/**
 * @brief Test reset of buffers.
 */
void test_reset_buffers(void){
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===  Test  reset buffers   === == -   │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    resetTxBuffer();
    resetRxBuffer();
    rxChar('#');
    rxChar('V');
    rxChar('0');
    rxChar('8');
    rxChar('6');
    rxChar('!');

    cmdProcessor(); 

    resetRxBuffer();
    resetTxBuffer();

    if (getRxBufferSize() == 0 && getTxBufferSize() == 0) {
        printf("Test succeeded, buffers reseted successfuly\n");
    }else {
        printf("Test failed, buffers did not reseted successfuly\n");
    }
    
    TEST_ASSERT_EQUAL(0, getRxBufferSize());
    TEST_ASSERT_EQUAL(0, getTxBufferSize());
}

/**
 * @brief Test for missing characters in the command.
 */
void test_incomplete_command(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │ - = =Test missing character in command= = - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    resetTxBuffer();
    resetRxBuffer();
    // 1 - Envia o comando
    rxChar('#');
    rxChar('V');
    //rxChar('0');
    rxChar('8');
    rxChar('6');
    rxChar('!');

    int err = cmdProcessor();
        
    if (err == -4) {
        printf("Test succeeded, an omission was detected\n");
    }else {
        printf("Test failed, as omission was not detected\n");
    }

    TEST_ASSERT_EQUAL(-4, err);

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Ef171!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Ef171!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Ef171!", ans, len);
}


/**
 * @brief Test buffer overflow in RX buffer.
 */
 void test_RxBufferOverflow(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test RX Buffer Overflow === == -│\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    // Fill buffer to capacity (should succeed)
    for (int i = 0; i < UART_RX_SIZE; i++) {
        TEST_ASSERT_EQUAL(0, rxChar('A'));
    }
    
    // Next character should fail (buffer full)
    TEST_ASSERT_EQUAL(-1, rxChar('B'));
    
    // Verify buffer size
    TEST_ASSERT_EQUAL(UART_RX_SIZE, getRxBufferSize());
    
    resetRxBuffer();
    printf("   ─> Test passed: RX buffer overflow handled correctly\n\n");
}

/**
 * @brief Test buffer overflow in TX buffer.
 */
void test_TxBufferOverflow(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test TX Buffer Overflow === == -│\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    // Fill buffer to capacity (should succeed)
    for (int i = 0; i < UART_TX_SIZE; i++) {
        TEST_ASSERT_EQUAL(0, txChar('A'));
    }
    
    // Next character should fail (buffer full)
    TEST_ASSERT_EQUAL(-1, txChar('B'));
    
    // Verify buffer size
    TEST_ASSERT_EQUAL(UART_TX_SIZE, getTxBufferSize());
    
    resetTxBuffer();
    printf("   ─> Test passed: TX buffer overflow handled correctly\n\n");
}


/**
 * @brief Test command missing EOF symbol.
 */
void test_MissingEOF(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===    Test Missing EOF     === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");
    
    resetTxBuffer();
    resetRxBuffer();
    // Send command without '!'
    rxChar('#');
    rxChar('V');
    rxChar('0');
    rxChar('8');
    rxChar('6'); // Missing '!'

    int result = cmdProcessor();
    printf("   ─> cmdProcessor returned -> %d\n\n", result);
    
    // Should return -4 (format error)
    TEST_ASSERT_EQUAL(-4, result);
    
    printf("   ─> Test passed: Missing EOF detected\n\n");
}

/**
 * @brief Test lowercase command handling
 */
void test_LowercaseCommands(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == === Test Lowercase Commands  === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    // Try lowercase 'p' instead of 'P'
    rxChar('#');
    rxChar('v'); // Lowercase
    rxChar('0');
    rxChar('8');
    rxChar('6');
    rxChar('!');

    int result = cmdProcessor();
    printf("   ─> cmdProcessor returned: %d\n", result);
    
    // Should reject lowercase commands
    TEST_ASSERT_EQUAL(-2, result); 
    printf("   ─> Test passed: Lowercase commands rejected\n\n");

    // obter a resposta gerada
    unsigned char ans[32];
    int len;
    getTxBuffer(ans, &len);

    // resposta esperada: '#Ei174!'
    // Imprimir a resposta esperada e gerada
    printf("   ─> Expected response:  #Ei174!");
    printf("\n   ─> Generated response: ");
    for (int i = 0; i < len; i++) {
        printf("%c", ans[i]); 
    }
    printf("\n\n");

    // verifica se a resposta gerada é igual à resposta esperada
    TEST_ASSERT_EQUAL_MEMORY("#Ei174!", ans, len);
}

int main(void){

    // inicia a Unity
    UNITY_BEGIN();

    
    RUN_TEST(test_ReadCurrentTemp);
    RUN_TEST(test_ReadDesiredTemp);
    RUN_TEST(test_SetDesiredTemp);
    RUN_TEST(test_SetPIDparams);
    RUN_TEST(test_ToggleVerbose);
    RUN_TEST(test_invalidcommand);
    RUN_TEST(test_invalidchecksum);
    RUN_TEST(test_calcChecksum_valid);
    RUN_TEST(test_reset_buffers);
    RUN_TEST(test_incomplete_command);
    
    // New tests
    RUN_TEST(test_RxBufferOverflow);
    RUN_TEST(test_TxBufferOverflow);
    RUN_TEST(test_MissingEOF);
    RUN_TEST(test_LowercaseCommands);

    // finaliza e retorna os resultados
    return UNITY_END();

}