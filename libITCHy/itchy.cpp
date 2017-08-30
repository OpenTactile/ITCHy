#include "itchy/itchy.h"

extern "C" {
#include "pjrc_rawhid.h"
}

#include <unistd.h>
#include <vector>
#include <map>

template<typename T>
unsigned int typeToBuffer(char* buffer,const T& type, unsigned int start=0, unsigned int typeLength=1) {
    char* p = (char *)((void *)(&type));
    buffer += start;
    unsigned int typeSize = sizeof(T)*typeLength;
    unsigned int i;
    for(i=0; i<typeSize; i++) {
        *buffer++ = *p++;
    }
    return i;
}

template<typename T>
unsigned int bufferToType(const char* buffer, T& type, unsigned int start=0, unsigned int typeLength=1) {
    char* p = (char *)((void *)(&type));
    buffer += start;
    unsigned int typeSize = sizeof(T)*typeLength;
    unsigned int i;
    for(i=0; i<typeSize; i++) {
        *p++ = *buffer++;
    }
    return i;
}

class ITCHyImplementation
{
public:

    enum USBOpCodes
    {
        Invalid = 0,
        CalibrationData = 1,
        SimulationData = 2,
        SetColor = 3,
        SaveConfig = 4,
        Calibrate = 5
    };

    DeviceIdentifier identifier;
    bool connected = false;

    std::map<ITCHy::CallbackType,
             std::vector<std::function<void()>>> callbacks;

    void callAll(ITCHy::CallbackType type)
    {
        for(auto& fun : callbacks[type])
        {
            fun();
        }
    }

    ITCHy::State lastState;
};

ITCHy::ITCHy() :
    ITCHy({0x16C0, 0x0486, 0xFFAB, 0x0200})
{
}

ITCHy::ITCHy(DeviceIdentifier identifier)
{
    impl = new ITCHyImplementation();

    impl->identifier = identifier;
    impl->lastState.leftSensor = {{0.0, 0.0}};
    impl->lastState.rightSensor = {{0.0, 0.0}};
}

ITCHy::~ITCHy()
{
    delete impl;
}

void ITCHy::connect()
{
    while(!tryConnect())
    {
        usleep(10000);
    }
}

bool ITCHy::tryConnect()
{
    if(impl->connected)
    {
        return true;
    }

    int ret = rawhid_open(1,
                          impl->identifier[0], impl->identifier[1],
                          impl->identifier[2], impl->identifier[3]);

    // Device not found
    if(ret <= 0)
    {
        return false;
    }

    // Device found
    impl->connected = true;
    impl->callAll(CallbackType::Connected);
    return true;
}

void ITCHy::disconnect()
{
    if(impl->connected)
    {
        rawhid_close(0);
        impl->connected = false;
        impl->callAll(CallbackType::Disconnected);
    }
}

bool ITCHy::connected() const
{
    return impl->connected;
}

bool ITCHy::setCalibrationParameters(const vec2f& target)
{
    if(!impl->connected)
    {
        return false;
    }

    char buffer[64] = {0};
    int p = 0;
    char opcode = ITCHyImplementation::CalibrationData;
    p += typeToBuffer(buffer, opcode, p);
    p += typeToBuffer(buffer, target[0], p, 2);

    int ret = rawhid_send(0, buffer, 64, 1000);

    if(ret <= 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        return false;
    }

    return true;
}

bool ITCHy::setSimulationParameters(
        float mass,
        float stiffness,
        float damping,
        int updateRate
        )
{
    if(!impl->connected)
    {
        return false;
    }

    char buffer[64] = {0};
    int p = 0;
    char opcode = ITCHyImplementation::SimulationData;
    p += typeToBuffer(buffer, opcode, p);
    p += typeToBuffer(buffer, damping, p);
    p += typeToBuffer(buffer, mass, p);
    p += typeToBuffer(buffer, stiffness, p);
    p += typeToBuffer(buffer, updateRate, p);

    int ret = rawhid_send(0, buffer, 64, 1000);

    if(ret <= 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        return false;
    }

    return true;
}

bool ITCHy::setColor(const color& cl)
{
    if(!impl->connected)
    {
        return false;
    }

    char buffer[64] = {0};
    int p = 0;
    char opcode = ITCHyImplementation::SetColor;
    p += typeToBuffer(buffer, opcode, p);
    p += typeToBuffer(buffer, cl[0], p, 3);

    int ret = rawhid_send(0, buffer, 64, 50);

    if(ret <= 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        return false;
    }

    return true;
}

bool ITCHy::startCalibration()
{
    if(!impl->connected)
    {
        return false;
    }

    char buffer[64] = {0};
    int p = 0;
    char opcode = ITCHyImplementation::Calibrate;
    p += typeToBuffer(buffer, opcode, p);

    int ret = rawhid_send(0, buffer, 64, 1000);


    if(ret <= 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        return false;
    }

    return true;
}

bool ITCHy::saveState()
{
    if(!impl->connected)
    {
        return false;
    }

    char buffer[64] = {0};
    int p = 0;
    char opcode = ITCHyImplementation::SaveConfig;
    p += typeToBuffer(buffer, opcode, p);

    int ret = rawhid_send(0, buffer, 64, 1000);


    if(ret <= 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        return false;
    }

    return true;
}

const ITCHy::State& ITCHy::currentState(unsigned int timeout)
{
    if(!impl->connected)
    {
        return impl->lastState;
    }

    union
    {
        State newState;
        char buffer[64];
    } data;

    // USB Read
    int num = rawhid_recv(0, data.buffer, 64, static_cast<int>(timeout));

    // Error
    if(num < 0)
    {
        impl->callAll(CallbackType::CommunicationError);
        disconnect();
    }
    else
    {
        State newState = data.newState;
        impl->lastState = newState;
    }

    return impl->lastState;
}


void ITCHy::addCallback(
        CallbackType type, const std::function<void()>& callback)
{
    impl->callbacks[type].push_back(callback);
}
