// Pin variables
#define GPS_TX_PIN 6
#define GPS_RX_PIN 3

#define PIN_SPEED_RIGHT 9
#define PIN_SPEED_LEFT 5

#define BLUETOOTH_TX_PIN 10
#define BLUETOOTH_RX_PIN 11

// Bluetooth GPS input
#define SOP '<'
#define EOP '>'

// Motor stuffs
#define RC_NEUTRAL 1500
#define RC_MAX 1600
#define RC_MIN 1400

// If one motor tends to spin faster than the other, add offset
#define MOTOR_A_OFFSET 0
#define MOTOR_B_OFFSET 0

// You must then add your 'Declination Angle' to the compass, which is the 'Error' of the magnetic field in your location.
// Find yours here: http://www.magnetic-declination.com/
// Mine is: 13° 24' E (Positive), which is ~13 Degrees, or (which we need) 0.23 radians
#define DECLINATION_ANGLE -0.17f

// The offset of the mounting position to true north
// It would be best to run the /examples/magsensor sketch and compare to the compass on your smartphone
#define COMPASS_OFFSET 0.35f

// How often the GPS should update in MS
// Keep this above 1000
#define GPS_UPDATE_INTERVAL 1000

// Number of changes in movement to timeout for GPS waypoints
// Keeps the robot from driving away if there is a problem
#define GPS_WAYPOINT_TIMEOUT 25

// Struct to combine our coordinates into one struct for ease of use
struct GeoLoc {
  float lat;
  float lon;
};
