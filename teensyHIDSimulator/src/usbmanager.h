#ifndef USBMANAGER_H
#define USBMANAGER_H

#include "types.h"

union USBPackage
{
    struct Data {
        // Actual output (25 bytes)
        vec2f position;
        float angle;
        vec2f velocity;
        float angularVelocity;
        byte  button;

        // Raw sensor data (16 bytes)
        vec2f leftSensor;
        vec2f rightSensor;

        // Debug data (8 bytes)
        vec2s leftIncrement;    // Contains sensor delta since last USB frame
        vec2s rightIncrement;   // Contains sensor delta since last USB frame

        // Simulation info (8 bytes)
        float timeStep;
        float time;

        // Unused
        byte unused[7];
    } data;

    char raw[64];
};

enum USBOpCodes
{
    Invalid = 0,
    CalibrationData = 1,
    SimulationData = 2,
    SetColor = 3,
    SaveConfig = 4,
    Calibrate = 5
};

class USBManager
{
public:
    USBManager();

    void sendFrame(const USBPackage& package);
    void checkIncoming();

    std::function<void()>& onTimeOut();
    std::function<void()>& onUSBError();

    std::function<void()>& onSaveConfiguration();
    std::function<void()>& onCalibrate();
    std::function<void(vec2f)>& onCalibrationData();
    std::function<void(color)>& onLEDColor();
    std::function<void(SimulationParameters)>& onParameters();

private:
    std::function<void()> timeOut;
    std::function<void()> USBError;
    std::function<void()> saveConfiguration;
    std::function<void()> calibrate;
    std::function<void(vec2f)> calibrationData;
    std::function<void(color)> LEDColor;
    std::function<void(SimulationParameters)> parameters;
};

#endif // USBMANAGER_H
