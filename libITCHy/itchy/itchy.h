#ifndef ITCHY_H
#define ITCHY_H

#include <cstdint>
#include <array>
#include <functional>

using DeviceIdentifier = std::array<int32_t, 4>;
using vec2f = std::array<float, 2>;
using vec2s = std::array<int16_t, 2>;
using color = std::array<uint8_t, 3>;
using byte = uint8_t;

class ITCHyImplementation;

class ITCHy
{
public:
    enum class CallbackType
    {
        // The device has been connected successfully
        Connected,

        // The device went offline (e.g. user removed cable)
        Disconnected,

        // An USB package was not received correctly
        CommunicationError
    };

    struct State {
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
    };

public:
    ITCHy();
    ITCHy(DeviceIdentifier identifier);
    ~ITCHy();

    void connect();
    bool tryConnect();
    void disconnect();
    bool connected() const;


    bool setCalibrationParameters(const vec2f& target);
    bool setSimulationParameters(
            float mass,
            float stiffness,
            float damping,
            int updateRate
            );
    bool setColor(const color& cl);
    bool startCalibration();
    bool saveState();

    const State& currentState(unsigned int timeout = 50);

    void addCallback(CallbackType type, const std::function<void()>& callback);


private:
    ITCHyImplementation* impl;
};

#endif // ITCHY_H
