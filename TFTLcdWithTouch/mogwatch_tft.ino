// the regular Adafruit "TouchScreen.h" library only works on AVRs

// different mcufriend shields have Touchscreen on different pins
// and rotation.
// Run the TouchScreen_Calibr_native sketch for calibration of your shield

#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.
#include <TouchScreen.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SensorDefaults.h"

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 22
#define WARNING_LED 23

#define TEMP_CHECK_TIME 10    // Number of seconds to wait between checking temperature sensors
#define NUM_OF_SENSORS  3     

int deviceCount = 0;
DeviceAddress Thermometer;
unsigned long time; 
unsigned long modTime = 50; 

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

struct Sensor {
  uint8_t slot;
  DeviceAddress address;
  uint8_t type;  // Portal = 1 (ambient + 20c) , Engine = 2, Gearbox = 3, Diff = 4, Air = 5 (no warn/alarm)
  char name[11]; // 10 characters + '\0'
  uint8_t warn;
  uint8_t alert;
  bool relativeToAmbient; // allows warn and alarm thresholds to be set relative to ambient temp (for portals/Diffs/engine/gearbox?)
};
// https://forum.expeditionportal.com/threads/unimog-hub-temperatures.191531/ (Reference for portals ambient + 20c)

// The different components that Temperature Sensors can monitor
#define PORTAL    0 // Front Left, Front Right, Rear Left & Rear Right - should be relative to ambient temp
#define ENGINE    1 // should NOT be relative to ambient temp???
#define GEARBOX   2 // should be relative to ambient temp
#define DIFF      3 // Front & Rear (& Rear2 for 6wd varient) - should be relative to ambient temp
#define AIR       4 // Cabin & Ambient - should not have warn or alarm levels set by default

//char *name = "Please Calibrate.";  //edit name of shield
char *name = "2.8 touch tft";
// const int XP=6,XM=A2,YP=A1,YM=7; //ID=0x9341
//const int TS_LEFT=907,TS_RT=136,TS_TOP=942,TS_BOT=139;
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x9595
const int TS_LEFT=908,TS_RT=119,TS_TOP=92,TS_BOT=888;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

// Touch screen senitivity
#define MINPRESSURE 50     // 200
#define MAXPRESSURE 1000

#define CHASSIS_AREA 120    // This is the area which the chassis is drawn in for touch references
#define LABEL_2CHAR_WIDTH 26
#define LABEL_HEIGHT      18

int currentDetails = 0;
#define DISPLAY_ENGINE_DETAILS      1      // probably should change to SENSOR_ENGINE
#define DISPLAY_FL_PORTAL_DETAILS   2
#define DISPLAY_FR_PORTAL_DETAILS   3
#define DISPLAY_F_DIFF_DETAILS      4
#define DISPLAY_GEARBOX_DETAILS     5
#define DISPLAY_R_DIFF_DETAILS      6
#define DISPLAY_RL_PORTAL_DETAILS   7
#define DISPLAY_RR_PORTAL_DETAILS   8
#define DISPLAY_CABIN_DETAILS       9
#define DISPLAY_AMBIENT_DETAILS     10
#define DISPLAY_TYRE_DETAILS        11
#define DISPLAY_FRONT_IMTERMEDIATE_TRANSFER_CASE_DETAILS    12
#define DISPLAY_REAR_IMTERMEDIATE_TRANSFER_CASE_DETAILS     13
#define DISPLAY_CLUTCH_DISPLAY      14
#define DISPLAY_FLF_BRAKE_CALIPER   15
#define DISPLAY_FLR_BRAKE_CALIPER   16
#define DISPLAY_FRF_BRAKE_CALIPER   17
#define DISPLAY_FRR_BRAKE_CALIPER   18
#define DISPLAY_RL_BRAKE_CALIPER    19
#define DISPLAY_RR_BRAKE_CALIPER    20


