// - 200mm에서 최대 밝기, 100/300mm에서 꺼짐, 150/250mm에서 duty 50%

#define PIN_LED  9    // PWM 가능 핀 (active low)
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)
#define _DIST_PEAK 200.0  // distance at which LED is brightest (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);  // sonar TRIGGER
  pinMode(PIN_ECHO, INPUT);   // sonar ECHO
  digitalWrite(PIN_TRIG, LOW);  // turn-off Sonar
  analogWrite(PIN_LED, 255);    // LED OFF

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float distance;
  int duty;   // analogWrite 값 (0: 가장 밝음, 255: 꺼짐)

  // wait until next sampling time. // polling
  // 뺄셈으로 비교하면 millis()가 오버플로(약 50일)되어도 올바르게 동작함
  if (millis() - last_sampling_time < INTERVAL)
    return;

  distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

  if ((distance == 0.0) || (distance > _DIST_MAX)) {
    distance = _DIST_MAX + 10.0;    // Set Higher Value
    duty = 255;                     // LED OFF
  } else if (distance < _DIST_MIN) {
    distance = _DIST_MIN - 10.0;    // Set Lower Value
    duty = 255;                     // LED OFF
  } else {    // In desired Range
    duty = distance_to_duty(distance);
  }
  analogWrite(PIN_LED, duty);

  // output to the serial port (serial plotter)
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",duty:");      Serial.print(duty);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}

//   밝기 비율 = 1 - |distance - 200| / 100   (200mm: 1.0, 150/250mm: 0.5, 100/300mm: 0.0)
//   active low이므로 duty = 255 * (1 - 밝기 비율)
// 정수 나눗셈으로 계산하면 결과가 0이 되므로 float로 계산 후 마지막에 int로 변환
int distance_to_duty(float distance)
{
  float brightness = 1.0 - fabs(distance - _DIST_PEAK) / (_DIST_PEAK - _DIST_MIN);
  if (brightness < 0.0) brightness = 0.0;
  if (brightness > 1.0) brightness = 1.0;

  return (int)(255.0 * (1.0 - brightness) + 0.5);
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}
