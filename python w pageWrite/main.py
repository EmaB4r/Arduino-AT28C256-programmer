import time
import serial
import serial.tools.list_ports
import serial.tools.list_ports_linux

#if delay is 1 it delays about 1 second
def delay_a_bit(delay):
    for i in range(delay*70000000):
        continue

#function to automatically detect the EEPROM programmer between all the COMs
def auto_detect_eeprom_programmer():
    print("don't connect the arduino EEPROM programmer yet\npress enter to continue", end="")
    input()

    #it creates two sets to easily find the differences
    set_before_connect = set()
    set_after_connect = set()
    cnt=15 #counter used to retry to find a new COM device

    #for loop that reads all the COMs connected and adds them to the first set
    for comport in serial.tools.list_ports.comports():
        set_before_connect.add(comport.device)
    set_after_connect=set_before_connect
    print("now connect the arduino EEPROM programmer\npress enter to continue", end="")
    input()


    while(set_after_connect==set_before_connect and cnt!=0):
        print(".", end="") #to show that is doing something
        set_after_connect=set()

        #read all the COMs after a new device has been connected
        for comport in serial.tools.list_ports.comports():
            set_after_connect.add(comport.device)
        cnt=cnt-1
        delay_a_bit(1) #slows the loop a bit


    if cnt==0:
        print("couldn't find your device\n")
        exit(1)

    #this operation should give a set with only one element corresponding to the EEPROM programmer
    EEPROMset = set_after_connect-set_before_connect

    if len(EEPROMset)>1:
        print("an error occurred during the device detection\ntry again")
        input()
        exit(3)

    print("found")
    EEPROM = EEPROMset.pop()
    return EEPROM
    

#function that gets from the user the path to the .bin file and returns it
def get_file_name():
    print("copy here your .bin file path")
    file_name=input().rstrip()
    return file_name

#function that sends a whole .bin file through serial communication
def send_file_via_serial(file, serial_port, start_address):
    line_cnt=0
    address_cnt=0
    sent_bytes=0
    while byte:=file.read(32):
        print(hex(address_cnt))
        serial_port.write(byte) #sends the data
        serial_port.flush()
        serial_port.read() # waits for the arduino
        address_cnt+=32
            



def main():

    #get the programmer's COM
    com_device=auto_detect_eeprom_programmer()

    print("opening serial communication.", end="")
    #wait about 3 seconds to let the arduino open it's Serial
    for i in range (3):
        delay_a_bit(1)
        print(".", end="")
    print("done")

    #open the serial communication
    serial_port=serial.Serial(com_device, 115200, serial.EIGHTBITS, timeout=6)
    while(1):
        print("want to read the EEPROM or write to it? R/W (T to terminate): ")
        readWrite=input()

        if readWrite=='W' or readWrite=='w':
            # get and open the .bin file
            file_name = get_file_name()
            try:
                file = open(file_name, "rb")
            except Exception as msg:
                print(msg)
                exit(2)

            print("set start address (int): ")
            start_address = int(input())

            a=49 #have to convert it to byte or it won't work, this value is needed to pair the mode with the arduino
            serial_port.write(a.to_bytes(1, byteorder="big"))
            serial_port.flush()

            # sends the whole file
            send_file_via_serial(file, serial_port, start_address)
            file.close()
        
        if readWrite=='R' or readWrite=='r':
            a=48
            adress=0
            serial_port.write(a.to_bytes(1, byteorder="big"))
            serial_port.flush()
            while 1:
                #leaves full control to arduino, because python is faster
                if (adress%16==0):
                    print()
                    print(hex(adress), end="   ")
                time_check=time.time()
                print(hex(int.from_bytes(serial_port.read(), byteorder="big")), end=" ")
                time_check=time.time()-time_check
                if(time_check > 2):
                    print("\nEND READING")
                    break
                adress+=1

        if readWrite=='T' or readWrite=='t':
            break


    serial_port.close()



main()