#pragma once
#include <Arduino.h>
#include <MPU6050_tockn.h>

#include "StepperMotor_Controller.h"
#include "PID_Manager.h"
#include "ControlPage_Routes.h"
#include "NetworkConnection_Manager.h"

class RobotController_Controller
{
private:
    MPU6050 &mpu;
    StepperMotor_Controller &leftMotor;
    StepperMotor_Controller &rightMotor;
    PID_Manager &pid;
    ControlPage_Router &webServer;
    NetworkConnection_Manager &network;

    // ── Константи ──────────────────────────────────────────
    static constexpr float FALL_ANGLE = 70.0f;
    static constexpr unsigned long OUTER_LOOP_MS = 50; 
    static constexpr unsigned long INNER_LOOP_MS = 20;  

    float maxMotorSpeed;
    float maxSteer;

    bool fallen = false;
    float targetAngle = 0.0f;

    unsigned long lastOuterMs = 0;
    unsigned long lastInnerMs = 0;
    unsigned long lastLogMs = 0;

public:
    RobotController_Controller(
        MPU6050 &mpu,
        StepperMotor_Controller &left,
        StepperMotor_Controller &right,
        PID_Manager &pid,
        ControlPage_Router &webServer,
        NetworkConnection_Manager &network,
        float maxSpeed = 50000.0f,
        float maxSteer = 2000.0f) : mpu(mpu), leftMotor(left), rightMotor(right),
                                    pid(pid), webServer(webServer), network(network),
                                    maxMotorSpeed(maxSpeed), maxSteer(maxSteer)
    {
    }

    // === Ініціалізація ===
    void begin()
    {
        leftMotor.begin();
        rightMotor.begin();
        leftMotor.setEnabled(true);
        rightMotor.setEnabled(true);

        pid.begin();

        webServer.setupRoutes();

        unsigned long now = millis();
        lastOuterMs = lastInnerMs = lastLogMs = now;

    }

    // === Головний цикл — викликати кожну ітерацію loop() ===
    void run()
    {

        leftMotor.run();
        rightMotor.run();

        unsigned long now = millis();


        if (now - lastInnerMs < INNER_LOOP_MS)
            return;
        lastInnerMs = now;

        mpu.update();
        float currentAngle = mpu.getAngleY();


        if (fabsf(currentAngle) > FALL_ANGLE)
        {
            handleFall(currentAngle);
            return;
        }

        else if (fabsf(targetAngle) <= FALL_ANGLE)
        {
            resetFallen();
        }

        if (now - lastOuterMs >= OUTER_LOOP_MS)
        {
            float dt = (now - lastOuterMs) / 1000.0f;
            lastOuterMs = now;

            Command cmd = webServer.getCommand();

            float targetSpeed = (float)map(cmd.y, -100, 100,
                                           -(long)maxMotorSpeed, (long)maxMotorSpeed);


            targetAngle = pid.updateOutter(targetSpeed);
        }


        float baseSpeed = pid.updateInner(currentAngle);

        // ── Поворот з джойстика ─────────────────────────────
        Command cmd = webServer.getCommand();
        float turnDiff = (float)map(cmd.x, -100, 100,
                                    -(long)maxSteer, (long)maxSteer);

        float leftSpeed = constrain(baseSpeed + turnDiff, -maxMotorSpeed, maxMotorSpeed);
        float rightSpeed = constrain(baseSpeed - turnDiff, -maxMotorSpeed, maxMotorSpeed);

        applyMotorSpeeds(leftSpeed, rightSpeed);

    }

    // ── Відновлення після падіння 
    void resetFallen()
    {
        fallen = false;
        pid.setActive(true);
    }

    bool isFallen() const { return fallen; }

private:
    void applyMotorSpeeds(float leftSpeed, float rightSpeed)
    {
        // Лівий мотор: BACKWARD = рух вперед (дзеркальний монтаж)
        leftMotor.setDir(leftSpeed >= 0 ? BACKWARD : FORWARD);
        leftMotor.setSpeed(fabsf(leftSpeed));

        rightMotor.setDir(rightSpeed >= 0 ? FORWARD : BACKWARD);
        rightMotor.setSpeed(fabsf(rightSpeed));
    }

    void handleFall(float angle)
    {
        leftMotor.setSpeed(0);
        rightMotor.setSpeed(0);
        pid.setActive(false); // скидає інтеграли
        fallen = true;
    }

};