uint16_t TFT_ID;            // ID to control touch screen TFT
uint8_t Orientation = 1;    //0=portrait, 1=landscape, 2=other portrait, 3=other landscape

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void show_Serial(void)
{
    Serial.println(F("Most Touch Screens use pins 6, 7, A1, A2"));
    Serial.println(F("But they can be in ANY order"));
    Serial.println(F("e.g. right to left or bottom to top"));
    Serial.println(F("or wrong direction"));
    Serial.println(name);
    Serial.print(F("ID=0x"));
    Serial.println(TFT_ID, HEX);
    Serial.println("Screen is " + String(tft.width()) + "x" + String(tft.height()));
    Serial.println("Calibration is: ");
    Serial.println("LEFT = " + String(TS_LEFT) + " RT  = " + String(TS_RT));
    Serial.println("TOP  = " + String(TS_TOP)  + " BOT = " + String(TS_BOT));
    switch (Orientation) {
      case 0:
        Serial.println("Orientation is PORTRAIT");
        break;
      case 1:
        Serial.println("Orientation is LANDSCAPE");
        break;
      case 2:
        Serial.println("Orientation is other PORTRAIT");
        break;
      case 3:
        Serial.println("Orientation is other LANDSCAPE");
        break;
    }
    Serial.println("YP=" + String(YP)  + " XM=" + String(XM));
    Serial.println("YM=" + String(YM)  + " XP=" + String(XP));
}

void show_splash(void)
{
    tft.setTextSize(3);
    tft.setCursor((tft.width() - 120) / 2, (tft.height() * 2) / 5);
    tft.setTextColor(GREEN);
    tft.println("MogWatch");

    tft.setTextSize(1);
    tft.setCursor((tft.width() - 220) / 2, (tft.height() * 2) / 3);
    tft.println("a collaboration for the OzMog community");
    
    int i = 50;

    while (i-- > 0) {
        tp = ts.getPoint();
        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);
        delay(100);
        if (tp.z < MINPRESSURE/4 || tp.z > MAXPRESSURE) continue;
        if (tp.x > 250 && tp.x < 770  && tp.y > 250 && tp.y < 770) break;
    }

    tft.fillScreen(BLACK);
}

void update_tempLabel(uint8_t sensor, bool highlightLabel, uint16_t textColour, char text[3]) {
  switch (sensor) {
    case DISPLAY_ENGINE_DETAILS:
      draw_tempLabel(47, 16, highlightLabel, textColour, text);     // Engine Label
      break;
    case DISPLAY_FL_PORTAL_DETAILS:
      draw_tempLabel(23, 44, highlightLabel, textColour, text);     // Tyre front passenger
      break;
    case DISPLAY_FR_PORTAL_DETAILS:
      draw_tempLabel(86, 44, highlightLabel, textColour, text);     // Tyre front driver
      break;
    case DISPLAY_F_DIFF_DETAILS:
      draw_tempLabel(20, 85, highlightLabel, textColour, text);     // front diff Label
      break;
    case DISPLAY_GEARBOX_DETAILS:
      draw_tempLabel(50, 120, highlightLabel, textColour, text);    // Gearbox Label
      break;
    case DISPLAY_R_DIFF_DETAILS:
      draw_tempLabel(20, 150, highlightLabel, textColour, text);    // rear diff Label
      break;
    case DISPLAY_RL_PORTAL_DETAILS:
      draw_tempLabel(23, 188, highlightLabel, textColour, text);    // Tyre rear passenger
      break;
    case DISPLAY_RR_PORTAL_DETAILS:
      draw_tempLabel(86, 188, highlightLabel, textColour, text);    // Tyre rear driver
      break;
  }
}

