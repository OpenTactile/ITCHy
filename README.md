# ITCHy
The *Interactive Tactile display Control Handle*

ITCHy is a combined input/output device similar to a conventional computer mouse. It provides accurate tracking of absolute position, orientation as well as translational and angular velocities - thanks to its *two* ADNS9800 laser sensors. Using a total of 20 vibrating actuators, various tactile impressions can be transmitted to ones fingertip. Additional buttons and a RGB LED allows for basic user interaction.

ITCHy is intended to be used in conjunction with the [SCRATCHy system](https://github.com/OpenTactile/SCRATCHy).<br>
However, it can also be used in a stand-alone fashion.

### Parts needed for building the tactile mouse

Positional und rotational tracking is done using two *ADSN9800 Motion Sensor* boards, both connected to a *PJRC Teensy 3.2* USB Development Board via [SPI](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus). By adding some buttons and a RGB LED (optional), additional user interaction is possible.

  * Teensy 3.2 USB Development Board<br>
    see, e.g. https://www.pjrc.com/store/teensy32.html    
  * Two ADNS 9800 Laser Motion Sensors<br>
    https://www.tindie.com/products/jkicklighter/adns-9800-laser-motion-sensor/
  * Flexible USB-A to Micro-USB cable of appropriate length  
  * Microswitch (as used in computer mice, e.g. [this one](http://www.newark.com/omron-electronic-components/d2f/microswitch-pin-plunger-spdt-3a/dp/36K7257)) for the thumb button
  * Tactile switch for the reset button (e.g. [this one](http://www.newark.com/alps/skhhaqa010/switch-tactile-6x6mm-vert-red/dp/94T3234))
  * (optional) RGB LED or 3 individual LEDs
  * (optional) Appropriate resistors for operating the LEDs
  
3D printable parts are located in the *tactileMouse* subfolder. For convenience, single parts have been exported to *stl* Format that should be appropriate for most manufacturers. If modifications are needed, please use [Blender](https://www.blender.org/) for editing the *tactileMouse.blend* file.

We have positive experiences with [Shapeways](https://www.shapeways.com/) for manufacturing the parts using the following materials:

  * mouse_base.stl &rarr; [Strong & Flexible Plastic](https://www.shapeways.com/materials/strong-and-flexible-plastic), Black
  * mouse_buttons.stl &rarr; [Strong & Flexible Plastic](https://www.shapeways.com/materials/strong-and-flexible-plastic), Orange Polished
  * mouse_cover.stl &rarr; [Strong & Flexible Plastic](https://www.shapeways.com/materials/strong-and-flexible-plastic), White Polished
  * mouse_diffusor.stl &rarr; [Acrylic Plastic](https://www.shapeways.com/materials/acrylic-plastic), Transparent Acrylic (Transparent Detail)

***Important: Always check dimensions of individual parts before printing!***
  
    

