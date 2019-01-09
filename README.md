# mod-duox-controller

Files for the Duo X controller firmware

## Dependencies

A ARM compiler. E.g. on Arch Linux install

```
> sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib
```

## Build

```
> make modduos
```

The output is `out/mod-duox-controller.bin` which you can copy to a
Duo S like this:

```
> scp out/mod-duox-controller.bin root@modduo.local:/tmp/
```

SSH into the device and run:

```
> hmi-update /tmp/mod-duox-controller.bin
> systemctl restart mod-ui
```
