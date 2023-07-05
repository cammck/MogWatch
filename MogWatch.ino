#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 4
#define WARNING_LED 10

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
  // initialize digital pin 10 as an output for LED warning light
  pinMode(WARNING_LED, OUTPUT);
  
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  
  // Start up the DallasTemp library
  sensors.begin();
  
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("- MogWatch -");
}

void loop()
{
  float temp;
  
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 

  // get the temp value in Celcius of the first temp sensor on the one wire bus pin (4)
  temp = sensors.getTempCByIndex(0);

  // output the temp to the serial/debug console
  Serial.print("Celsius temperature: ");
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.print(temp); 

  // Move the cursor to the 2nd line of the LCD and print the temp with 1 decimal point
  lcd.setCursor(1,1);
  lcd.print(temp,1); 
  lcd.print("c");

  // Turn on the Warning LED if the temp is greater than 24c
  if (temp > 24) {
    // Turn on the High temp LED
    digitalWrite(WARNING_LED, HIGH);
  } else {
    // Turn off the High temp LED
    digitalWrite(WARNING_LED, LOW);
  }
}
