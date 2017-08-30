#ifndef SENSOR_H
#define SENSOR_H

#include "types.h"

class Sensor
{
public:
    Sensor(std::array<unsigned char, 2> pins, bool flipX, bool flipY);

    struct CalibrationState
    {
        vec2f alpha;
        vec2f scale;
    };

    void reset();
    vec2f integrate();
    vec2f absolutePosition();

    void calibrationStart();
    void calibrationFinishX();
    void calibrationFinishY();

    vec2l rawPosition();
    void setCalibrationTarget(vec2f target);
    float correctionAngle() const;

    CalibrationState calibration();
    void setCalibration(const CalibrationState& cal);

private:
    void adns_com_begin();
    void adns_com_end();
    byte adns_read_reg(byte reg_addr);
    void adns_write_reg(byte reg_addr, byte data);


    void uploadFirmware();

    std::array<unsigned char, 2> pins;

    CalibrationState calib;

    mat2f correction;
    float correctionAlpha;

    vec2l integrated;

    vec2f calibrationTarget;
    bool flipX;
    bool flipY;

    enum class SensorState
    {
        Idle,
        CalibrateWait,
        CalibrateRun,
        Integrate
    } state;

};

#endif // SENSOR_H