void draw_tempLabel(int xloc, int yloc, bool highlightLabel, uint16_t textColour, char text[3]) {
  // If the label is not being highlighted draw white box around white text

  if (!highlightLabel) {
    tft.fillRect(xloc, yloc, LABEL_2CHAR_WIDTH, LABEL_HEIGHT, WHITE);
  }
  tft.fillRect(xloc+1, yloc+1, LABEL_2CHAR_WIDTH-2, LABEL_HEIGHT-2, BLACK);
  tft.setCursor(xloc+2, yloc+2);
  tft.setTextSize(2);
  if (highlightLabel) {
    textColour = RED;
  }
  tft.setTextColor(textColour);
  //tft.print(F(label));
  tft.print(text);
}

void draw_wheel(int xloc, int yloc, uint16_t tyreColour) {
  tft.drawRoundRect(xloc, yloc, 16, 46, 4, tyreColour);
}

void draw_diff(int xleftWheel, int xrightWheel, int yloc) {
  //.  __/-\___ shape
  tft.drawLine(xleftWheel, yloc, xleftWheel+18, yloc, GREEN);
  tft.drawLine(xleftWheel+18, yloc, xleftWheel+22, yloc-4, GREEN);
  tft.drawLine(xleftWheel+22, yloc-4, xleftWheel+28, yloc-4, GREEN);
  tft.drawLine(xleftWheel+28, yloc-4, xleftWheel+32, yloc, GREEN);
  tft.drawLine(xleftWheel+32, yloc, xrightWheel, yloc, GREEN);

  yloc+=6;
  //.  --\_/--- shape
  tft.drawLine(xleftWheel, yloc, xleftWheel+18, yloc, GREEN);
  tft.drawLine(xleftWheel+18, yloc, xleftWheel+22, yloc+4, GREEN);
  tft.drawLine(xleftWheel+22, yloc+4, xleftWheel+28, yloc+4, GREEN);
  tft.drawLine(xleftWheel+28, yloc+4, xleftWheel+32, yloc, GREEN);
  tft.drawLine(xleftWheel+32, yloc, xrightWheel, yloc, GREEN);
}

