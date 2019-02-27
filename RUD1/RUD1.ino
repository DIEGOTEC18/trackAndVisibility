/*
  Reading multiple RFID tags, simultaneously!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 3rd, 2016
  https://github.com/sparkfun/Simultaneous_RFID_Tag_Reader

  Read the user writeable data from a detected tag
*/

#include <SoftwareSerial.h> //Used for transmitting to the device

SoftwareSerial softSerial(2, 3); //RX, TX
SoftwareSerial BTserial(10, 11); // RX | TX

#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module
RFID nano; //Create instance

void setup()
{
  Serial.begin(115200);
  BTserial.begin(115200);

  while (!Serial);
  Serial.println();
  BTserial.println();
  Serial.println("Initializing...");
  BTserial.println("Initializing...");

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println("Module failed to respond. Please check wiring.");
    BTserial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }

  nano.setRegion(REGION_OPEN); //Set to North America

  nano.setReadPower(2700); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling
}

void loop()
{
  /*//LOOP IT HERE:
  Serial.println(F("Press a key to read user data"));
  while (!Serial.available()); //Wait for user to send a character
  Serial.read(); //Throw away the user's character*/

  //Read the data from the tag
  byte responseType;
  byte responseTypeE;
  byte myData[64];
  byte myDataLength = sizeof(myData); //Tell readUserData to read up to 64 bytes
  byte myEPC[12]; //Most EPCs are 12 bytes
  byte myEPClength;

  myEPClength = sizeof(myEPC);

  responseTypeE = nano.readTagEPC(myEPC, myEPClength, 500); //Scan for a new tag up to 500ms
  
  responseType = nano.readUserData(myData, myDataLength); //readUserData will modify myDataLength to the actual # of bytes read

  if (responseType == RESPONSE_SUCCESS)
  {
    //Print User Data
    Serial.print(F("Size ["));
    BTserial.print(F("Size ["));
    Serial.print(myDataLength);
    BTserial.print(myDataLength);
    Serial.print(F("] User data["));
    BTserial.print(F("] User data["));
    for (byte x = 0 ; x < myDataLength ; x++)
    {
      if (myData[x] < 0x10) Serial.print(F("0")); //BTserial.print(F("0"));
      Serial.print(myData[x], HEX);
      BTserial.print(myData[x], HEX);
      Serial.print(F(" "));
      BTserial.print(F(" "));
      
    }
    Serial.println(F("]"));
    BTserial.println(F("]"));

    String product = String((char *)myData);
    Serial.println(product);
    BTserial.println(product);
  }
  else
    Serial.println(F("Error reading tag data"));
    //BTserial.println(F("Error reading tag data"));

      //Print EPC
  Serial.print(F(" epc["));
  BTserial.print(F(" epc["));
  
  for (byte x = 0 ; x < myEPClength ; x++)
  {
    if (myEPC[x] < 0x10) Serial.print(F("0")); //BTserial.print(F("0"));
    Serial.print(myEPC[x], HEX);
    BTserial.print(myEPC[x], HEX);
    Serial.print(F(" "));
    BTserial.print(F(" "));
    
  }
  Serial.println(F("]"));
  BTserial.println(F("]"));
  
   String productE = String((char *)myEPC);
   Serial.println(productE);
   BTserial.println(productE);

}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (!softSerial); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));
    BTserial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}

