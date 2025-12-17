#include <Arduino.h>
#include <AccelStepper.h>
#include <cstdint>

#include "Button.hpp"

constexpr int PWM_A = PIN_D3;
constexpr int DIR_A = PIN_D12;

AccelStepper stepper{ AccelStepper::DRIVER, PWM_A, DIR_A };

constexpr int stepsPerRevolution = 400; // for 1.8 degree stepper motor with 1/2 microstepping

enum class ButtonID : uint8_t
{
    Default = 0,
    Start = 1,
    Stop = 2
};

struct ButtonTag
{
    ButtonID id{};
    const char* name = "";
};

using PushButton = Button<ButtonTag>;

constexpr ButtonPinStatus buttonActiveLevel = ButtonPinStatus::Low; // buttons connect to GND when pressed
constexpr PushButton::ClockType longPressMs = 2000;

PushButton startButton{ PIN_D8, buttonActiveLevel, longPressMs, { .id = ButtonID::Start, .name = "Start" } };
PushButton stopButton{ PIN_D9, buttonActiveLevel, longPressMs, { .id = ButtonID::Stop, .name = "Stop" } };

bool stepperIsMoving = false;
bool requestMoveForward = false;
bool requestQuickStop = false;

void handlePress(PushButton& btn)
{
    const ButtonTag& tag = btn.tag();
    switch (tag.id)
    {
        case ButtonID::Start:
        {
            requestMoveForward = true;
            Serial.println("Pressed Start");
            break;
        }
        case ButtonID::Stop:
        {
            requestQuickStop = true;
            Serial.println("Pressed Stop - QUICK STOP requested!");
            break;
        }
        default:
        {
            Serial.println("Unknown button pressed");
            break;
        }
    }
}

void handleLongPress(PushButton& btn)
{
    const ButtonTag& tag = btn.tag();
    switch (tag.id)
    {
        case ButtonID::Start:
        {
            Serial.println("Long pressed Start");
            break;
        }
        case ButtonID::Stop:
        {
            Serial.println("Long pressed Stop");
            break;
        }
        default:
        {
            Serial.println("Unknown button long pressed");
            break;
        }
    }
}

void handleRelease(PushButton& btn)
{
    const ButtonTag& tag = btn.tag();
    switch (tag.id)
    {
        case ButtonID::Start:
        {
            Serial.println("Released Start");
            break;
        }
        case ButtonID::Stop:
        {
            Serial.println("Released Stop");
            break;
        }
        default:
        {
            Serial.println("Unknown button released");
            break;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    pinMode(DIR_A, OUTPUT);
    pinMode(PWM_A, OUTPUT);

    stepper.setMinPulseWidth(20);
    //stepper.setSpeed(400); // constant speed mode
    stepper.setMaxSpeed(3000);
    stepper.setAcceleration(3000);

    startButton.begin();
    startButton.setHandler(ButtonEvent::Press, handlePress);
    startButton.setHandler(ButtonEvent::LongPress, handleLongPress);
    startButton.setHandler(ButtonEvent::Release, handleRelease);

    stopButton.begin();
    stopButton.setHandler(ButtonEvent::Press, handlePress);
    stopButton.setHandler(ButtonEvent::LongPress, handleLongPress);
    stopButton.setHandler(ButtonEvent::Release, handleRelease);

    Serial.println("Setup complete!");
    Serial.println("Button A: Start motor");
    Serial.println("Button B: Quick stop motor");
}

void loop()
{
    startButton.update();
    stopButton.update();

    if (requestQuickStop)
    {
        stepper.stop();

        stepperIsMoving = false;
        requestQuickStop = false;
        requestMoveForward = false;

        Serial.println("Quick stop executed");
    }
    else if (requestMoveForward && !stepperIsMoving)
    {
        stepperIsMoving = true;
        requestMoveForward = false;

        stepper.move(10 * stepsPerRevolution);

        Serial.println("Forward movement started");
    }

    if (stepperIsMoving && stepper.distanceToGo() == 0)
    {
        stepperIsMoving = false;
        Serial.println("Movement completed");
    }

    stepper.run();
}
