#include <Encoder.h>
Encoder motorLeft(4, 5);  // Pins for left motor encoder
Encoder motorRight(6, 7); // Pins for right motor encoder (adjust as needed)
// Motor pin definitions
#define m1 12 // Right Motor pin1
#define m2 11 // Right Motor pin2
#define m3 9  // Left Motor pin1
#define m4 8  // Left Motor pin2
#define e1 10 // Right Motor Enable
#define e2 13 // Left Motor Enable

// 5 Channel IR Sensor Connections
#define ir1 A5
#define ir2 A4
#define ir3 A3
#define ir4 A2
#define ir5 A1

// PD control parameters
double Kp = 120;     // Proportional gain
double Kd = 35;      // Derivative gain
double setpoint = 2; // Center sensor position
double last_error = 0;
long last_time = 0;
long l_time = 0;
double output = 0;
double last_output = 0;
long oldLeftPos = 0, oldRightPos = 0;
bool is_in_black = false;
const int trigPin = 3;
const int echoPin = 2;

void setup()
{
    Serial.begin(9600);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    // Motor pins
    pinMode(m1, OUTPUT);
    pinMode(m2, OUTPUT);
    pinMode(m3, OUTPUT);
    pinMode(m4, OUTPUT);
    pinMode(e1, OUTPUT);
    pinMode(e2, OUTPUT);

    // IR sensor pins
    pinMode(ir1, INPUT);
    pinMode(ir2, INPUT);
    pinMode(ir3, INPUT);
    pinMode(ir4, INPUT);
    pinMode(ir5, INPUT);
}

void loop()
{
    long duration, inches, cm;
    // IR sensor readings

    double s1 = !digitalRead(ir1); // Left Most Sensor
    double s2 = !digitalRead(ir2); // Left Sensor
    double s3 = !digitalRead(ir3); // Middle Sensor
    double s4 = !digitalRead(ir4); // Right Sensor
    double s5 = !digitalRead(ir5); // Right Most Sensor

    if (s2 == 1.0)
    {
        s1 = 1;
    }
    if (s4 == 1.0)
    {
        s5 = 1;
    }
    double sum_sensor = (s1 + s2 + s3 + s4 + s5);

    double sensor_line = 0;

    if (sum_sensor != 0)
    {
        sensor_line = (s1 * 0 + s2 * 1 + s3 * 2 + s4 * 3 + s5 * 4) / sum_sensor;
    }

    // Calculate error
    double error = setpoint - sensor_line;

    /*
       // Apply stronger correction when outermost sensors are active
      if (s1 == 1) {  // Far left sensor sees line
        error = -2.0;  // Strong left turn command (adjust value as needed)
      }
      else if (s5 == 1) {  // Far right sensor sees line
        error = 2.0;   // Strong right turn command (adjust value as needed)
      }
      */
    // Calculate time difference
    long current_time = millis();
    double delta_time = (current_time - last_time) / 1000.0; // Convert to seconds
    last_time = current_time;

    // Calculate derivative term
    double derivative = (error - last_error) / delta_time;
    last_error = error;

    if (sum_sensor == 5)
    {
        output = last_output;
    }
    else
    {
        // Calculate PD output
        output = Kp * error + Kd * derivative;
        last_output = output;
    }

    // Base motor speeds
    int base_speed = 60;
    int right_speed = base_speed + output;
    int left_speed = base_speed - output;

    // Constrain speeds to valid PWM range (0-255)
    right_speed = constrain(right_speed, 0, 100);
    left_speed = constrain(left_speed, 0, 100);

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // The same pin is used to read the signal from the PING))): a HIGH pulse
    // whose duration is the time (in microseconds) from the sending of the ping
    // to the reception of its echo off of an object.

    duration = pulseIn(echoPin, HIGH);

    // convert the time into a distance
    inches = microsecondsToInches(duration);
    cm = microsecondsToCentimeters(duration);
    if (cm <= 10)
    {
        analogWrite(e1, 0);
        analogWrite(e2, 0);

        cm = 0;
    }
    else
    {

        analogWrite(e1, right_speed);
        analogWrite(e2, left_speed);
    }
    // Set motor speeds

    // Set motor directions (forward)
    digitalWrite(m1, LOW);
    digitalWrite(m2, HIGH);
    digitalWrite(m3, LOW);
    digitalWrite(m4, HIGH);

    // Debugging output
    Serial.print("Error: ");
    Serial.print(error);
    Serial.print(" Output: ");
    Serial.print(output);
    Serial.print(" Right: ");
    Serial.print(right_speed);
    Serial.print(" Left: ");
    Serial.println(left_speed);
}

long microsecondsToInches(long microseconds)
{
    // According to Parallax's datasheet for the PING))), there are 73.746
    // microseconds per inch (i.e. sound travels at 1130 feet per second).
    // This gives the distance travelled by the ping, outbound and return,
    // so we divide by 2 to get the distance of the obstacle.
    // See: https://www.parallax.com/package/ping-ultrasonic-distance-sensor-downloads/
    return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the object we
    // take half of the distance travelled.
    return microseconds / 29 / 2;
}