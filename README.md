# mod-duox-controller

Files for the Duo X controller firmware

Not ready for a stable release yet, though good for testing.

This branch will only work with the latest revision of the MOD DuoX controller board (Rev 5, March 2019).
The board features a 12Mhz crystal (main CLK of 120Mhz) on the LPC1777.

In order to flash the LPC1777 the update script hmi-update in /bin/ needs a change. 
the baudrate at which the data is send needs to be 57600bps instead of 115200bps (the last line of the script)
this is because the LPC1777 doesnt accept an ISP baud rate of 115200bps

Known bugs:
- Banks menu can crash the HMI because of an indexing issue 
- The control_set callback is not implemented (also not on the MOD-UI side)

missing features:
- Pages callback
- Instant snapshots
- Gain stages control combined with PGA gain control

## Build dependencies

A ARM compiler. E.g. on Arch Linux install
```
pacman -S arm-none-eabi-gcc
```
A  compiler. E.g. on Debian Linux install
```
add-apt-repository ppa:team-gcc-arm-embedded/ppa
apt-get install gcc-arm-embedded
```