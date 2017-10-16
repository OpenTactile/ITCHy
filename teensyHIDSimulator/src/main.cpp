#include "WProgram.h"

#include "types.h"
#include "statusled.h"
#include "usbmanager.h"
#include "sensor.h"
#include "button.h"

#include <SPI.h>
#include <elapsedMillis.h>
#include <EEPROM.h>

enum class State
{
    Init,
    CalibrateX,
    CalibrateY,
    Simulate
};

constexpr int eepromSize()
{
    return sizeof(SimulationParameters) + sizeof(vec2f) + 2*sizeof(Sensor::CalibrationState);
}

union
{
    struct
    {
        SimulationParameters parameters;
        vec2f calibration;
        Sensor::CalibrationState calibStateLeft;
        Sensor::CalibrationState calibStateRight;
    } data;
    byte raw[eepromSize()];
} eepData;

static elapsedMillis usbTimeout;
static elapsedMicros simTime;

const float sensorDistance = 0.0756019f;

const vec2f sensorLeftOffset = {
    {-0.0124523f, 0.0356911f}
};

const vec2f sensorRightOffset = {
    {0.0124523f, -0.0356911f}
};

const float staticAngle = atan2(sensorLeftOffset[1], sensorLeftOffset[0]);

static vec2f calibration;

const color defaultColor = {{255, 64, 0}};


void resetSimulation(SimulationState& sim)
{
    sim.dt = 0.0;
    sim.time = 0.0;

    // Set relative positions of sensors:
    sim.positionLeft = sensorLeftOffset;
    sim.positionRight = sensorRightOffset;

    sim.rawLeft = sim.positionLeft;
    sim.rawRight = sim.positionRight;

    sim.velocityLeft = {{0.0, 0.0}};
    sim.velocityRight = {{0.0, 0.0}};

    sim.position = {{0.0f, 0.0f}};
    sim.velocity = {{0.0f, 0.0f}};
    sim.angle = 0.0f;
    sim.angularVelocity = 0.0f;
    sim.rotation = {{1.0f, 0.0f, 0.0f, 1.0f}};
}

void resetRotation(SimulationState& sim)
{
    // Set relative positions of sensors:
    sim.positionLeft[0] = sensorLeftOffset[0] + sim.position[0];
    sim.positionLeft[1] = sensorLeftOffset[1] + sim.position[1];

    sim.positionRight[0] = sensorRightOffset[0] + sim.position[0];
    sim.positionRight[1] = sensorRightOffset[1] + sim.position[1];

    sim.rawLeft = sim.positionLeft;
    sim.rawRight = sim.positionRight;
}

