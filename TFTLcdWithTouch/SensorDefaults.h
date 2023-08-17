// All default values for Sensors

// Defaults for Portal sensors
#define SENSOR_DEFAULT_PORTAL_WARN  20
#define SENSOR_DEFAULT_PORTAL_ALERT 40
#define SENSOR_DEFAULT_PORTAL_REL   true

// Defaults for Engine sensors
#define SENSOR_DEFAULT_ENGINE_WARN  80
#define SENSOR_DEFAULT_ENGINE_ALERT 100
#define SENSOR_DEFAULT_ENGINE_REL   false

// Defaults for Gearbox sensors
#define SENSOR_DEFAULT_GEARBOX_WARN  80
#define SENSOR_DEFAULT_GEARBOX_ALERT 100
#define SENSOR_DEFAULT_GEARBOX_REL   false

// Defaults for Diff sensors
#define SENSOR_DEFAULT_DIFF_WARN  80
#define SENSOR_DEFAULT_DIFF_ALERT 100
#define SENSOR_DEFAULT_DIFF_REL   false

// Defaults for Air based sensors (eg. Cabin or Ambient)
#define SENSOR_DEFAULT_AIR_WARN  80
#define SENSOR_DEFAULT_AIR_ALERT 100
#define SENSOR_DEFAULT_AIR_REL   false

// Structure to hold Sensor defaults so that an array can be initialised
struct SensorDefaults {
  uint8_t warn;
  uint8_t alert;
  bool relativeToAmbient;
};

// Initialise the array with the specified default values above
SensorDefaults sensorDef[5] = {
  {SENSOR_DEFAULT_PORTAL_WARN, SENSOR_DEFAULT_PORTAL_ALERT, SENSOR_DEFAULT_PORTAL_REL},
  {SENSOR_DEFAULT_ENGINE_WARN, SENSOR_DEFAULT_ENGINE_ALERT, SENSOR_DEFAULT_ENGINE_REL},
  {SENSOR_DEFAULT_GEARBOX_WARN, SENSOR_DEFAULT_GEARBOX_ALERT, SENSOR_DEFAULT_GEARBOX_REL},
  {SENSOR_DEFAULT_DIFF_WARN, SENSOR_DEFAULT_DIFF_ALERT, SENSOR_DEFAULT_DIFF_REL},
  {SENSOR_DEFAULT_AIR_WARN, SENSOR_DEFAULT_AIR_ALERT, SENSOR_DEFAULT_AIR_REL}
};