void draw_mog4x4chasis(bool includeBlankLabels = true)
{
  // draw front axle and wheels
  draw_wheel(20, 30, GREEN);        // Tyre front passenger
  draw_diff(36, 99, 50);            // Front diff
  draw_wheel(99, 30, GREEN);        // Tyre front driver

  // draw rear axle and wheels
  draw_wheel(20, 174, GREEN);       // Tyre rear passenger
  draw_diff(36, 99, 194);           // Rear diff
  draw_wheel(99, 174, GREEN);       // Tyre rear driver

  // Tail shafts
  tft.drawRect(59, 60, 6, 131, GREEN);
  tft.drawRect(60, 60, 4, 131, BLACK);

  // Gearbox
  tft.drawRect(53, 85, 18, 41, GREEN);
  tft.drawRect(59, 91, 30, 20, GREEN);
  tft.fillRect(54, 86, 16, 39, BLACK);
  tft.fillRect(59, 91, 12, 20, BLACK);

  // Engine
  tft.drawRect(60, 10, 27, 30, GREEN);

  // Drive shaft
  tft.drawRect(74, 40, 6, 53, GREEN);
  tft.fillRect(75, 40, 4, 52, BLACK);

  if (includeBlankLabels) {
    // Labels
    update_tempLabel(DISPLAY_ENGINE_DETAILS, false, WHITE, "--\0");
    update_tempLabel(DISPLAY_FL_PORTAL_DETAILS, false, WHITE, "--\0");
    update_tempLabel(DISPLAY_FR_PORTAL_DETAILS, false, WHITE, "--\0");
    update_tempLabel(DISPLAY_GEARBOX_DETAILS, false, WHITE, "--\0");
    update_tempLabel(DISPLAY_RL_PORTAL_DETAILS, false, WHITE, "--\0");
    update_tempLabel(DISPLAY_RR_PORTAL_DETAILS, false, WHITE, "--\0");

    update_tempLabel(DISPLAY_F_DIFF_DETAILS, false, WHITE, "--\0");     // front diff Label
    tft.drawLine(40, 84, 57, 58, WHITE);              // label pointer to front diff
    update_tempLabel(DISPLAY_R_DIFF_DETAILS, false, WHITE, "--\0");    // rear diff Label
    tft.drawLine(40, 169, 57, 192, WHITE);            // label pointer to rear diff
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

void clearEEProm() {
   // initialize the LED pin as an output.
   pinMode(WARNING_LED, OUTPUT);

   for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
   }

   // turn the LED on when we're done
   digitalWrite(WARNING_LED, HIGH);
   delay(500);
   digitalWrite(WARNING_LED, LOW);
}

void loadSensorsFromEEPROM(void) {
  // EEPROM structure
  // X -  first 2 character record the number of saves sensors (uint8_t - range 0-255)
  // [SDDDDDDDDTNNNNNNNNNNNWAR] (total bytes = 24) repeat the number of sensors that are saved
  //                         [SDDDDDDDDTNNNNNNNNNNNWAR]
  //                                                 [SDDDDDDDDTNNNNNNNNNNNWAR]
  // 012
  //    345678901234567890123456 2+(1x24)=26
  //                            789012345678901234567890 2+(2x24)=26
  //  S - Slot - the position of the sensor on the vehicle
  //   DDDDDDDD - Device Address of the sensor stored
  //           T - Type ... Portal = 1 (ambient + 20c) , Engine = 2, Gearbox = 3, Diff = 4, Air = 5 (no warn/alarm)
  //            NNNNNNNNNNN - Name/Label (11 characters = 10 + '\0')
  //                       W - Warning level threshold - 255 means don't warn
  //                        A - Alert level threshold - 255 means don't alert
  //                         R - true/false ... Warnings/Alert levels relative to ambient temp. eg. true W=40 and amb temp = 25 .. warn at 65

// sensor = (Sensor){1, "SENSOR123\0", 50, 70};

  int eeAddress = 0;
  uint8_t numSensors;
  // struct 
  Sensor sensor;
  bool newSensorFound;

  // Start up the DallasTemp library
  sensors.begin();

  // locate devices on the bus
  deviceCount = sensors.getDeviceCount();

  Serial.println("devices");
  Serial.println(deviceCount);

  // Might need to use addresses for the sensors when there are lots?!?
  for (int i = 0;  i < deviceCount;  i++) {
    newSensorFound = true;
    DeviceAddress devAddr;

    sensors.getAddress(Thermometer, i);

    printAddress(Thermometer);

    // Check each device address in the EEProm
    // If devcie address is in EEProm then load into the sensor array - does it need to be marked as present and then load present EEProms into array and warn of others
    // If not, add sensor to EEProm and then load into the sensor array

    EEPROM.get(eeAddress, numSensors);

    // Search for current device address in the EEprom
    for (int j = 0; j < numSensors; j++) {

      
      int eeDeviceLocation = 2 + (j * sizeof(Sensor)); // "2" shoulkd be the position of the first DeviceAddress if there is one recorded
      EEPROM.get(eeDeviceLocation, devAddr);

      if (Thermometer == devAddr) {
        newSensorFound = false;
        break;
      }

      Serial.println(j);

    }

    if (newSensorFound) {
      // Display screen to see if user wants to save new sensor to the EEProm
      Serial.println("New sensor found");
      printAddress(devAddr);

        // #define DISPLAY_ENGINE_DETAILS      1      // probably should change to SENSOR_ENGINE
        // #define DISPLAY_FL_PORTAL_DETAILS   2
        // #define DISPLAY_FR_PORTAL_DETAILS   3
      // sensor.slot = DISPLAY_ENGINE_DETAILS;
      // sensor.address = devAddr;
      // sensor.type = ENGINE;
      // sensor.name = "ENGINE\0";
      // sensor.warn = sensorDef[ENGINE].warn;
      // sensor.alert = sensorDef[ENGINE].alert;
      // sensor.relativeToAmbient = sensorDef[ENGINE].relativeToAmbient;
      // sensor = (Sensor){.slot = DISPLAY_ENGINE_DETAILS, .address = devAddr, .type = ENGINE, .warn = sensorDef[ENGINE].warn, .alert = sensorDef[ENGINE].alert, .relativeToAmbient =  sensorDef[ENGINE].relativeToAmbient};
      sensor.slot = DISPLAY_ENGINE_DETAILS;
      // sensor.address = devAddr;
      sensor.type = ENGINE;
      sensor.warn = sensorDef[ENGINE].warn;
      sensor.alert = sensorDef[ENGINE].alert;
      sensor.relativeToAmbient =  sensorDef[ENGINE].relativeToAmbient;
      // sensor = (Sensor){.slot = DISPLAY_ENGINE_DETAILS, .address = devAddr, .type = ENGINE, .warn = sensorDef[ENGINE].warn, .alert = sensorDef[ENGINE].alert, .relativeToAmbient =  sensorDef[ENGINE].relativeToAmbient};
      // (Sensor)
      sensor.name = "ENGINE...\0";

      // get number of sensors recorded in EEProm currently
      eeAddress = 0;
      EEPROM.get(eeAddress, numSensors);

      EEPROM.put(eeAddress, numSensors + 1);

      // point EEProm address to after the last current sensor
      eeAddress = (numSensors * sizeof(Sensor)) + 1;

      EEPROM.put(eeAddress, sensor);
    }
  }

  // // Reset EEProm
  // numSensors = 0;
  // // EEPROM.put(eeAddress, numSensors);

  // EEPROM.get(eeAddress, numSensors);

  // if (numSensors == 0) {
  //   Serial.println("No EEProm sensors recorded");

  //   // add one for testing purposes
  //   numSensors = 1;
  //   EEPROM.put(eeAddress, numSensors);

  //   eeAddress += sizeof(numSensors);
  //   // DeviceAddress dummyDevAddr;
  //   // DeviceAddress newAddress = { 0x28, 0x2A, 0x3A, 0x59, 0x04, 0x00, 0x00, 0x8B };
  //   // for (int i = 0; i < 8; i++) dummyDevAddr[i] = newAddress[i];
    
  //   // DeviceAddress dummyDevAddr = { 0x28, 0x2A, 0x3A, 0x59, 0x04, 0x00, 0x00, 0x8B };

  //   // sensor = (Sensor){1, dummyDevAddr, "SENSOR123\0", 50, 70};
  //   sensor = (Sensor){1, "SENSOR123\0", 50, 70};

  //   EEPROM.put(eeAddress, sensor);
  // } else {
  //   Serial.println(String(numSensors)  + " EEProm sensors recorded");

  //   eeAddress += sizeof(numSensors);
  //   EEPROM.get(eeAddress, sensor);
  //   Serial.print(String(sensor.slot)  + " " + sensor.name + " " + sensor.warn + " " + sensor.alarm);
  //   // printAddress(sensor.addr);
  // }

}

void setupTempSensors(void) {
  // initialize digital pin 10 as an output for LED warning light
  pinMode(WARNING_LED, OUTPUT);
  
  // Start up the DallasTemp library
  sensors.begin();

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

void setup(void)
{
    tft.reset();
    TFT_ID = tft.readID();
    tft.begin(TFT_ID);
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);

    Serial.begin(9600);
    // show_Serial();

    setupTempSensors();
    // clearEEProm(); // Reset EEProm memory to forget all sensors (might need to just clear out the number of sensors instead - quicker)
    loadSensorsFromEEPROM();

    // show_splash();
    tft.fillScreen(BLACK);
    draw_mog4x4chasis(false);
}

void mapLandscapeXYvalues(int touchX, int touchY, int* mapx, int* mapy) {
  // See commented function mapXYvalues at end of file for other orientations
  *mapx = map(touchY, TS_TOP, TS_BOT, 0, tft.width());
  *mapy = map(touchX, TS_RT, TS_LEFT, 0, tft.height());
  Serial.println("x  = " + String(*mapx)  + " y = " + String(*mapy));
}

void showDetails(int detailSummary, float displayTemp) {
  Serial.println("x  = " + String(detailSummary));
  Serial.println("x  = " + String(currentDetails));
  if (currentDetails !=  detailSummary) {
    // clear the area where the text will be displayed
    tft.fillRect(120, 80, 240, 160, RED);

    currentDetails = detailSummary;

    tft.setTextSize(2);
    tft.setCursor(130, (tft.height() * 2) / 5);
    tft.setTextColor(GREEN);

    switch (detailSummary) {
      case DISPLAY_ENGINE_DETAILS:
        tft.println("Engine Temp");
        break;
      case DISPLAY_FL_PORTAL_DETAILS:
        tft.println("FL Portal Temp");
        break;
      case DISPLAY_FR_PORTAL_DETAILS:
        tft.println("FR Portal Temp");
        break;
      case DISPLAY_F_DIFF_DETAILS:
        tft.println("Front Diff Temp");
        break;
      case DISPLAY_GEARBOX_DETAILS:
        tft.println("Gearbox Temp");
        break;
      case DISPLAY_R_DIFF_DETAILS:
        tft.println("Rear Diff Temp");
        break;
      case DISPLAY_RL_PORTAL_DETAILS:
        tft.println("RL Portal Temp");
        break;
      case DISPLAY_RR_PORTAL_DETAILS:
        tft.println("RR Portal Temp");
        break;
    }

    tft.setCursor(130, (tft.height() * 2) / 3);
    tft.println(displayTemp,0);
  }
}

float getTemps(void) {
  float tempC;
  bool warningLEDon = false;
  unsigned long tempTime;

  time = millis(); 
  
  // Get the MOD seconds for time and only do the temperature check every TEMP_CHECK_TIME (10) seconds
  tempTime = (time / 1000) % TEMP_CHECK_TIME;

  if (tempTime >= modTime) {
    modTime = tempTime; 
  } else {
    modTime = 0;
    // Serial.println("Checking temps");

    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    // pinMode(XM, OUTPUT);
    // pinMode(YP, OUTPUT);

    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    sensors.requestTemperatures(); 

    // Serial.print("TimerT:"); time = millis();Serial.println(time); 

    // Display temperature from each sensor
    for (int i = 0;  i < deviceCount;  i++)
    {
      // Serial.print("TimeS:"); time = millis();Serial.println(time); 
      // Serial.print("Sensor ");
      // Serial.print(i+1);
      // Serial.print(" : ");
      sensors.getAddress(Thermometer, i);
      tempC = sensors.getTempCByIndex(i);
      // Serial.print(tempC,1);
      // Serial.print("c  |  ");

      // Turn on the Warning LED if the temp is greater than 24c
      if (tempC > 24 && !warningLEDon) {
        warningLEDon = true;
      }
    }

    if (warningLEDon) {
      // Turn on the High temp LED  
      digitalWrite(WARNING_LED, HIGH);
    } else {
      // Turn off the High temp LED
      digitalWrite(WARNING_LED, LOW);
    }

  }

  return tempC;
}

void loop()
{
    uint16_t xpos, ypos;  //screen coordinates
    int detailArea = 0;
  
    float aTemp = 0;

    // modify loop to get temps less frequently - every 5-10 seconds
    aTemp = getTemps();

    // Display temps (only if changed)

    tp = ts.getPoint();   //tp.x, tp.y are ADC values

        // if sharing pins, you'll need to fix the directions of the touchscreen pins
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!

    // Serial.println("z  = " + String(tp.z));
            
    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
      // Map touch values for X and Y to calibrated X and Y values for this orientation
      mapLandscapeXYvalues(tp.x, tp.y, &xpos, &ypos);

      // Check where the touch was
      if (xpos < CHASSIS_AREA) {                                     // TODO: work out the chasis touch area and define in variable
          // this is within the chasis touch area

            // Label positions y/height for touch selection (start = y and end = y+18):
            // Engine          10 - 28
            //      36
            // Front portals   44 - 62
            //      74
            // Front diff      85 - 103
            //      111
            // Gearbox        120 - 138
            //      144
            // Rear diff      150 - 168
            //      178
            // Rear portals   188 - 206
          if (ypos < 36) {
            // Engine Area
            detailArea = DISPLAY_ENGINE_DETAILS;
          } else if (ypos < 74) {
            // Front Portal Area
            if (xpos < CHASSIS_AREA/2) {  
              detailArea = DISPLAY_FL_PORTAL_DETAILS;
            } else {
              detailArea = DISPLAY_FR_PORTAL_DETAILS;
            }
          } else if (ypos < 111) {
            // Front Diff Area
            detailArea = DISPLAY_F_DIFF_DETAILS;
          } else if (ypos < 144) {
            // Gearbox Area
            detailArea = DISPLAY_GEARBOX_DETAILS;
          } else if (ypos < 178) {
            // Rear Diff Area
            detailArea = DISPLAY_R_DIFF_DETAILS;
          } else {
            // Rear Portal Area
            if (xpos < CHASSIS_AREA/2) {  
              detailArea = DISPLAY_RL_PORTAL_DETAILS;
            } else {
              detailArea = DISPLAY_RR_PORTAL_DETAILS;
            }
          }
          showDetails(detailArea, aTemp);
        } else {
          // this is the details area
          tft.setTextSize(3);
          tft.setCursor(((tft.width() - 120) / 4) * 3, (tft.height() * 2) / 5);
          tft.setTextColor(GREEN);
          tft.println("MogWatch");
        }
    }
}

