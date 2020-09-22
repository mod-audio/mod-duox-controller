#!/bin/bash

make clean

make modduox

sshpass -p "mod" scp ./out/mod-duox-controller.bin root@192.168.51.1:/tmp/

sshpass -p 'mod' ssh root@192.168.51.1 hmi-update /tmp/mod-duox-controller.bin

sshpass -p 'mod' ssh root@192.168.51.1 systemctl restart mod-ui
