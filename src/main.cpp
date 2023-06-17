#include <Arduino.h>
#include "esp_attr.h"
#include "rtc.h"
#include "rtc_store.h"

// GPIO NUMBER IS NOT BOARD OUTPUT NUMBER! See pin_map-2.png
const int MOTOR_1 =                     gpio_num_t::GPIO_NUM_21; // GPIO 21 = PIN D6
const int MOTOR_2 =                     gpio_num_t::GPIO_NUM_7;  // GPIO 7 = PIN D5 as written on board. See pin_map-2.png for more info
const int MOTOR_3 =                     gpio_num_t::GPIO_NUM_4;  // D2
const int MOTOR_4 =                     gpio_num_t::GPIO_NUM_3;  // D1
const int MOTOR_CONTROL_ENABLE_PIN =    gpio_num_t::GPIO_NUM_20; // D7
const int FAULT_PIN =                   gpio_num_t::GPIO_NUM_10; // D10. Is high when board is overheating or over current.
const int POWER_5V_PIN =                gpio_num_t::GPIO_NUM_6;  // D4
const gpio_num_t POWER_BUTTON_PIN_INV = gpio_num_t::GPIO_NUM_2;  // D0. Requires external Pullup
const int USB_DETECT_PIN =              gpio_num_t::GPIO_NUM_8;  // D8 Update to D9 so USB power wakes device. Requires external pulldown
// POWER_BUTTON_PIN will no longer be inverted. External pulldown attached.
// USB_DETECT_PIN updated to pin D9 in V2
// MOTOR_1 updated to pin D5 in V2
// MOTOR_2 updated to pin D4 in V2
// MOTOR_3 updated to pin D3 in V2
// MOTOR_4 updated to pin D2 in V2
// MOTOR_5 updated to pin D6 in V2
// MOTOR_6 updated to pin D10 in V2

enum ButtonState {BUTTON_STATE_OFF, BUTTON_STATE_PUSHED, BUTTON_STATE_ON, BUTTON_STATE_RELEASED};
ButtonState powerButtonState = ButtonState::BUTTON_STATE_OFF;

uint32_t msWhenPowButtonPushed = 0;
uint32_t msNext5vPowerPulse = 5000;

const uint64_t MS_BETWEEN_5V_POWER_PULSES = 10000;
const uint32_t MS_TO_HOLD_BEFORE_POWER_OFF = 2600;
const uint32_t MS_TO_HOLD_BEFORE_POWER_ON = 2600;

uint64_t numTimesMotorControllerFault = 0;

void IRAM_ATTR FaultInterruptHandler();
void IRAM_ATTR OnPowerButtonPushedInterruptHandler();
void CheckStatus();
void Set5VPower(bool powerState);
bool PowerButtonHeldForPowerOn();
void SetupLEDCChannels();
void SetupGPIOs();
void Pulse5VPower(void* parameters);
void PowerOff();
void FakePowerOff();
void PowerOn();
void UpdatePowerButtonState();

//RTC_DATA_ATTR uint64_t mode = 0;
bool isInProgramMode = true;
bool hasButtonBeenReleasedSinceBoot = false;

int timer = 167;           // The higher the number, the slower the timing.

int motorSelections[] = 
{
    4, 1, 2, 3, 1, 4, 2, 3, 1, 2, 4, 3, 13, 13, 13, 13, 13, 13, 13, 13,
    4, 3, 2, 1, 3, 1, 2, 4, 2, 4, 3, 1, 13, 13, 13, 13, 13, 13, 13, 13,
    3, 4, 1, 2, 1, 3, 2, 4, 2, 1, 4, 3, 13, 13, 13, 13, 13, 13, 13, 13,
    3, 4, 2, 1, 4, 3, 1, 2, 1, 4, 3, 2, 13, 13, 13, 13, 13, 13, 13, 13,
    2, 3, 1, 4, 1, 3, 4, 2, 1, 2, 3, 4, 13, 13, 13, 13, 13, 13, 13, 13,
    4, 2, 3, 1, 3, 2, 1, 4, 2, 1, 3, 4, 13, 13, 13, 13, 13, 13, 13, 13,
    3, 1, 4, 2, 4, 2, 1, 3, 2, 3, 4, 1, 13, 13, 13, 13, 13, 13, 13, 13,
    4, 1, 3, 2, 3, 2, 4, 1, 2, 4, 1, 3, 13, 13, 13, 13, 13, 13, 13, 13
};
// an array of pin numbers to which LEDs are attached Pin 13 is a pause
const int pinCount = sizeof(motorSelections) / sizeof(int);           // the number of pins (i.e. the length of the array)
int motorSelectionIdx = 0;