/*void old_loop()
{
    uint16_t xpos, ypos;  //screen coordinates
    tp = ts.getPoint();   //tp.x, tp.y are ADC values

    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!

    if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE) {
        // most mcufriend have touch (with icons) that extends below the TFT
        // screens without icons need to reserve a space for "erase"
        // scale the ADC values from ts.getPoint() to screen values e.g. 0-239
        //
        // Calibration is true for PORTRAIT. tp.y is always long dimension 
        // map to your current pixel orientation

        switch (Orientation) {
            case 0:
                xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
                ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
                break;
            case 1:
                xpos = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
                ypos = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());
                break;
            case 2:
                xpos = map(tp.x, TS_RT, TS_LEFT, 0, tft.width());
                ypos = map(tp.y, TS_BOT, TS_TOP, 0, tft.height());
                break;
            case 3:
                xpos = map(tp.y, TS_BOT, TS_TOP, 0, tft.width());
                ypos = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());
                break;
        }

        // are we in top color box area ?
        if (ypos < BOXSIZE) {               //draw white border on selected color box
            oldcolor = currentcolor;

            if (xpos < BOXSIZE) {
                currentcolor = RED;
                tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
            } else if (xpos < BOXSIZE * 2) {
                currentcolor = YELLOW;
                tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, WHITE);
            } else if (xpos < BOXSIZE * 3) {
                currentcolor = GREEN;
                tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, WHITE);
            } else if (xpos < BOXSIZE * 4) {
                currentcolor = CYAN;
                tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, WHITE);
            } else if (xpos < BOXSIZE * 5) {
                currentcolor = BLUE;
                tft.drawRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, WHITE);
            } else if (xpos < BOXSIZE * 6) {
                currentcolor = MAGENTA;
                tft.drawRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, WHITE);
            }

            if (oldcolor != currentcolor) { //rub out the previous white border
                if (oldcolor == RED) tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
                if (oldcolor == YELLOW) tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
                if (oldcolor == GREEN) tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
                if (oldcolor == CYAN) tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
                if (oldcolor == BLUE) tft.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
                if (oldcolor == MAGENTA) tft.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);
            }
        }
        // are we in drawing area ?
        if (((ypos - PENRADIUS) > BOXSIZE) && ((ypos + PENRADIUS) < tft.height())) {
            tft.fillCircle(xpos, ypos, PENRADIUS, currentcolor);
        }
        // are we in erase area ?
        // Plain Touch panels use bottom 10 pixels e.g. > h - 10
        // Touch panels with icon area e.g. > h - 0
        if (ypos > tft.height() - 10) {
            // press the bottom of the screen to erase
            tft.fillRect(0, BOXSIZE, tft.width(), tft.height() - BOXSIZE, BLACK);
        }
    }
}*/



