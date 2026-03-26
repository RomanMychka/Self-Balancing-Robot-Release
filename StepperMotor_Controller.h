#pragma once
#include <Arduino.h>

enum StepDir : uint8_t
{
    FORWARD = HIGH,
    BACKWARD = LOW
};

class StepperMotor_Controller
{

private:
    // Піни
    const uint8_t stepPin, dirPin, enPin;
    const float maxSpeed;

    // Стан двигуна
    bool motionEnabled = false;
    StepDir currentDirection = FORWARD;
    float currentSpeed = 0.0f;
    uint32_t pulseIntervalUs = 0; 

    unsigned long lastStepUs = 0;
    unsigned long pulseStartUs = 0;
    bool pulseActive = false;
    volatile long currentPosition = 0;

    static constexpr uint32_t MIN_PULSE_WIDTH_US = 2; // >= 2 мкс для DRV8825

public:
    StepperMotor_Controller(uint8_t stepPin, uint8_t dirPin, uint8_t enPin, float maxSpeed)
        : stepPin(stepPin), dirPin(dirPin), enPin(enPin), maxSpeed(maxSpeed) {}

    // --- begin() — викликається в setup() ---
    // Налаштовує піни та початковий стан
    void begin()
    {
        pinMode(stepPin, OUTPUT);
        pinMode(dirPin, OUTPUT);
        pinMode(enPin, OUTPUT);

        digitalWrite(stepPin, LOW);
        digitalWrite(dirPin, FORWARD);
        digitalWrite(enPin, HIGH); // HIGH = вимкнено (DRV8825 active-LOW)

        lastStepUs = micros();
    }

    // --- Вмикає або вимикає двигун. При вимкненні — зупиняє і скидає стан ---
    void setEnabled(bool en)
    {
        motionEnabled = en;
        digitalWrite(enPin, en ? LOW : HIGH);
        if (!en)
        {
            currentSpeed = 0.0f;
            pulseIntervalUs = 0;
            pulseActive = false;
            digitalWrite(stepPin, LOW);
        }
    }

    // --- Встановлює швидкість у кроках/секунду ---
    void setSpeed(float speed)
    {
        currentSpeed = constrain(fabsf(speed), 0.0f, maxSpeed);

        // Обчислюємо інтервал між імпульсами в мікросекундах.
        // pulseIntervalUs — це час ПОВНОГО циклу (HIGH + LOW).
        // HIGH триматиметься MIN_PULSE_WIDTH_US (2 мкс),
        // решта часу — LOW (очікування наступного кроку).
        pulseIntervalUs = (currentSpeed > 0.5f)
                              ? (uint32_t)(1000000.0f / currentSpeed)
                              : 0;
    }

    // --- Встановлює напрямок ---
    void setDir(StepDir dir)
    {
        currentDirection = dir;
        digitalWrite(dirPin, static_cast<uint8_t>(dir));
    }

    // --- Генерує імпульси — викликати якомога частіше в loop() ---
    void run()
    {
        if (pulseIntervalUs == 0 || !motionEnabled)
        {
            pulseActive = false;
            return;
        }

        unsigned long now = micros();

        if (pulseActive)
        {
            // Чекаємо завершення HIGH-імпульсу
            if (now - pulseStartUs >= MIN_PULSE_WIDTH_US)
            {
                digitalWrite(stepPin, LOW);
                pulseActive = false;
                lastStepUs = now;
            }
        }
        else
        {
            // Час наступного кроку?
            if (now - lastStepUs >= pulseIntervalUs)
            {
                digitalWrite(stepPin, HIGH);
                pulseStartUs = now;
                pulseActive = true;
                currentPosition += (currentDirection == FORWARD) ? 1 : -1;
            }
        }
    }

    float getSpeed() const { return currentSpeed; }
    StepDir getDir() const { return currentDirection; }
    long getPosition() const { return currentPosition; }
    bool isEnabled() const { return motionEnabled; }
    void resetPos() { currentPosition = 0; }
};