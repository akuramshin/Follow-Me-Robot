// Imports
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include "./Definitions.h"

// GPS
TinyGPSPlus gps;
#define GPS_BAUD 4800

// Motors
Servo servoThrottleRight;
Servo servoThrottleLeft;

// Master Enable
bool enabled = true;

// Bluetooth input
bool started = false;
bool ended = false;
char inData[80]; // creates an 80 character array called "inData"
byte index; //creates a variable type=byte called "index"
double Long; //variable for longitude coordinate
double Lat; //variable for latitude coordinate

// Serial components
SoftwareSerial bluetoothSerial(BLUETOOTH_TX_PIN, BLUETOOTH_RX_PIN);
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN); // TXD to digital pin 6

/* Compass */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

GeoLoc checkGPS() {
  Serial.println("Reading onboard GPS: ");
  unsigned long start = millis();
  while (millis() - start < GPS_UPDATE_INTERVAL) {
    // If we recieved new location then take the coordinates and pack them into a struct
    if (feedgps())
      return gpsdump();
  }

  GeoLoc robotLoc;
  robotLoc.lat = 0.0;
  robotLoc.lon = 0.0;
  
  return robotLoc;
}

// Get and process GPS data
GeoLoc gpsdump() {
  GeoLoc robotLoc;
  robotLoc.lat = gps.location.lat();
  robotLoc.lon = gps.location.lng();

  Serial.print(robotLoc.lat, 7); Serial.print(", "); Serial.println(robotLoc.lon, 7);

  return robotLoc;
}

// Feed data as it becomes available 
bool feedgps() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read()))
      return true;
  }
  return false;
}

void displayCompassDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

#ifndef DEGTORAD
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#endif

float geoBearing(struct GeoLoc &a, struct GeoLoc &b) {
  float y = sin(b.lon-a.lon) * cos(b.lat);
  float x = cos(a.lat)*sin(b.lat) - sin(a.lat)*cos(b.lat)*cos(b.lon-a.lon);
  return atan2(y, x) * RADTODEG;
}

float geoDistance(struct GeoLoc &a, struct GeoLoc &b) {
  const float R = 6371000; // radius of earth in metres
  float p1 = a.lat * DEGTORAD;
  float p2 = b.lat * DEGTORAD;
  float dp = (b.lat-a.lat) * DEGTORAD;
  float dl = (b.lon-a.lon) * DEGTORAD;

  float x = sin(dp/2) * sin(dp/2) + cos(p1) * cos(p2) * sin(dl/2) * sin(dl/2);
  float y = 2 * atan2(sqrt(x), sqrt(1-x));

  // returns distance in meters
  return R * y;
}

float geoHeading() {
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);

  // Offset
  heading -= DECLINATION_ANGLE;
  heading -= COMPASS_OFFSET;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  // Map to -180 - 180
  while (headingDegrees < -180) headingDegrees += 360;
  while (headingDegrees >  180) headingDegrees -= 360;

  return headingDegrees;
}

void setSpeedMotorA(int speed) {  
  servoThrottleRight.writeMicroseconds(speed + MOTOR_A_OFFSET);
}

void setSpeedMotorB(int speed) {
  servoThrottleLeft.writeMicroseconds(speed + MOTOR_B_OFFSET);
}

void stop() {
  // stop the motors
  servoThrottleRight.writeMicroseconds(RC_NEUTRAL);
  servoThrottleLeft.writeMicroseconds(RC_NEUTRAL);
}

