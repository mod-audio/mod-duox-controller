# mod-duox-controller

Files for the Duo X controller firmware

Not ready for a stable release yet, though good for testing.

Known bugs:
-Banks menu can crash the HMI because of an indexing issue 
-The control_set callback is not implemented (also not on the MOD-UI side)

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