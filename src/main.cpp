#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

#define GreenPin 9
#define RedPin 10
#define BluePin 11

int prevValue = 0;
bool available = true;
bool initLoop = true;

Adafruit_LiquidCrystal lcd(0x20);

void resetLcd() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);
}

void setRemoteState(char cmd1, char cmd2, char value) {
  Serial1.write(0x7e);            // Start byte
  Serial1.write((byte)0x0);       // High byte of length (always 0)
  Serial1.write(0x10);            // Low byte of length (the number of bytes that follow, not including checksum)
  Serial1.write(0x17);            // 0x17 is a remote AT command
  Serial1.write((byte)0x0);       // Frame id set to zero for no reply

  // ID for recipient, or set to broadcast 0xFFFF
  Serial1.write((byte)0x0);
  Serial1.write((byte)0x0);
  Serial1.write((byte)0x0);
  Serial1.write((byte)0x0);
  Serial1.write((byte)0x0);
  Serial1.write((byte)0x0);
  Serial1.write(0xFF);
  Serial1.write(0xFF);

  // 16 bit of recipient or 0xFFFE for unknown
  Serial1.write(0xFF);
  Serial1.write(0xFE);

  Serial1.write(0x02);            // 0x02 apply changes immediately

  // Command name in ASCII characters
  Serial1.write(cmd1);
  Serial1.write(cmd2);

  // Command data in as many bytes as needed
  Serial1.write(value);

  // Checksum is all bytes after length bytes
  long sum = 0x17 + 0xFF + 0xFF + 0xFF + 0xFE + 0x02 + cmd1 + cmd2 + value;
  Serial1.write(0xFF - ( sum & 0xFF ));   // Calculate the proper checksum
}

void setup() {  
  Serial1.begin(9600);
  Serial.begin(9600);

  pinMode(GreenPin, OUTPUT);
  pinMode(RedPin, OUTPUT);
  pinMode(BluePin, OUTPUT);

  analogWrite(BluePin, 0);

  // This while loop only works for debugging
  // with the console. It needs to be commented out
  // when not using the console.
  // while (!Serial)
  // {
  //   ; // Do nothing and wait for serial connection
  // }

  Serial.println("Serial lines initialized.");

  // set up the LCD's number of rows and columns:
  if (!lcd.begin(16, 2)) {
    Serial.println("Could not init backpack. Check wiring.");
    while(1);
  }

  lcd.setBacklight(255);
  Serial.println("LCD initialized. Backlight set to 255.");

  // Print a message to the LCD.
  resetLcd();
  lcd.print("Available");
  Serial.println("Available");
  setRemoteState('D', '2', 0x0);
  analogWrite(GreenPin, 255);
  setRemoteState('D', '3', 0x5);
  analogWrite(RedPin, 0);
  available = true;
}

void loop() {
  if (initLoop) {
    initLoop = false;
    return;
  }

  if (Serial1.available() > 21) {
    int tempByte = Serial1.read();

    if (tempByte == 126) {
      for (int i = 0; i < 19; i++) {
        tempByte = Serial1.read();
      }

      tempByte = Serial1.read();
      Serial.print("Temp: ");
      Serial.println(tempByte);
      Serial.print("Prev: ");
      Serial.println(prevValue);
      
      if (tempByte != prevValue) {
        prevValue = tempByte;

        if (available) {
          resetLcd();
          lcd.print("In Meeting");
          setRemoteState('D', '2', 0x5);
          analogWrite(GreenPin, 0);
          setRemoteState('D', '3', 0x0);
          analogWrite(RedPin, 255);
          available = false;
        }
        else {
          resetLcd();
          lcd.print("Available");
          setRemoteState('D', '2', 0x0);
          analogWrite(GreenPin, 255);
          setRemoteState('D', '3', 0x5);
          analogWrite(RedPin, 0);
          available = true;
        }
      }
    }
  }
}
