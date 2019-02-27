/*
  Reading multiple RFID tags, simultaneously!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 3rd, 2016
  https://github.com/sparkfun/Simultaneous_RFID_Tag_Reader

  Constantly reads and outputs any tags heard

  If using the Simultaneous RFID Tag Reader (SRTR) shield, make sure the serial slide
  switch is in the 'SW-UART' position
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
  //while (!Serial); //Wait for the serial port to come online

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    BTserial.println(F("Module failed to respond. Please check wiring."));
    while (1); //Freeze!
  }

  nano.setRegion(REGION_OPEN); //Set to North America

  nano.setReadPower(2700); //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  //Serial.println(F("Press a key to begin scanning for tags."));
  //BTserial.println(F("Press a key to begin scanning for tags."));
  //while (!Serial.available()); //Wait for user to send a character
  //Serial.read(); //Throw away the user's character
  nano.startReading(); //Begin scanning for tags
  
  
}

void loop()
{
  //nano.startReading();
  if (nano.check() == true) //Check to see if any new data has come in from module
  {
    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

    if (responseType == RESPONSE_IS_KEEPALIVE)
    {
      Serial.println(F("Scanning"));
      BTserial.println(F("Scanning"));
      
    }
    else if (responseType == RESPONSE_IS_TAGFOUND)
    {
      //If we have a full record we can pull out the fun bits
      int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

      long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

      long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message

      byte tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

      byte tagDataBytes = nano.getTagDataBytes();

      Serial.print(F(" rssi["));
      BTserial.print(F(" rssi["));
      Serial.print(rssi);
      BTserial.print(rssi);
      Serial.print(F("]"));
      BTserial.print(F("]"));

      Serial.print(F(" freq["));
      BTserial.print(F(" freq["));
      Serial.print(freq);
      BTserial.print(freq);
      
      Serial.print(F("]"));
      BTserial.print(F("]"));

      Serial.print(F(" time["));
      BTserial.print(F(" time["));
      Serial.print(timeStamp);
      BTserial.print(timeStamp);
      Serial.print(F("]"));
      BTserial.print(F("]"));

      byte tagData[tagDataBytes];

      Serial.print(F(" data["));
      BTserial.print(F(" data["));
      for (byte x = 0 ; x < tagDataBytes ; x++)
      {
        tagData[x] = nano.msg[26 + x];
        if (nano.msg[26 + x] < 0x10) Serial.print(F("0")); //Pretty print
        Serial.print(nano.msg[26 + x], HEX);
        BTserial.print(nano.msg[26 + x], HEX);
        Serial.print(F(" "));
        BTserial.print(F(" "));
      }
      Serial.print(F("]"));
      BTserial.print(F("]"));

      byte epc[tagEPCBytes];

      //Print EPC bytes, this is a subsection of bytes from the response/msg array
      Serial.print(F(" epc["));
      BTserial.print(F(" epc["));
      for (byte x = 0 ; x < tagEPCBytes ; x++)
      {
        epc[x] = nano.msg[31 + tagDataBytes + x];
        if (nano.msg[31 + tagDataBytes + x] < 0x10) Serial.print(F("0")); BTserial.print(F("0")); //Pretty print----
        Serial.print(nano.msg[31 + tagDataBytes + x], HEX);
        BTserial.print(nano.msg[31 + tagDataBytes + x], HEX);
        Serial.print(F(" "));
        BTserial.print(F(" "));
      }
      Serial.print(F("]"));
      BTserial.print(F("]"));

      Serial.println();
      BTserial.println();

      String epcString = String((char *)epc);
      Serial.println(epcString);
      BTserial.println(epcString);

    }
    else if (responseType == ERROR_CORRUPT_RESPONSE)
    {
      Serial.println("Bad CRC");
      BTserial.println("Bad CRC");
    }
    else
    {
      //Unknown response
      Serial.print("Unknown error");
      BTserial.println("Unknown error");
    }
  }
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while(!softSerial); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while(softSerial.available()) softSerial.read();
  
  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    //nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));
    BTserial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
    //nano.startReading();
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

