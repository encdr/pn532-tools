                               ////---- SETTINGS ---- /////

// COMPORT: basically any, preferred are USB 3.0 (because of faster transfer limits)
// BAUDRATE: needs to be 115200
// TRANSFER: switch your PN532-module into SPI 

#include <Arduino.h>              // used so that I can use funcs, definitions, variables from the offical Arduino IDE
#include <Wire.h>                 // common wire library, its needed so it justs works
#include <SPI.h>                  // we communicate over SPI, since some functions wont work over I2C 
#include <Adafruit_PN532.h>       // custom library that got added, do not try to use the official Adafruit_PN532 library!

/// PIN-Definition for the first Arduino ///

#define PN532_SCK_I  (2)          // PIN2 for SCK
#define PN532_MOSI_I (3)          // PIN3 for MOSI
#define PN532_SS_I   (4)          // PIN4 for SS
#define PN532_MISO_I (5)          // PIN5 for MISO

#define PN532_IRQ_I   (10)         // PIN10 for IRQ (NOT USED)
#define PN532_RESET_I (11)         // PIN11 for RST (NOT USED)


/// PIN-Definition for the second Arduino ///

#define PN532_SCK_II  (6)          // PIN6 for SCK         
#define PN532_MOSI_II (7)          // PIN7 for MOSI
#define PN532_SS_II   (8)          // PIN8 for SS
#define PN532_MISO_II (9)          // PIN9 for MISO

#define PN532_IRQ_I   (12)         // PIN12 for IRQ (NOT USED)
#define PN532_RESET_I (13)         // PIN13 for RST (NOT USED)

Adafruit_PN532 nfc(PN532_SCK_I, PN532_MISO_I, PN532_MOSI_I, PN532_SS_I);              // first PN532 module  
Adafruit_PN532 nfc2(PN532_SCK_II, PN532_MISO_II, PN532_MOSI_II, PN532_SS_II);         // second PN532 module     

void setup(void) {    
  Serial.begin(115200);                                                               // 115200 Baudrate, cause its blazingly f a s t 
  delay(2000);                                                                        // lazy fix, so the text is not displayed multiple times when uploading to the Arduino
  while (!Serial) delay(10);

  nfc.begin();                                                                        // init. nfc      
  nfc2.begin();                                                                       // init. nfc2
  nfc.SAMConfig();                                                                    // int. sam config for rx & tx  
  nfc2.SAMConfig();                                                                   // int. sam config for rx & tx (nfc2) 

  Serial.println("- - debug menu || made with <3 by kai - -");                        // intro
  Serial.println(" ");                                                                // blankspace
                                                                                                                     
  uint32_t versiondata = nfc2.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("could not find the PN53X board, check your wiring.");
    while(1); 
  }

  Serial.print("[>] identified board as: pn5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("[>] firmware on the chip is: "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  nfc2.setPassiveActivationRetries(0xFF);                                               // sets the max. number of retry attempts, it just fancy

  delay(200);

  Serial.println(" ");
  Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
  Serial.println(" ");
  Serial.println("[1] dump contents of the chip");
  Serial.println("[2] emulate a blank ISO1443A nfc-tag");
  Serial.println("[3] copy a tag and emulate it");
  Serial.println(" ");
  Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
}

void ReadNFC(){                                                                       // this is the ReadNFC function, it attempts to read the contents of the chip (must be NTAG2XX) that is in the area of the reader

  uint8_t success;                                                                    // if succeeded, then proceed
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };                                            // store the value, in this case a 7-digit UID
  uint8_t uidLength;                                                                  // UID-Length of 7 bytes
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);         // expects the type ISO14443A, the 3D-printer rings are exactly that chip type

  if(success) {
    Serial.println(" ");
    Serial.println("[..] found chip, reading info...");
    Serial.print("[..] uid length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("[..] uid value: ");
    nfc.PrintHex(uid, uidLength);

    if (uidLength == 7)
    {
      uint8_t data[32];
      Serial.println("[..] chip identified as a NTAG215/216/217");
      Serial.println(" ");
      Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
      Serial.println(" ");

      for (uint8_t i = 0; i < 42; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        if (success)
        {
          nfc.PrintHexChar(data, 4);
        }
        else
        {
          Serial.println("[?] could not read the requested PAGE of the chip, try again & hold still!");
        }
      }
    }
    else
    {
      Serial.println(" ");
      Serial.println("[!] size differs from expected value --> this is not an NTAG2XX chip");
    }
  }
}

void EmulateBlankTag(){                                                               // emulates an NFC-Tag, currently a ISO-14443-4 (which we need, it w0rks!)
  
  uint8_t sendbuf[] = {0x04, 0x03, 0x02, 0x01};                                       // UID-format of a NTAG215 chip   
  uint8_t resbuf[42];                                                                 // buffersize that holds the chip
  uint8_t reslen;

  if(nfc2.TgInitAsTarget()) {
    Serial.println("[..] emulation started successfully!");
    if(!nfc2.TgGetData(resbuf, &reslen)) {
      Serial.println("[!] failed to get Data");
    }
    nfc2.TgSetData(sendbuf, sizeof(sendbuf));
    Serial.print("[..] got following Response: ");
    nfc2.PrintHex(resbuf, reslen);
    Serial.println("[..] data that got sent: ");
    nfc2.PrintHex(sendbuf, sizeof(sendbuf));
  }
  delay(1000);
}

void Extend(){

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  
  uint8_t uidLength;
  uint8_t emulatedexpect;

  emulatedexpect = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);   

  if(emulatedexpect){

    if(uidLength == 7){
      uint8_t data[16];
      Serial.println("[..] compatible chip layer found!");

    for (uint8_t i = 0; i < 42; i++)
    {
      nfc.ntag2xx_ReadPage(i, data);
    }

    /* this is very w.i.p, as it dosent seem to work when you set the variables in the library to represent a MIFARE 1443A*/

    Serial.println("[..] copying contents of the chip");
    Serial.println("[..] initialzing #2 nfc-module as host");
    nfc2.TgInitAsTarget();                                                                  // init. nfc-module as host machine
    nfc2.TgSetData(data, sizeof(data));                                                     // set host data to that what we have read in the beginning
    Serial.println("[..] setting data 1:1 as the tag");
    Serial.println("[..] emulation started successfully started!");
    }
  }
}

void DebugMenu(){

    if (Serial.read() == '1'){                                                              // fancy debug menu listener, almost a hidden keylogger haha 
    Serial.println(" ");
    Serial.print("[> 1.] please hold your nfc-chip in against of the reader");
    Serial.println(" ");
    ReadNFC();
  }
  else if (Serial.read() == '2')
  {
    Serial.println(" ");
    Serial.print("[> 2.] please hold your reader against the PN532 Module");
    Serial.println(" ");
    EmulateBlankTag();
  }
  else if (Serial.read() == '3')
  {
    Serial.println(" ");
    Serial.print("[> 3.] please hold your nfc-tag against the reader");
    Serial.println(" ");
    Extend();
  }
}

void loop(void) {                                                                       // definitions must be declared before the void loop starts, otherwise these cannot get called
  DebugMenu();
} 