extern "C" int main(void)
{
    // Peripherals initialization

    StatusLED& LED = StatusLED::instance();
    Button resetButton(0);
    Button thumbButton(1);

    USBManager USB;
    USBPackage usbRaw = {0};
    USBPackage::Data& data = usbRaw.data;
    usbTimeout = 0;

    SPI.begin();
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(2);

    Sensor rightSensor({{15, 16}}, true, false); // Cyan
    Sensor leftSensor({{9, 8}}, false, true);    // Green

    leftSensor.reset();
    rightSensor.reset();

    LED.blink(defaultColor, 1.0);
    LED.on();

    // Thumb button state
    byte thumbButtonState = 0;

    // Debug variables
    vec2l lastSensorLeft = {{0, 0}};
    vec2l lastSensorRight = {{0, 0}};

    // Simulation initialization
    SimulationState sim;
    SimulationParameters parameters;

    // Read EEPROM data if available
    if(EEPROM.read(0) == 42) // State available
    {
        for(int n = 0; n < eepromSize(); n++)
        {
            eepData.raw[n] = EEPROM.read(n+1);
        }

        calibration = eepData.data.calibration;
        parameters = eepData.data.parameters;
        sim.inverseMass = 1.0f/parameters.mass;
        leftSensor.setCalibration(eepData.data.calibStateLeft);
        rightSensor.setCalibration(eepData.data.calibStateRight);
    }
    else // Default value
    {
        // TODO: These default calibration values do not reflect actual scenario
        calibration = {{0.1f, 0.1f}}; // [m]
        parameters.mass = 0.1f;
        parameters.stiffness = 2000.0f;
        parameters.damping = 30.0f;
        parameters.updateRate = 20;
        sim.inverseMass = 1.0f/parameters.mass;

        Sensor::CalibrationState defaultCalibration;
        defaultCalibration.alpha[0] = 0.0;
        defaultCalibration.alpha[1] = 1.0;
        defaultCalibration.scale[0] = 1.0;
        defaultCalibration.scale[1] = 1.0;
        leftSensor.setCalibration(defaultCalibration);
        rightSensor.setCalibration(defaultCalibration);
    }

    leftSensor.setCalibrationTarget(calibration);
    rightSensor.setCalibrationTarget(calibration);

    // Reset simulation
    resetSimulation(sim);
    leftSensor.reset();
    rightSensor.reset();
    State state = State::Init;

    // USB callbacks
    USB.onUSBError() = [&]()
    {
        while(!resetButton.query())
        {
            LED.blink(defaultColor, 0.1f);
            LED.blink(defaultColor, 0.1f);
            delay(500);
        }

        LED.blink(defaultColor, 2.0);
    };

    USB.onTimeOut() = [&]()
    {
        return;
        /*
        while(!resetButton.query())
        {
            LED.blink(defaultColor, 0.1);
            delay(500);
        }

        LED.blink(defaultColor, 2.0);
        */
    };

    USB.onLEDColor() = [&](color cl)
    {
        LED.on();
        LED.setColor(cl);
    };

    USB.onCalibrationData() = [&](vec2f calib)
    {
        LED.blink(defaultColor, 1.0f);
        LED.blink(defaultColor, 0.1f);
        delay(500);

        calibration = calib;
        leftSensor.setCalibrationTarget(calib);
        rightSensor.setCalibrationTarget(calib);
        resetSimulation(sim);
        leftSensor.reset();
        rightSensor.reset();

        state = State::Init;
    };

    USB.onParameters() = [&](SimulationParameters params)
    {
        LED.blink(defaultColor, 1.0f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        delay(500);

        parameters = params;
        resetSimulation(sim);
        leftSensor.reset();
        rightSensor.reset();
        sim.inverseMass = 1.0f/parameters.mass;
    };

    USB.onSaveConfiguration() = [&]()
    {
        LED.blink(defaultColor, 1.0f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        delay(500);

        eepData.data.calibration = calibration;
        eepData.data.parameters = parameters;
        eepData.data.calibStateLeft = leftSensor.calibration();
        eepData.data.calibStateRight = rightSensor.calibration();

        EEPROM.write(0, 42);

        for(int n = 0; n < eepromSize(); n++)
        {
            EEPROM.write(n + 1, eepData.raw[n]);
        }
    };

    USB.onCalibrate() = [&]()
    {
        LED.blink(defaultColor, 1.0f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        LED.blink(defaultColor, 0.1f);
        delay(500);

        LED.off();
        delay(1000);
        state = State::CalibrateX;
        leftSensor.calibrationStart();
        rightSensor.calibrationStart();
        LED.on();
    };


    while(true)
    {
        if(state == State::Init)
        {
            USB.checkIncoming();
            LED.setColor(defaultColor);

            if(resetButton.query())
            {
                LED.off();
                delay(1000);
                state = State::Simulate;
                resetSimulation(sim);
                leftSensor.reset();
                rightSensor.reset();
                LED.on();
            }
        }

        if(state == State::CalibrateX)
        {
            LED.on();
            LED.setColor({{255,0,0}});
            leftSensor.integrate();
            rightSensor.integrate();            

            if(thumbButton.query())
            {
                LED.off();                
                leftSensor.calibrationFinishX();
                rightSensor.calibrationFinishX();
                delay(1000);
                LED.on();
                while(!thumbButton.query())
                {
                    LED.setColor({{255,255,0}});
                }
                delay(1000);
                state = State::CalibrateY;
                LED.off();
                simTime = 0;
                leftSensor.reset();
                rightSensor.reset();
            }
        }

        if(state == State::CalibrateY)
        {
            leftSensor.integrate();
            rightSensor.integrate();

            LED.on();
            LED.setColor({{0,255,0}});

            if(thumbButton.query())
            {
                LED.off();                
                leftSensor.calibrationFinishY();
                rightSensor.calibrationFinishY();
                delay(1000);
                LED.blink(defaultColor, 0.25);
                LED.blink(defaultColor, 0.25);
                LED.blink(defaultColor, 0.25);

                state = State::Simulate;
                resetSimulation(sim);
                leftSensor.reset();
                rightSensor.reset();
                LED.off();
                simTime = 0;
            }
        }

        if(state == State::Simulate)
        {
            USB.checkIncoming();

            // Get movement since last frame in [m]
            vec2f deltaLeftRaw = leftSensor.integrate();
            vec2f deltaRightRaw = rightSensor.integrate();

            vec2f deltaLeft = mul(sim.rotation, deltaLeftRaw);
            vec2f deltaRight = mul(sim.rotation, deltaRightRaw);

            sim.rawLeft = add(sim.rawLeft, deltaLeft);
            sim.rawRight = add(sim.rawRight, deltaRight);

            // Projection onto rigid body
            vec2f rawCenter = mul(
                        add(sim.rawLeft, sim.rawRight),
                        0.5f);

            vec2f rawDelta = {{
                                  sim.rawLeft[0] - sim.rawRight[0],
                                  sim.rawLeft[1] - sim.rawRight[1]
                              }};

            float rawLengthInv = invLen(rawDelta);

            rawDelta = mul(rawDelta, rawLengthInv);


            sim.rawLeft = add(rawCenter,
                                   mul(rawDelta, sensorDistance*0.5f));

            sim.rawRight = sub(rawCenter,
                                   mul(rawDelta, sensorDistance*0.5f));

            // Calculate forces
            vec2f diffLeft  = mul(sub(sim.positionLeft, sim.rawLeft),
                                  parameters.stiffness);
            vec2f diffRight = mul(sub(sim.positionRight, sim.rawRight),
                                  parameters.stiffness);

            vec2f dampedVLeft = mul(sim.velocityLeft, parameters.damping);
            vec2f dampedVRight = mul(sim.velocityRight, parameters.damping);

            vec2f accelLeft = mul(add(diffLeft, dampedVLeft), sim.inverseMass);
            vec2f accelRight = mul(add(diffRight, dampedVRight), sim.inverseMass);

            // Velocity half-step
            vec2f velHalfLeft = sub(sim.velocityLeft, mul(accelLeft, 0.5 * sim.dt));
            vec2f velHalfRight = sub(sim.velocityRight, mul(accelRight, 0.5 * sim.dt));

            // Integrate positions
            sim.positionLeft = add(sim.positionLeft,
                                   mul(velHalfLeft,
                                       sim.dt));

            sim.positionRight = add(sim.positionRight,
                                   mul(velHalfRight,
                                       sim.dt));

            // Calculate forces for t + dt
            diffLeft  = mul(sub(sim.positionLeft, sim.rawLeft),
                                  parameters.stiffness);
            diffRight = mul(sub(sim.positionRight, sim.rawRight),
                                  parameters.stiffness);

            dampedVLeft = mul(velHalfLeft, parameters.damping);
            dampedVRight = mul(velHalfRight, parameters.damping);

            accelLeft = mul(add(diffLeft, dampedVLeft), sim.inverseMass);
            accelRight = mul(add(diffRight, dampedVRight), sim.inverseMass);

            // Integrate velocity
            sim.velocityLeft  = sub(velHalfLeft, mul(accelLeft, 0.5 * sim.dt));
            sim.velocityRight = sub(velHalfRight, mul(accelRight, 0.5 * sim.dt));


            // Calculate current rotation            
            vec2f posDelta = {{
                                  sim.positionLeft[0] - sim.positionRight[0],
                                  sim.positionLeft[1] - sim.positionRight[1]
                              }};

            float deltaLengthInv = invLen(posDelta);

            posDelta = mul(posDelta, deltaLengthInv);

            sim.position = mul(
                        add(sim.positionLeft, sim.positionRight),
                        0.5f);

            sim.angle = 0.75f * (atan2(posDelta[1], posDelta[0]) - staticAngle);

            float sinAlpha = sin(sim.angle);
            float cosAlpha = cos(sim.angle);

            sim.rotation = {{
                               cosAlpha, -sinAlpha,
                               sinAlpha,  cosAlpha
                            }};

            // Apply impulses (assume equal mass everywhere)
            sim.velocity = add(sim.velocityLeft, sim.velocityRight);
            sim.angularVelocity = cross({{0.0, sensorDistance*0.5f}}, sim.velocityLeft);
            sim.angularVelocity += cross({{0.0, -sensorDistance*0.5f}}, sim.velocityRight);


            if(usbTimeout > parameters.updateRate)
            {
              data.position = sim.position;
              data.velocity = sim.velocity;
              data.angle = sim.angle;
              data.angularVelocity = sim.angularVelocity;
              data.button = thumbButtonState;

              //data.leftSensor = leftSensor.absolutePosition();
              //data.rightSensor = rightSensor.absolutePosition();

              data.leftSensor = sim.positionLeft;
              data.rightSensor = sim.positionRight;

              data.leftIncrement[0] = int16_t(leftSensor.rawPosition()[0]
                                              - lastSensorLeft[0]);
              data.leftIncrement[1] = int16_t(leftSensor.rawPosition()[1]
                                              - lastSensorLeft[1]);
              data.rightIncrement[0] = int16_t(rightSensor.rawPosition()[0]
                                               - lastSensorRight[0]);
              data.rightIncrement[1] = int16_t(rightSensor.rawPosition()[1]
                                               - lastSensorRight[1]);

              lastSensorLeft = leftSensor.rawPosition();
              lastSensorRight = rightSensor.rawPosition();

              data.time = sim.time;
              data.timeStep = sim.dt;

              USB.sendFrame(usbRaw);
              usbTimeout = 0;
              thumbButtonState = 0;
            }

            if(thumbButton.query() && resetButton.query())
            {
                // Force calibration
                LED.on();
                LED.blink(defaultColor, 1.0f);
                LED.blink(defaultColor, 0.1f);
                LED.blink(defaultColor, 0.1f);
                LED.blink(defaultColor, 0.1f);
                LED.blink(defaultColor, 0.1f);
                LED.blink(defaultColor, 0.1f);
                state = State::CalibrateX;
                leftSensor.calibrationStart();
                rightSensor.calibrationStart();
            }
            else if(resetButton.query())
            {
                LED.off();
                delay(1000);
                resetSimulation(sim);
                leftSensor.reset();
                rightSensor.reset();
                LED.on();
            }
            else if(thumbButton.query())
            {
                thumbButtonState = 1;
            }

            if(leftSensor.isLifted() || rightSensor.isLifted())
            {
                resetRotation(sim);
            }

            sim.dt = float(simTime) * 1.0e-6f;
            simTime = 0;
        }
    }
}

