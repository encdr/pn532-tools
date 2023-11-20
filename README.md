## code snippets for the PN532 module using Adafruit's library

New release & documentation coming soon.<br>
<br>
<ins>Current features are: </ins> <br>
• reading an NTAG215/216/217<br>
• emulate a blank ISO14443A MiFare tag<br>
• copy a nfc-tag and emulate it via a second pn532 module<br>
  → identifies as a NXP - Mifare Classic 1k<br>
  → has NfcA, MifareClassic, Ndef Formatable Storage<br>
  → 1kByte Storage, 16 secotrs which individually 4 blocks to write to<br>
<br>
tested on an Arduino UNO R3, although it should work with many more, just make sure that it has the right CPU and enough flash memory.<br>  
To access the inbuilt menu for debugging, you must use a serial monitor application.
