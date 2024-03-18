# Arduino-AT28C256-programmer  
Project that let's you **read from and write to** an **AT28C256 EEPROM**.  
Got the idea from [Ben Eater](https://youtu.be/K88pgWhEb1M?si=ZITRxYaQYVkPP_2X) but remade the arduino code from scratch and made myself the python interface.  
**TESTED ON BOTH LINUX AND WINDOWS**

If on Linux (Ubuntu in my case) and using a clone arduino with a CH34x interface you need to run these commands in the terminal:  
`systemctl stop brltty-udev.service`  
`sudo systemctl mask brltty-udev.service`  
`systemctl stop brltty.service`  
`systemctl disable brltty.service`  
They stop your OS from seeing the arduino nano as a braille e-reader  

## Next updates will add the pagewrite to speed up writing, dumping the rom content to a .bin and add arguments support

# Python code:
`pyserial` needs to be installed by running `pip install pyserial` on your terminal.  
The code's able to automatically detect the arduino-programmer, send data to it for writing and receive for reading.  
For now the read function only prints to terminal. In some days I'll probably add the possibility to write to a .bin file.  
 Automatically detects when done reading/writing and restarts.

# Arduino code:
Code written using PlatformIO; if using arduino IDE the `#include <Arduino.h>` *probably* needs to be removed.  
The code makes the programmer talk to python via Serial communication. It's able to write, given an address and the data to write, and to read (the whole rom).