// old code
/*void show_tft(void)
{
    tft.setCursor(50, 0);
    tft.setTextSize(1);
    tft.print(F("ID=0x"));
    tft.println(ID, HEX);
    tft.println("Screen is " + String(tft.width()) + "x" + String(tft.height()));
    tft.println("");
    tft.setTextSize(2);
    tft.println(name);
    tft.setTextSize(1);
    tft.println("Values:");
    tft.println("LEFT = " + String(TS_LEFT) + " RT  = " + String(TS_RT));
    tft.println("TOP  = " + String(TS_TOP)  + " BOT = " + String(TS_BOT));
    tft.println("\nWiring is: ");
    tft.println("YP=" + String(YP)  + " XM=" + String(XM));
    tft.println("YM=" + String(YM)  + " XP=" + String(XP));
    tft.setTextSize(2);
    tft.setTextColor(RED);
    tft.setCursor((tft.width() - 48) / 2, (tft.height() * 2) / 4);
    tft.print("EXIT");
    tft.setTextColor(YELLOW, BLACK);
    tft.setCursor(0, (tft.height() * 6) / 8);
    tft.print("Touch screen for loc");
    while (1) {
        tp = ts.getPoint();
        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);
        if (tp.z < MINPRESSURE || tp.z > MAXPRESSURE) continue;
        if (tp.x > 450 && tp.x < 570  && tp.y > 450 && tp.y < 570) break;
        tft.setCursor(0, (tft.height() * 3) / 4);
        tft.print("tp.x=" + String(tp.x) + " tp.y=" + String(tp.y) + "   ");
    }
}*/

