#include "sensor.h"

#include <WProgram.h>
#include <avr/pgmspace.h>
#include "../firmware/adns9800_srom_A6.h"
#include "statusled.h"
#include <SPI.h>

// Registers
#define REG_Product_ID                           0x00
#define REG_Revision_ID                          0x01
#define REG_Motion                               0x02
#define REG_Delta_X_L                            0x03
#define REG_Delta_X_H                            0x04
#define REG_Delta_Y_L                            0x05
#define REG_Delta_Y_H                            0x06
#define REG_SQUAL                                0x07
#define REG_Pixel_Sum                            0x08
#define REG_Maximum_Pixel                        0x09
#define REG_Minimum_Pixel                        0x0a
#define REG_Shutter_Lower                        0x0b
#define REG_Shutter_Upper                        0x0c
#define REG_Frame_Period_Lower                   0x0d
#define REG_Frame_Period_Upper                   0x0e
#define REG_Configuration_I                      0x0f
#define REG_Configuration_II                     0x10
#define REG_Frame_Capture                        0x12
#define REG_SROM_Enable                          0x13
#define REG_Run_Downshift                        0x14
#define REG_Rest1_Rate                           0x15
#define REG_Rest1_Downshift                      0x16
#define REG_Rest2_Rate                           0x17
#define REG_Rest2_Downshift                      0x18
#define REG_Rest3_Rate                           0x19
#define REG_Frame_Period_Max_Bound_Lower         0x1a
#define REG_Frame_Period_Max_Bound_Upper         0x1b
#define REG_Frame_Period_Min_Bound_Lower         0x1c
#define REG_Frame_Period_Min_Bound_Upper         0x1d
#define REG_Shutter_Max_Bound_Lower              0x1e
#define REG_Shutter_Max_Bound_Upper              0x1f
#define REG_LASER_CTRL0                          0x20
#define REG_Observation                          0x24
#define REG_Data_Out_Lower                       0x25
#define REG_Data_Out_Upper                       0x26
#define REG_SROM_ID                              0x2a
#define REG_Lift_Detection_Thr                   0x2e
#define REG_Configuration_V                      0x2f
#define REG_Configuration_IV                     0x39
#define REG_Power_Up_Reset                       0x3a
#define REG_Shutdown                             0x3b
#define REG_Inverse_Product_ID                   0x3f
#define REG_Motion_Burst                         0x50
#define REG_SROM_Load_Burst                      0x62
#define REG_Pixel_Burst                          0x64

Sensor::Sensor(std::array<unsigned char, 2> pins, bool flipX, bool flipY)
{
    this->flipX = flipX;
    this->flipY = flipY;
    integrated = {{0,0}};
    this->pins = pins;
    pinMode (pins[0], OUTPUT); // Chip select
    pinMode (pins[1], INPUT_PULLUP); // Motion pin
    //attachInterrupt(mot, sensorChanged, FALLING);

    // TODO: which parts of the code have to be executed once / foreach sensor ?
    adns_com_end(); // ensure that the serial port is reset
    adns_com_begin(); // ensure that the serial port is reset
    adns_com_end(); // ensure that the serial port is reset
    adns_write_reg(REG_Power_Up_Reset, 0x5a); // force reset
    delay(50); // wait for it to reboot
    // read registers 0x02 to 0x06 (and discard the data)
    adns_read_reg(REG_Motion);
    adns_read_reg(REG_Delta_X_L);
    adns_read_reg(REG_Delta_X_H);
    adns_read_reg(REG_Delta_Y_L);
    adns_read_reg(REG_Delta_Y_H);
    // upload the firmware
    uploadFirmware();

    delay(10);
    //enable laser(bit 0 = 0b), in normal mode (bits 3,2,1 = 000b)
    // reading the actual value of the register is important because the real
    // default value is different from what is said in the datasheet, and if you
    // change the reserved bytes (like by writing 0x00...) it would not work.
    byte laser_ctrl0 = adns_read_reg(REG_LASER_CTRL0);
    adns_write_reg(REG_LASER_CTRL0, laser_ctrl0 & 0xf0 );
    //adns_write_reg(REG_Configuration_I, 0x01 ); // 200 dpi
    //adns_write_reg(REG_Configuration_I, 0x09 ); // 1800 dpi
    //adns_write_reg(REG_Configuration_I, 0x29 ); // 8200 dpi
    adns_write_reg(REG_Configuration_I, 0xA4 ); // 8200 dpi

    delay(1);
}

void Sensor::setCalibrationTarget(vec2f target)
{
    calibrationTarget = target;
}

void Sensor::adns_com_begin()
{
    digitalWrite(pins[0], LOW);
}

void Sensor::adns_com_end()
{
    digitalWrite(pins[0], HIGH);
}