const int PWM_FREQ = 5000;
const int LEDC_CHANNEL1 = 0;
const int LEDC_CHANNEL2 = 1;
const int LEDC_CHANNEL3 = 2;
const int LEDC_CHANNEL4 = 3;
const int PWM_RES = 8;// PWM values of 0-255 are valid with 8 bits of resolution

/// @brief Generally speaking, the first function called by arduino.
void setup() {
    hasButtonBeenReleasedSinceBoot = false;
    SetupGPIOs();
    
    if(!PowerButtonHeldForPowerOn())// Waits here for MS_TO_HOLD_BEFORE_POWER_ON milliseconds or less before continueing
    {
        PowerOff();// Does not return
    }
    PowerOn();
    msNext5vPowerPulse = millis() + MS_BETWEEN_5V_POWER_PULSES;
    delay(10);// Give time for 5v to power up.
}

/// @brief Setup up GPIOs as inputs or outputs and attach interrupts.
void SetupGPIOs()
{
    pinMode(POWER_5V_PIN, OUTPUT);
    digitalWrite(POWER_5V_PIN, HIGH);
    pinMode(MOTOR_CONTROL_ENABLE_PIN, OUTPUT);

    SetupLEDCChannels();

    pinMode(FAULT_PIN, INPUT + PULLUP);
    attachInterrupt(FAULT_PIN, FaultInterruptHandler, FALLING);// If pin goes low, then motor driver has a fault and run this function.
    
    pinMode(POWER_BUTTON_PIN_INV, INPUT);
    gpio_pullup_en(POWER_BUTTON_PIN_INV);
    attachInterrupt(POWER_BUTTON_PIN_INV, OnPowerButtonPushedInterruptHandler, FALLING);
}

/// @brief Setup LEDC channels. These are used to generate PWM signals for motors.
void SetupLEDCChannels()
{
    ledcSetup(LEDC_CHANNEL1, PWM_FREQ, PWM_RES);
    ledcSetup(LEDC_CHANNEL2, PWM_FREQ, PWM_RES);
    ledcSetup(LEDC_CHANNEL3, PWM_FREQ, PWM_RES);
    ledcSetup(LEDC_CHANNEL4, PWM_FREQ, PWM_RES);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(MOTOR_1, LEDC_CHANNEL1);
    ledcAttachPin(MOTOR_2, LEDC_CHANNEL2);
    ledcAttachPin(MOTOR_3, LEDC_CHANNEL3);
    ledcAttachPin(MOTOR_4, LEDC_CHANNEL4);
}

/// @brief Main loop of arduino. This function gets called forever.
void loop() {
    static bool isFirstTime = true;
    if(isFirstTime)
    {
        vTaskPrioritySet(NULL, tskIDLE_PRIORITY + 2);
        isFirstTime = false;
    }
    CheckStatus();
    switch (motorSelections[motorSelectionIdx])
    { 
        case 1:
            ledcWrite(LEDC_CHANNEL1, 153); 
            delay(timer);
            ledcWrite(LEDC_CHANNEL1, 0);
        break;
        case 2:
            ledcWrite(LEDC_CHANNEL2, 153);
            delay(timer);
            ledcWrite(LEDC_CHANNEL2, 0);
        break;
        case 3:
            ledcWrite(LEDC_CHANNEL3, 153); 
            delay(timer);
            ledcWrite(LEDC_CHANNEL3, 0);
        break;
        case 4:
            ledcWrite(LEDC_CHANNEL4, 153); 
            delay(timer);
            ledcWrite(LEDC_CHANNEL4, 0);
        break;
        case 13:
            delay(timer);
        break;
    }
    motorSelectionIdx = (motorSelectionIdx + 1) % pinCount;
}

/// @brief Check current power button state and do any interactions associated with that state.
void UpdatePowerButtonState()
{
    if(powerButtonState == ButtonState::BUTTON_STATE_ON)
    {
    }
    else if(powerButtonState == ButtonState::BUTTON_STATE_OFF)
    {
        hasButtonBeenReleasedSinceBoot = true;
    }
    else if(powerButtonState == BUTTON_STATE_PUSHED)
    {
        msWhenPowButtonPushed = millis();
        powerButtonState = ButtonState::BUTTON_STATE_ON;
    }
    else if(powerButtonState == BUTTON_STATE_RELEASED)
    {
        powerButtonState = ButtonState::BUTTON_STATE_OFF;
    }
}

