#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

#include "ControlPage_WebPage.h"
#include "ControlPage_Routes.h"
#include "NetworkConnection_Manager.h"
#include "StepperMotor_Controller.h"
#include "PID_Manager.h"
#include "RobotController_Controller.h"

// =========================================================
//  ПІНИ
// =========================================================
const uint8_t leftStepPin = 33;
const uint8_t leftDirPin = 14;
const uint8_t leftEnablePin = 26;

const uint8_t rightStepPin = 32;
const uint8_t rightDirPin = 25;
const uint8_t rightEnablePin = 27;

const uint8_t SDA_PIN = 21;
const uint8_t SCL_PIN = 22;

// =========================================================
//  WiFi налаштування
// =========================================================
#define WIFI_AP_SSID "BalanceBot"
#define WIFI_AP_PASS "12345678"

// =========================================================
//  ОБ'ЄКТИ
// =========================================================

MPU6050 mpu6050(Wire);

StepperMotor_Controller leftMotor(leftStepPin, leftDirPin, leftEnablePin, 15000.0f);
StepperMotor_Controller rightMotor(rightStepPin, rightDirPin, rightEnablePin, 15000.0f);

PID_Manager pid(
    800.0f,   // kP
    200.0f,     // kI
    5.0f,     // kD
    7.0f,     // kpOutter
    500.0f,   // maxIntegral
    15000.0f, // maxOutput
    1.0f      // balanceOffset
);

NetworkConnection_Manager network( 
    "", // STA SSID 
    "", // STA pass
    WIFI_AP_SSID,
    WIFI_AP_PASS);

ControlPage_Router webServer(controlPageHTML);

RobotController_Controller robot(
    mpu6050,
    leftMotor,
    rightMotor,
    pid,
    webServer,
    network);

// =========================================================
//  SETUP
// =========================================================
void setup()
{
    Serial.begin(115200);

    // I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    // MPU6050
    mpu6050.begin();
    mpu6050.setGyroOffsets(0.0f, 0.0f, 0.0f); 

    // Підключення до WiFi
    network.setupLocalWiFi();
    network.printStatus();

    // Старт основного коду
    robot.begin();

    
    Serial.printf("\n📱 Open: http://%s\n\n", network.getIP().c_str());
}

// =========================================================
//  LOOP
// =========================================================
void loop()
{
    robot.run();
}