// #include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 4
#define WARNING_LED 10

int deviceCount = 0;

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

  DeviceAddress Thermometer;
  
  // Start serial communication for debugging purposes
  Serial.begin(9600);
  
  // Start up the DallasTemp library
  sensors.begin();
  
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	lcd.backlight();
	lcd.print("- MogWatch -");

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");

  // Might need to use addresses for the sensors when there are lots?!?
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" address : ");
    sensors.getAddress(Thermometer, i);
    printAddress(Thermometer);
  }
}

void loop()
{
  float tempC;
  int charCol;
  bool warningLEDon = false;
  
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 

  // get the temp value in Celcius of the first temp sensor on the one wire bus pin (4)
  
  // Display temperature from each sensor
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    tempC = sensors.getTempCByIndex(i);
    Serial.print(tempC);
    Serial.print((char)176);//shows degrees character
    Serial.print("C  |  ");

    charCol = i*5;
    lcd.setCursor(charCol,1);
    lcd.print(tempC,1); 
    lcd.print("c"); // degrees character

    // Turn on the Warning LED if the temp is greater than 24c
    if (tempC > 24 && !warningLEDon) {
      warningLEDon = true;
    }
  }

  Serial.println();

  if (warningLEDon) {
    // Turn on the High temp LED  
    digitalWrite(WARNING_LED, HIGH);
  } else {
    // Turn off the High temp LED
    digitalWrite(WARNING_LED, LOW);
  }

}

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}
