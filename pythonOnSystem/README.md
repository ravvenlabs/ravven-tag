# onSystem Files

Directory contains on system files for the Fusion 2 board by Craft Cameras.

## How to Run

### Go to file directory (must be superuser)

Superuser will be necessary for later steps.

```
sudo -i
xilinx
cd <onSystem fusion2 directory>
ifconfig
```

Write down the IP address of the wlan0. Typically something like ```192.168.1.14```.

### Load the correct bit file

Bit files can be found in `bin/bits/`. Copy the desired bit file to
`/boot/system.bit` to load the file like below.

```cp bin/bits/kirsch.bit /boot/system.bit```

Reboot to flash the bit file onto the FPGA.

```reboot```

### Initialize the system

Run the following command to configure the system for usage.

```./bin/initSystem```

Make sure the binary is executable with ```chmod +x ./bin/initSystem```.

### Run the desired server

Run the desired server which will use the ip address of the device on port 8081.

```python feedthroughServer.py```

## Bit Files

All bit files configured for this system can be found in `bin/bits/`. These are
loaded onto the system by copying the desired bit file to `/boot/system.bit` and
reboooting. More details on how these bit files are configured and their AXI
interface can be found in their respective _axi.txt files. The bit files are their
usage are described below:

- kirsch.bit: Outputs all intermediate output images from the hardware regarding
the Ravven Tag algorithm.
- hwTarget.bit: Outputs the segment data from the Ravven Tag algorithm.
