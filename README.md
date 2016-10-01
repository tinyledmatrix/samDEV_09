# samDEV_09
small test/development board for Atmels's SAMD09 ARM Cortex M0+ Microcontroller

see my [poject](https://hackaday.io/project/12099) on Hackaday.io for more information. 

I build this board to getting started with the SAMD microcontroller family after I made the mistake to try the much bigger SAMD21 first. The SAMD09 used here is he smallest from this family and therefore has the shortest (still 709 p.) datasheet. I think this board will be, as my other microcontroller breakout boards, very useful for an early prototyping phase on breadboard. So far I quite like the SAMD09. If you compare it to the the ATmegas there are a lot more possibilities. Maybe this is the Arduino Nano killer for me.
Features

    3.3V 0.8A LDO (1117-3.3)
    powered from USB, Vin pin (3.6-13.8V) or regulated 3.3V
    on board USB to serial bride IC (CH340)
    reset switch
    SWD programming header
    optional 32kHz crystal (if not needed pins can be used as additional IOs)
    on board user LED (connected to PA02)
    on board power LED (connected to 3.3 rail)

Schematic

Partlist:

Part   Value        Package     Comment
                    
A08                 0603        User LED connected to PA08
C1     100n         C0603       
C2     15p          C0603       optional 
C3     15p          C0603       optional 
C4     10n          C0603       
C5     100n         C0603       
C7     100n         C0603       
D1     MBR0520LT    SOD123      power selection diode
D2     MBR0520LT    SOD123      power selection diode
IC1    CH340G       SOIC16      USB-UART bridge
IC2    1117-3.3     SOT223      LDO
JP1                 1X08_OCTA   pin header
JP2                 1X08_OCTA   pin header
JP5                 2X05_1,27B  programming pin header 
K1     mini USB     generic     Mini USB connector
PWR                 0603 	Power LED
Q1     3215         3215        optional external 32kHz crystal 
Q2     CSTCRCSTCE   CSTCESMALL  12 MHz resonator for CH340G
R1     solder / 0   R0201       PA08 - pin header
R2     solder / 0   R0201       PA09 - pin header
R3     120  / 220   R0603       resistor for power LED
R4     120  / 220   R0603       resistor for user LED
R5     ~10k         R0603       pullup for reset
S2                  SMDSWITCH   Chinese SMD switch
U$1    SAMD09SOIC14 SO14        ARM Cortex M0+ microcontroller
