# mod-duox-controller

Files for the Duo X controller firmware

Not ready for a stable release yet, though good for testing.

This repo will only work with the latest revision of the MOD DuoX controller board (Rev 5, March 2019).
The board features a 12Mhz crystal (main CLK of 120Mhz) on the LPC1777.

In order to flash the HMI the LPC2ISP tool is used: https://github.com/moddevices/lpc21isp.git

In order to flash the LPC1777 the update script hmi-update in /bin/ needs the correct value's 
the baudrate at which the data is send needs to be 57600bps instead of 115200bps for the MOD Duo
this is because the LPC1777 doesnt accept an ISP baud rate of 115200bps
The clock frequency also needs to be adjusted to 12Mhz instead of 10Mhz. 

## Build dependencies

A ARM compiler. E.g. on Arch Linux install:
```
pacman -S arm-none-eabi-gcc
```
Or on Debian Linux install:
```
add-apt-repository ppa:team-gcc-arm-embedded/ppa
apt-get install gcc-arm-embedded
```
