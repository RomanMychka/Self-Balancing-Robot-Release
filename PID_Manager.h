#pragma once
#include <Arduino.h>

#define MAX_OUTTER_ANGLE 8.0f

class PID_Manager
{

private:
    float kP, kI, kD, kOutter;
    float maxIntegral, maxOutput;

    float balanceOffset = 0.0f;

    float innerIntegral = 0.0f;
    float lastError = 0.0f;
    float outterIntegral = 0.0f;
    float targetAngle = 0.0f;

    // ✨ НОВИЙ КОД: зберігаємо останній вихід PID
    float lastPidOutput = 0.0f;

    unsigned long lastInnerMs = 0;
    unsigned long lastOuterMs = 0;

    bool active = false;

public:
    PID_Manager(float kP, float kI, float kD, float kpOutter,
                float maxIntegral, float maxOutput,
                float balanceOffset = 0.0f)
        : kP(kP), kI(kI), kD(kD), kOutter(kpOutter),
          maxIntegral(maxIntegral), maxOutput(maxOutput),
          balanceOffset(balanceOffset) {}

    void begin()
    {
        reset();
        lastInnerMs = millis();
        lastOuterMs = millis();
        active = true;
    }

    // --- Зовнішній контур ---
    // ✨ ВИПРАВЛЕНО: використовуємо вихід PID як оцінку реальної швидкості
    float updateOutter(float targetSpeed)
    {
        if (!active)
            return 0.0f;

        unsigned long now = millis();
        float dt = (now - lastOuterMs) / 1000.0f;
        lastOuterMs = now;

        dt = constrain(dt, 0.0005f, 0.1f);

        // ✨ КЛЮЧОВА ЗМІНА: віднімаємо реальну швидкість (= вихід PID) від інтегралу
        // Логіка: якщо мотори вже крутяться з певною швидкістю (lastPidOutput),
        // то інтеграл повинен зменшуватися на цю величину
        float speedError = targetSpeed - lastPidOutput;
        outterIntegral += speedError * dt;
        outterIntegral = constrain(outterIntegral, -maxOutput, maxOutput);

        // Нормалізуємо: ділимо на maxOutput -> [-1..1]
        // Множимо на kOutter -> кут у градусах
        targetAngle = kOutter * (outterIntegral / maxOutput);
        targetAngle = constrain(targetAngle, -MAX_OUTTER_ANGLE, MAX_OUTTER_ANGLE);

        return targetAngle;
    }

    // --- Внутрішній контур ---
    // PID за кутом нахилу -> швидкість моторів
    float updateInner(float currentAngle)
    {
        if (!active)
            return 0.0f;

        unsigned long now = millis();
        float dt = (now - lastInnerMs) / 1000.0f;
        lastInnerMs = now;

        dt = constrain(dt, 0.0005f, 0.1f);

        float angle = currentAngle - balanceOffset;
        float error = targetAngle - angle;

        float P = kP * error;

        innerIntegral += error * dt;
        innerIntegral = constrain(innerIntegral, -maxIntegral, maxIntegral);

        float I = kI * innerIntegral;

        float D = kD * (error - lastError) / dt;
        lastError = error;

        // ✨ НОВИЙ КОД: зберігаємо вихід для зовнішнього контуру
        lastPidOutput = constrain(P + I + D, -maxOutput, maxOutput);
        
        return lastPidOutput;
    }

    // --- Скидає весь накопичений стан ---
    void reset()
    {
        innerIntegral = 0.0f;
        lastError = 0.0f;
        outterIntegral = 0.0f;
        targetAngle = 0.0f;
        lastPidOutput = 0.0f; // ✨ НОВИЙ КОД
    }

    void setActive(bool on)
    {
        active = on;
        if (!on)
            reset();
    }
    bool isActive() const { return active; }
    float getTargetAngle() const { return targetAngle; }
    float getEstimatedSpeed() const { return lastPidOutput; } // ✨ Повертаємо вихід PID

    void setKp(float p) { kP = p; }
    void setKi(float i)
    {
        kI = i;
        innerIntegral = 0.0f;
    }
    void setKd(float d) { kD = d; }
    void setKpSpeed(float speed) { kOutter = speed; }
    void setMaxIntegral(float integral)
    {
        maxIntegral = integral;
        innerIntegral = 0.0f;
    }
    void setMaxOutput(float output) { maxOutput = output; }
    void setBalanceOffset(float offset) { balanceOffset = offset; }
};