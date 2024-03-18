#include <Arduino.h>

// WARNING!! DO NOT USE Tx AND Rx PINS ON YOUR BOARD AS THEY'RE NEEDED BY THE SERIAL COMMUNICATION

#define SHIFTR_CLK_PIN 11  //SH_CP
#define SHIFTR_LATCH_PIN 12  //ST_CP    // <---- all can be changed
#define SHIFTR_DATA_PIN 10  //DS

#define OUTPUT_ENABLEB 13 
#define WRITE_ENABLEB A0

int Data_Pins[]={9,8,7,6,5,4,3,2}; // <-- need to be changed based on your board. Least significant bit first
int Data_Pins_mode=2;


byte flipByte(byte c){
  c = ((c>>1)&0x55)|((c<<1)&0xAA);
  c = ((c>>2)&0x33)|((c<<2)&0xCC);
  c = (c>>4) | (c<<4) ;

  return c;
}


void my_shiftOut(byte Data){
  Data=flipByte(Data);
  for(int i=0; i<8; i++){
    PORTB = PORTB | ((Data&1)<<2); //set sr data pin
    PORTB = PORTB | B00001000; //set clock pin high
    PORTB = PORTB & B11110111; //set clock pin low
    PORTB = PORTB & B11111011; //reset sr data pin
    Data=(Data>>1);
  }
}



//sets all the data pins to Input or Output
void set_data_pins_IO(int mode){
  if(Data_Pins_mode!=mode){
    for(int i=0; i<8; i++){
      pinMode(Data_Pins[i], mode);
    }
    delay(10);
    Data_Pins_mode=mode;
  }
}

//given high and low bytes of the address sets the shift registers
void set_address_from_byte(byte HighByte, byte LowByte){
  PORTB = PORTB & B11101111; //set latch 0
  my_shiftOut(HighByte);
  my_shiftOut(LowByte);
  PORTB = PORTB | B00010000;//set latch 1
}



//sets all the board data pins given a byte and writes to the eeprom
void write_EEPROM(byte DataByte){
  digitalWrite(OUTPUT_ENABLEB, HIGH);
  set_data_pins_IO(OUTPUT);
  DataByte=flipByte(DataByte);
  PORTB=PORTB | DataByte/64;
  PORTD=PORTD | (DataByte%64)<<2;
  digitalWrite(WRITE_ENABLEB, LOW);
  digitalWrite(WRITE_ENABLEB, HIGH);
  delayMicroseconds(50);
  PORTB=PORTB & B11111100;
  PORTD=PORTD & B00000011;
}

//reads from the datapins and returns the byte red
byte read_EEPROM(){
  digitalWrite(OUTPUT_ENABLEB, LOW);
  set_data_pins_IO(INPUT);
  byte Data_byte=0;
  for(int i=7; i>=0; i--){
    Data_byte= (Data_byte<<1) + digitalRead(Data_Pins[i]);
  }
  return Data_byte;
}

//sets it's working mode to read or write
void start_execution(int mode){
  int high=0, low=0, stop=0;
  byte DataB[64];
  unsigned long time_check=millis();


  switch (mode){
  case 48://read
    while (high<128){
      //for reading it increments the address and sends the byte stored to python
      set_address_from_byte(high, low);
      Serial.write(read_EEPROM());
      low++;
      if(low==256){
        low=0;
        high++;
      }
    }
    break;
  case 49://write
    digitalWrite(OUTPUT_ENABLEB, HIGH);
    set_data_pins_IO(OUTPUT);
    while(!stop){
      if(Serial.available()>=32){
        //reads an address and a byte from python and proceeds to write on the eeprom
        time_check=millis();
        for(int i=0; i<32; i++){
          set_address_from_byte(high, low);
          write_EEPROM(Serial.read());
          low++;
          if(low==256){
            high++;
            low=0;
          }
        }
        delay(5);

        //to not lose data due to python beeing too fast, python halts until arduino sends one byte over serial
        //can be every value
        Serial.write((int)DataB[0]);
        
      } 
      else if (millis()-time_check > 4000){
        stop=1;
      }
    }
    break;
  default:
    break;
  }
}

void setup() {
  //set all pins to output and write enable to high
  pinMode(SHIFTR_CLK_PIN, OUTPUT);
  pinMode(SHIFTR_LATCH_PIN, OUTPUT);
  pinMode(SHIFTR_DATA_PIN, OUTPUT);
  pinMode(WRITE_ENABLEB, OUTPUT);
  pinMode(OUTPUT_ENABLEB, OUTPUT);
  digitalWrite(WRITE_ENABLEB, HIGH);
  pinMode(OUTPUT_ENABLEB, HIGH);
  Serial.begin(115200);
  Serial.setTimeout(1500);
  set_address_from_byte(0x00, 0x00);
}

void loop() {
  if(Serial.available()>0)
    //interfaces to the python program that sends a byte to set the working mode
    start_execution(Serial.read());
}