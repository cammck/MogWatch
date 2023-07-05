# MogWatch
Auto sensor arduino project aimed at monitoring Mercedes Benz Unimog vital telemetry

Initially intended to be used to monitor the following (and alert when threshold breached):
- portal temps (x4)
- engine temp
- radiator/cooling temp (is this any different from engine temp?)
- transmission temp
- diff temp (2)
- charge voltage (24-28v)
  
Additionally, to assess other possible sensors
- Tyre Pressures (x4) + spares (x2?)
- Cabin temp?
- air pressures?

# Phases
Phase.1
Simple sketch to connect Arduino Uno R3 to a 16x2 LCD and an 18B20 temperature sensor, with simple logic to send temp to LCD and turn on a warning LED when temp exceeds a threshold (24c)

Phase.2
Add multiple 18B20 sensors and trigger warning LED when one or more temp sensors exceeds threshold

Phase.3
Assess best Arduino device and Colour OLED display for purposes

Phase.4
Working the

# todo
get Fritzing working and sketch the wiring diagram for Phase 1