//void mapXYvalues(int touchX, int touchY, int* mapx, int* mapy) {
  // most mcufriend have touch (with icons) that extends below the TFT
  // screens without icons need to reserve a space for "erase"
  // scale the ADC values from ts.getPoint() to screen values e.g. 0-239
  //
  // Calibration is true for PORTRAIT. tp.y is always long dimension 
  // map to your current pixel orientation

  // convert the values of X and Y from the touch to a 0-320 and 0-240 value (or vice versa)
  // based on the screen orientation and the calibration settings
/*  switch (Orientation) {
    case 0:
      *mapx = map(touchX, TS_LEFT, TS_RT, 0, tft.width());
      *mapy = map(touchY, TS_TOP, TS_BOT, 0, tft.height());
      break;
    case 1:*/
      //
      //
      // Remove commented sections to be able to change orientation
      // Hard coded to landscape orientation for the moment
      //
      //
//      *mapx = map(touchY, TS_TOP, TS_BOT, 0, tft.width());
//      *mapy = map(touchX, TS_RT, TS_LEFT, 0, tft.height());
/*      break;
    case 2:
      *mapx = map(touchX, TS_RT, TS_LEFT, 0, tft.width());
      *mapy = map(touchY, TS_BOT, TS_TOP, 0, tft.height());
      break;
    case 3:
      *mapx = map(touchY, TS_BOT, TS_TOP, 0, tft.width());
      *mapy = map(touchX, TS_LEFT, TS_RT, 0, tft.height());
      break;
  }*/
  // Serial.println("x  = " + String(*mapx)  + " y = " + String(*mapy));
// }

// Other sensors reference:
// GPS/Speedo - https://how2electronics.com/diy-speedometer-using-gps-module-arduino-oled/
