#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// Hardware Pin Definitions
#define GPIO_PWM_ENA    GPIO_NUM_19
#define GPIO_DIR_IN1    GPIO_NUM_18
#define GPIO_DIR_IN2    GPIO_NUM_5

// LEDC PWM Configuration
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT // Duty cycle range: 0 - 1023
#define LEDC_FREQUENCY  (10000)             // max 10 kHz PWM frequency for motor driver board

void init_motor_hardware(void)
{
    // 1. Configure Direction Pins (GPIO output)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_DIR_IN1) | (1ULL << GPIO_DIR_IN2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // 2. Configure LEDC Timer for PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 3. Configure LEDC Channel for ENA PWM Output
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GPIO_PWM_ENA,
        .duty           = 0, // Initial duty 0 (off)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

// Set motor speed (0 to 1023)
void set_motor_speed(uint32_t duty)
{
    if (duty > 1023) duty = 1023; // Clamp max duty
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

// Direction Control Functions
void motor_forward(uint32_t speed_duty)
{
    gpio_set_level(GPIO_DIR_IN1, 1);
    gpio_set_level(GPIO_DIR_IN2, 0);
    set_motor_speed(speed_duty);
}

void motor_backward(uint32_t speed_duty)
{
    gpio_set_level(GPIO_DIR_IN1, 0);
    gpio_set_level(GPIO_DIR_IN2, 1);
    set_motor_speed(speed_duty);
}

void motor_stop(void)
{
    gpio_set_level(GPIO_DIR_IN1, 0);
    gpio_set_level(GPIO_DIR_IN2, 0);
    set_motor_speed(0);
}

void app_main(void)
{
    printf("Initializing Motor Driver Test Routine...\n");
    init_motor_hardware();
    
    // 1. Move Forward at ~50% Speed (Duty 512 / 1023)
    printf("Motor: FORWARD (20%% Speed)\n");
    motor_forward(512);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 2. Stop
    printf("Motor: STOP\n");
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 3. Move Backward at ~75% Speed (Duty 768 / 1023)
    printf("Motor: BACKWARD (50%% Speed)\n");
    motor_backward(768);
    vTaskDelay(pdMS_TO_TICKS(30000));

    // 4. Stop
    printf("Motor: STOP\n");
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
}