void drive(int distance, float turn) {
  int fullSpeed = 230;
  int stopSpeed = 0;

  // drive to location
  int s = fullSpeed;
  // Slowing down??? Needed???
  if ( distance < 8 ) {
    int wouldBeSpeed = s - stopSpeed;
    wouldBeSpeed *= distance / 8.0f;
    s = stopSpeed + wouldBeSpeed;
  }
  
  int autoThrottle = constrain(s, stopSpeed, fullSpeed);
  autoThrottle = 230;

  float t = turn;
  while (t < -180) t += 360;
  while (t >  180) t -= 360;
  
  Serial.print("turn: ");
  Serial.println(t);
  Serial.print("original: ");
  Serial.println(turn);
  
  float t_modifier = (180.0 - abs(t)) / 180.0;
  float autoSteerA = 1;
  float autoSteerB = 1;

  if (t < 0) {
    autoSteerB = t_modifier;
  } else if (t > 0){
    autoSteerA = t_modifier;
  }

  Serial.print("steerA: "); Serial.println(autoSteerA);
  Serial.print("steerB: "); Serial.println(autoSteerB);

//  int speedA = (int) (((float) map(autoThrottle, stopSpeed, fullSpeed, RC_NEUTRAL, RC_MAX)) * autoSteerA);
//  int speedB = (int) (((float) map(autoThrottle, stopSpeed, fullSpeed, RC_NEUTRAL, RC_MIN)) * autoSteerB);
  int speedA = (int) (((float) map(autoThrottle, stopSpeed, fullSpeed, RC_NEUTRAL, RC_MIN)) * autoSteerA);
  int speedB = (int) (((float) map(autoThrottle, stopSpeed, fullSpeed, RC_NEUTRAL, RC_MAX)) * autoSteerB);
  
  setSpeedMotorA(speedA);
  setSpeedMotorB(speedB);
}

void driveTo(struct GeoLoc &loc, int timeout) {
  gpsSerial.listen();
  GeoLoc robotLoc = checkGPS();
  bluetoothSerial.listen();

  if (robotLoc.lat != 0 && robotLoc.lon != 0 && enabled) {
    float distance = 0;
    //Start move loop here
    do {
      gpsSerial.listen();
      robotLoc = checkGPS();
      bluetoothSerial.listen();
      
      distance = geoDistance(robotLoc, loc);
      float bearing = geoBearing(robotLoc, loc) - geoHeading();
      
      Serial.print("Distance: ");
      Serial.println(distance);
    
      Serial.print("Bearing: ");
      Serial.println(geoBearing(robotLoc, loc));

      Serial.print("Heading: ");
      Serial.println(geoHeading());
      
      drive(distance, bearing);
      timeout -= 1;
    } while (distance > 1.0 && enabled && timeout>0);

    stop();
  }
}

void setupCompass() {
   /* Initialise the compass */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displayCompassDetails();
}

void setup()
{
  // Attaching motors
  servoThrottleRight.attach(PIN_SPEED_RIGHT);
  servoThrottleLeft.attach(PIN_SPEED_LEFT);
  
  //Debugging via serial
  Serial.begin(115200);
  
  // Compass
  setupCompass();

  //GPS
  gpsSerial.begin(GPS_BAUD);

  //Bluetooth
  bluetoothSerial.begin(9600);
  //Blynk.begin(bluetoothSerial, auth);
}

// Testing
void testDriveNorth() {
  float heading = geoHeading();
  int testDist = 5;
  Serial.println(heading);
  
  while(!(heading < 5 && heading > -5)) {
    drive(testDist, heading);
    heading = geoHeading();
    Serial.println(heading);
    delay(500);
  }
  
  stop();
}

void loop()
{
  //Blynk.run();
  // Read all serial data available, as fast as possible
  while (bluetoothSerial.available() > 0)
  {
    char inChar = ((byte)bluetoothSerial.read());
    if (inChar == SOP)
    {
      index = 0;
      inData[index] = '\0';
      started = true;
      ended = false;
    }
    else if (inChar == EOP)
    {
      ended = true;
      break;
    }
    else
    {
      if (index < 79)
      {
        inData[index] = inChar;
        index++;
        inData[index] = '\0';
      }
    }
  }

  // We are here either because all pending serial
  // data has been read OR because an end of
  // packet marker arrived. Which is it?
  if (started && ended)
  {
    char *token = strtok(inData, ","); 
    boolean first = true;
    while (token)
    {
      double val = atof(token);
      token = strtok(NULL, ",");     
      if (first){
        Lat = val;
      }else{
        Long = val;
      }
      first = false;
    }

    // Reset for the next packet
    started = false;
    ended = false;
    index = 0;
    inData[index] = '\0';

    GeoLoc phoneLoc;
    phoneLoc.lat = Lat;
    phoneLoc.lon = Long;

    Serial.print(phoneLoc.lat, 7); Serial.print(", "); Serial.println(phoneLoc.lon, 7);
    driveTo(phoneLoc, GPS_WAYPOINT_TIMEOUT);
  }
}
