# mod-duox-controller

Files for the Duo X controller firmware

Not ready for a stable release yet, though good for testing.

This branch will only work with the latest revision of the MOD DuoX controller board (Rev 5, March 2019).
The board features a 12Mhz crystal (main CLK of 120Mhz) on the LPC1777.

In order to flash the LPC1777 the update script hmi-update in /bin/ needs a change. 
the baudrate at which the data is send needs to be 57600bps instead of 115200bps (the last line of the script)
this is because the LPC1777 doesnt accept an ISP baud rate of 115200bps

# Known bugs:

## Things to sort out between MOD-UI and the HMI

- Add pedalboard name refresh callback so the HMI knows when to update
- When an actuator is assigned, and of the same plugin the preset list is assigned on the encoder, when the preset is changed   the value of the actuator needs to change as well. This should happen with the 'control_set <effect_instance> <symbol>  <value>' command. Right now all the actuator are reassigned, which causes the HMI to crash. 
- the get_out_chan_link command crashes the HMI for some reason.  
- there is no Tap tempo unasignment, also the tempo is not changeable, some funky stuf here, we need to have a look at all the adressings from global tempo! 

Things to check:
- There seems to be some kind of bug whith the pedalboard name. the second word is not being displayed for some reason, not     sure if the HMI or MOD-UI is to blame here, we need to check the message.
- The send_load_pedalboard command does not work, it also crashes the HMI at some point, we need to check the message.

## missing features:
- Pages callback
- Instant snapshots
- Gain stages control combined with PGA gain control

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
