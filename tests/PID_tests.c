//gcc tests.c modules/cmdproc.c Unity/src/unity.c -o test
#include "Unity/src/unity.h"
#include "modules/cmdproc.h"
#include "modules/rtdb.h"
#include "modules/PID.h"


/** \file tests.c
*   \brief Unit tests for Assignment 3
**
*        This file tests all the funtions on the 
*       PID function developed for this assignment
**
* \author Pedro Ramos, n.º 107348
* \author Rafael Morgado, n.º 104277
* \date 01/06/2025
*/

/**
 * @brief Set up function executed before each test.
 */
void setUp(void) {}

/**
 * @brief Tear down function executed after each test.
 */
void tearDown(void) {
}  

/**
 * @brief Test basic PID calculation with all terms
 */
void test_PID_BasicCalculation(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===  Test PID Calculation   === == -  │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    // Setup test conditions
    float setpoint = 100.0f;
    float measured = 90.0f;
    float dt = 0.1f;
    float last_error = 0.0f;
    float integral = 0.0f;

    // Set PID parameters in RTDB

    float Kp = 1.0f;
    float Ki = 0.1f;
    float Kd = 0.01f;
    rtdb_set_PID_params(Kp, Ki, Kd);  // Kp=1, Ki=0.1, Kd=0.01

    float output = pid_calculate(setpoint, measured, dt, &last_error, &integral);

    printf("   ─> Output: %.2f, Last Error: %.2f, Integral: %.2f\n", output, last_error, integral);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 11.1f, output);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, last_error);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, integral);  // integral sum is error*dt = 1.0
    printf("   ─> Test passed: Basic PID calculation correct\n\n");
}

/**
 * @brief Test integral windup protection
 */
void test_PID_IntegralWindup(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test Integral Windup   === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    float setpoint = 100.0f;
    float measured = 50.0f;
    float dt = 1.0f;
    float last_error = 0.0f;
    float integral = 15.0f;  // Start near limit

    rtdb_set_PID_params(1.0f, 0.5f, 0.0f);  // Kp=1, Ki=0.5, Kd=0

    // First call - should push integral near limit (15 + 50*1 = 65 but capped at 20)
    float output = pid_calculate(setpoint, measured, dt, &last_error, &integral);

    printf("   ─> Output: %.2f, Integral: %.2f\n", output, integral);
    
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, integral);  // Should be capped at 20
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 60.0f, output);    // P=50, I=20 (10*0.5)
    printf("   ─> Test passed: Integral term properly limited\n\n");
}

/**
 * @brief Test derivative term calculation
 */
void test_PID_DerivativeTerm(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test Derivative Term   === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    float setpoint = 100.0f;
    float measured1 = 90.0f;
    float measured2 = 95.0f;  // Temperature rising
    float dt = 0.1f;
    float last_error = 0.0f;
    float integral = 0.0f;

    rtdb_set_PID_params(1.0f, 0.0f, 0.1f);  // Kp=1, Ki=0, Kd=0.1

    // First call - establish baseline
    pid_calculate(setpoint, measured1, dt, &last_error, &integral);
    // Second call - check derivative
    float output = pid_calculate(setpoint, measured2, dt, &last_error, &integral);

    printf("   ─> Output: %.2f, Last Error: %.2f\n", output, last_error);
    
    // P term: 5.0 (100-95)
    // D term: 0.1 * ((5-10)/0.1) = -5.0
    // Expected: 5.0 + (-5.0) = 0.0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, output);
    printf("   ─> Test passed: Derivative term calculated correctly\n\n");
}

/**
 * @brief Test zero delta time handling
 */
void test_PID_ZeroDeltaTime(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test Zero Delta Time   === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    float setpoint = 100.0f;
    float measured = 80.0f;
    float dt = 0.0f;  // Zero delta time
    float last_error = 0.0f;
    float integral = 0.0f;

    rtdb_set_PID_params(1.0f, 0.1f, 0.01f);

    float output = pid_calculate(setpoint, measured, dt, &last_error, &integral);

    printf("   ─> Output: %.2f\n", output);
    
    // Should handle division by zero gracefully (skip derivative term)
    // P term: 20.0, I term: 0.0 (dt=0), D term: skipped
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, output);
    printf("   ─> Test passed: Handled zero delta time gracefully\n\n");
}

/**
 * @brief Test negative error values
 */
void test_PID_NegativeError(void) {
    printf("\n");
    printf(" ╭─────────────────────────────────────────────╮\n");
    printf(" │  - == ===   Test Negative Error    === == - │\n");
    printf(" ╰─────────────────────────────────────────────╯\n");

    float setpoint = 100.0f;
    float measured = 110.0f;  // Negative error
    float dt = 0.1f;
    float last_error = 0.0f;
    float integral = 0.0f;

    rtdb_set_PID_params(1.0f, 0.1f, 0.01f);

    float output = pid_calculate(setpoint, measured, dt, &last_error, &integral);

    printf("   ─> Output: %.2f, Integral: %.2f\n", output, integral);
    
    // P term: -10.0
    // I term: -1.0 (-10*0.1*0.1)
    // D term: -1.0 (0.01 * (-10-0)/0.1)
    // Expected: -10 + (-1) + (-1) = -12.0
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -12.0f, output);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -10.0f, last_error);
    printf("   ─> Test passed: Handled negative error correctly\n\n");
}

int main(void) {
    // Initialize Unity test framework
    UNITY_BEGIN();

    // Run PID calculation tests
    RUN_TEST(test_PID_BasicCalculation);
    RUN_TEST(test_PID_IntegralWindup);
    RUN_TEST(test_PID_DerivativeTerm);
    RUN_TEST(test_PID_ZeroDeltaTime);
    RUN_TEST(test_PID_NegativeError);

    // Finalize and return test results
    return UNITY_END();
}