/// @brief Checks status of buttons and runs 5v power pulses. Turns off microcontroller if requirements met.
void CheckStatus()
{
    UpdatePowerButtonState();
    if(((millis() - msWhenPowButtonPushed) > MS_TO_HOLD_BEFORE_POWER_OFF) && (powerButtonState != ButtonState::BUTTON_STATE_OFF && powerButtonState != BUTTON_STATE_RELEASED) && hasButtonBeenReleasedSinceBoot)
    {
        PowerOff();// Does not return
    }
    if(millis() > msNext5vPowerPulse && powerButtonState != ButtonState::BUTTON_STATE_ON && powerButtonState != ButtonState::BUTTON_STATE_PUSHED)
    {
        msNext5vPowerPulse += MS_BETWEEN_5V_POWER_PULSES;
        xTaskCreate(Pulse5VPower, "Pulse5VPower", 6000, NULL, tskIDLE_PRIORITY + 1, NULL);// Create non-blocking task to keep power on.
    }
    powerButtonState = digitalRead(POWER_BUTTON_PIN_INV)? ButtonState::BUTTON_STATE_OFF : ButtonState::BUTTON_STATE_ON;
}

/// @brief One-shot ISR for when motor controller produces a fault. 
/// This can happen because either the motor controller is overheating or the current limit has been exceeded.
/// IRAM_ATTR used to keep function in RAM.
void IRAM_ATTR FaultInterruptHandler()
{
    numTimesMotorControllerFault += 1;
}

/// @brief One-shot ISR for when power button pushed down.
/// IRAM_ATTR used to keep function in RAM.
void IRAM_ATTR OnPowerButtonPushedInterruptHandler()
{
    powerButtonState = BUTTON_STATE_PUSHED;
}

/// @brief If usb plugged in, don't go to sleep. Just wait for user to power on device and restart on button push.
void FakePowerOff()
{
    while(digitalRead(POWER_BUTTON_PIN_INV)){delay(10);}
    esp_restart();
}

/// @brief This function will not return unless plugged into USB.
void PowerOff()
{
    digitalWrite(MOTOR_CONTROL_ENABLE_PIN, false); // Turn off Motor Controller.
    Set5VPower(false); // Turn off 5v supply for motors.
    if(!digitalRead(USB_DETECT_PIN)) // If USB not plugged in.
    {
        esp_deep_sleep_enable_gpio_wakeup(1 << uint64_t(POWER_BUTTON_PIN_INV), esp_deepsleep_gpio_wake_up_mode_t::ESP_GPIO_WAKEUP_GPIO_LOW);
        while(!digitalRead(POWER_BUTTON_PIN_INV));// Wait until button released
        esp_deep_sleep_start(); // Go into deep sleep. Will only wake on power button being pushed.
    }
    else 
    {
        FakePowerOff();
    }
    return;
}

/// @brief Power on motor controller and 5v power rail.
void PowerOn()
{
    digitalWrite(MOTOR_CONTROL_ENABLE_PIN, true); // Turn on Motor Controller.
    Set5VPower(true); // Turn on 5v supply for motors.
}

/// @brief Task to pulse the 5v power "on" signal. Must be done within every 30 seconds.
/// @param parameters 
void Pulse5VPower(void* parameters)
{
    Set5VPower(true);
    vTaskDelete(NULL);
}

/// @brief Sets 5v power rail to on or off state. Blocks for up to 1.5 seconds.
/// @param powerState true for 5v on. false for off.
void Set5VPower(bool powerState)
{
    if(powerState)
    {
        // Serial.println("Pulsing 5v power on");
        digitalWrite(POWER_5V_PIN, LOW);
        delay(400);
        digitalWrite(POWER_5V_PIN, HIGH);
    }
    else
    {
        // Serial.println("Pulsing 5v power off");
        digitalWrite(POWER_5V_PIN, LOW);
        delay(400);
        digitalWrite(POWER_5V_PIN, HIGH);
        delay(400);
        digitalWrite(POWER_5V_PIN, LOW);
        delay(400);
        digitalWrite(POWER_5V_PIN, HIGH);
    }
}

/// @brief Blocks and checks if user has held the power button for "MS_TO_HOLD_BEFORE_POWER_ON" ms.
/// @return Returns true if power button held for enough time. false if button was held for any shorter or not pressed at all.
bool PowerButtonHeldForPowerOn()
{
    while((millis() - msWhenPowButtonPushed) <= MS_TO_HOLD_BEFORE_POWER_ON)
    {
        if(digitalRead(POWER_BUTTON_PIN_INV))// If button released early
        {
            powerButtonState = ButtonState::BUTTON_STATE_OFF;
            return false;
        }
        powerButtonState = ButtonState::BUTTON_STATE_ON;
        delay(50);
    }
    msWhenPowButtonPushed = INT_MAX;
    return true;
}