#include "usbmanager.h"
#include <WProgram.h>

#include <usb_rawhid.h>
#include "util/TypeBuffer.h"

#include "statusled.h"

// Ugly hack to make stdlib work
// I am aware that using the stdlib on an embedded device is generally not
// a good idea. But the teensy has plenty of space left and std::functions
// are much nicer than plain function pointers...
extern "C"{
    int _getpid(){ return -1;}
    int _kill(int pid, int sig){ return -1; }
}

USBManager::USBManager()
{

}

void USBManager::sendFrame(const USBPackage &package)
{
    int ret = RawHID.send(package.raw, 2);
    if(ret < 0) // USB Error
    {
        USBError();
    }
    else if(ret == 0) // Timeout
    {
        timeOut();
    }
}

void USBManager::checkIncoming()
{
    char buffer[64];
    int ret = RawHID.recv(buffer, 0); // Do not wait

    if(ret < 0) // USB Error
    {
        USBError();
    }

    if(ret > 0) // Parse command
    {
        int p = 0;
        char mode = Invalid;
        p += bufferToType(buffer, mode, p);

        switch(mode)
        {
        case Invalid:
            // Ignore
            break;

        case CalibrationData:
        {
            vec2f calibrationTarget;
            p += bufferToType(buffer, calibrationTarget[0], p, 2);
            calibrationData(calibrationTarget);
            break;
        }

        case SimulationData:
        {
            SimulationParameters params;
            p += bufferToType(buffer, params.damping, p);
            p += bufferToType(buffer, params.mass, p);
            p += bufferToType(buffer, params.stiffness, p);
            p += bufferToType(buffer, params.updateRate, p);
            parameters(params);
            break;
        }

        case SetColor:
        {
            color cl;
            p += bufferToType(buffer, cl[0], p, 3);
            LEDColor(cl);
            break;
        }

        case SaveConfig:
            saveConfiguration();
            break;

        case Calibrate:
            calibrate();
            break;
        }
    }
}

std::function<void()>& USBManager::onTimeOut()
{
    return timeOut;
}

std::function<void()>& USBManager::onUSBError()
{
    return USBError;
}

std::function<void()>& USBManager::onSaveConfiguration()
{
    return saveConfiguration;
}

std::function<void()>& USBManager::onCalibrate()
{
    return calibrate;
}

std::function<void(vec2f)>& USBManager::onCalibrationData()
{
    return calibrationData;
}

std::function<void(color)> &USBManager::onLEDColor()
{
    return LEDColor;
}

std::function<void(SimulationParameters)>& USBManager::onParameters()
{
    return parameters;
}