byte Sensor::adns_read_reg(byte reg_addr)
{
    adns_com_begin();    
    SPI.transfer(reg_addr & 0x7f );
    delayMicroseconds(100);
    byte data = SPI.transfer(0);
    delayMicroseconds(1);
    adns_com_end();
    delayMicroseconds(19);
    return data;
}

void Sensor::adns_write_reg(byte reg_addr, byte data)
{
    adns_com_begin();    
    SPI.transfer(reg_addr | 0x80 );
    SPI.transfer(data);
    delayMicroseconds(20);
    adns_com_end();
    delayMicroseconds(100);
}

void Sensor::uploadFirmware()
{    
    adns_write_reg(REG_Configuration_IV, 0x02);
    adns_write_reg(REG_SROM_Enable, 0x1d);
    delay(10);
    adns_write_reg(REG_SROM_Enable, 0x18);
    adns_com_begin();
    SPI.transfer(REG_SROM_Load_Burst | 0x80);
    delayMicroseconds(15);
    unsigned char c;
    for (int i = 0; i < firmware_length; i++) {
      c = (unsigned char)pgm_read_byte(firmware_data + i);
      SPI.transfer(c);
      delayMicroseconds(15);
    }
    adns_com_end();
}

vec2f Sensor::integrate()
{
    vec2f delta = {{0.0f, 0.0f}};
    short deltaX = 0;
    short deltaY = 0;

    byte motion = adns_read_reg(REG_Motion);

    // semantic see datasheet
    bool motionOccured = motion & 0x80;
    bool motionFault = motion & 0x40;
    bool laserValid = motion & 0x20;

    if(motionOccured && !motionFault && laserValid)
    {
        deltaX = (short) adns_read_reg(REG_Delta_X_L) |
                    ((short) adns_read_reg(REG_Delta_X_H) << 8);
        deltaY = (short) adns_read_reg(REG_Delta_Y_L) |
                    ((short) adns_read_reg(REG_Delta_Y_H) << 8);
    }

    if(!laserValid)
    {
        StatusLED::instance().blink({{255,0,0}}, 1.0);
        StatusLED::instance().blink({{255,128,0}}, 0.5);
    }

    if(motionFault)
    {
        StatusLED::instance().blink({{255,0,0}}, 1.0);
        StatusLED::instance().blink({{255,128,0}}, 0.5);
        StatusLED::instance().blink({{255,128,0}}, 0.5);
    }

    if(flipX)
        deltaX *= -1;
    if(flipY)
        deltaY *= -1;

    integrated[0] += deltaX;
    integrated[1] += deltaY;

    delta[0] = deltaX;
    delta[1] = deltaY;

    delta[0] *= calib.scale[0];
    delta[1] *= calib.scale[1];

    delta = mul(correction, delta);
    return delta;
}

vec2f Sensor::absolutePosition()
{
    vec2f pos = {{
                     integrated[0] * calib.scale[0],
                     integrated[1] * calib.scale[1]
                 }};    
    //return pos;
    return mul(correction, pos);
}

void Sensor::reset()
{
    integrated[0] = 0;
    integrated[1] = 0;

    // Delete previous sensor data
    adns_read_reg(REG_Motion);
    adns_read_reg(REG_Delta_X_L);
    adns_read_reg(REG_Delta_X_H);
    adns_read_reg(REG_Delta_Y_L);
    adns_read_reg(REG_Delta_Y_H);

    //adns_write_reg(REG_Motion, 0);
}

void Sensor::calibrationStart()
{
    StatusLED::instance().blink({255,64,0}, 0.5);
    reset();
}

Sensor::CalibrationState Sensor::calibration()
{
    return calib;
}

void Sensor::setCalibration(const Sensor::CalibrationState& cal)
{
    calib = cal;

    correctionAlpha = acos(calib.alpha[0]);

    correction = {{
                     calib.alpha[0], calib.alpha[1],
                    -calib.alpha[1],  calib.alpha[0]
                  }};

    reset();
}

void Sensor::calibrationFinishX()
{    

    vec2f target = {{1.0,0.0}};
    vec2f measured = normalized({{
                              float(integrated[0]),
                              float(integrated[1])
                          }});

    calib.alpha[0] = dot(target, measured);
    calib.alpha[1] = cross(target, measured);

    correctionAlpha = acos(calib.alpha[0]);

    correction = {{
                     calib.alpha[0], calib.alpha[1],
                    -calib.alpha[1],  calib.alpha[0]
                  }};

    float dxLen = correction[0] * calibrationTarget[0]/len(integrated);

    //float dxLen = calibrationTarget[0]/integrated[0];
    calib.scale[0] = dxLen;

    reset();
}


void Sensor::calibrationFinishY()
{
    float dyLen = correction[0] * calibrationTarget[1]/len(integrated);
    //float dyLen = calibrationTarget[1]/integrated[1];
    calib.scale[1] = dyLen;

    reset();
}

float Sensor::correctionAngle() const
{
    return correctionAlpha;
}

vec2l Sensor::rawPosition()
{
    return integrated;
}
