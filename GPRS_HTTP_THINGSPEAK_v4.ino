/*
  Sketch: GPRS HTTP Test

*/
#include <gprs.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define DHTPIN 10     // what digital pin we're connected to
#define ONE_WIRE_BUS 9

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
char temp1f[5];
float temp1;
float temp2;
float batt1;
float hum;
const int batt_sens_input_pin = A2;
char buffer[512];
GPRS gprs;

void setup() {
  
  Serial.begin(9600);
  Serial.println("GPRS - HTTP Connection Test...");

  dht.begin();
  tempSensor.begin();

}

void loop()
{
  
  readSensors();  // updates temp1, temp2, hum, batt1
  
  gprs.preInit();
  
  while (0 != gprs.init()) {
    delay(1000);
    Serial.println("init error");
  }
  
  while (!gprs.join("YOUR APN HERE")) { //change "cmnet" to your own APN
    Serial.println("gprs join network error");
    delay(2000);
  }
  
  // successful DHCP
  Serial.print("IP Address is ");
  Serial.println(gprs.getIPAddress());

  Serial.println("Init success, start to connect Thingspeak...");

  if (0 == gprs.connectTCP("api.thingspeak.com", 80)) {
    Serial.println("connect api.thingspeak.com success");
  } else {
    Serial.println("connect error");
    // while(1);
  }
  Serial.println(batt1);
  Serial.println("waiting to fetch...");

  char temp1_convert[10];
dtostrf(temp1,1,2,temp1_convert);
 
char temp2_convert[10];
dtostrf(temp2,1,2,temp2_convert);
 
char hum_convert[10];
dtostrf(hum,1,2,hum_convert);
 
char batt1_convert[10];
dtostrf(batt1,1,2,batt1_convert);
 
 snprintf(
    buffer,
    sizeof(buffer),
    "GET /update?api_key=YOUR API KEY HERE&field1=%s&&field2=%s&&field3=%s&&field4=%s HTTP/1.0\r\n\r\n",
    temp1_convert,
    temp2_convert,
    hum_convert,
    batt1_convert
);
  if (0 == gprs.sendTCPData(buffer));
  
  else {
    Serial.println("connect error");
    return;
  }
  Serial.println(buffer);
  {
    // gprs.serialDebug();

    Serial.println("Reset GSM module!!");
    pinMode(6, OUTPUT);
    digitalWrite(6, LOW);
    delay(2000);
    digitalWrite(6, HIGH);
    delay(20000);
  }


}

// updates temp1, temp2, hum, batt1
void readSensors(){
  delay(2000);
  hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temp1 = dht.readTemperature();

  if (isnan(hum) || isnan(temp1)) {
    Serial.println("Failed to read from DHT sensor!");
    //return;
  }

  batt1 = (float(analogRead(batt_sens_input_pin)) / 1023.0) * 5.0;  // range: 0.0 - 5.0V

  tempSensor.requestTemperatures();
  temp2 = tempSensor.getTempCByIndex(0);
  Serial.println("Water Temp:");
  Serial.println(temp2);
  Serial.println("Air Temp:");
  Serial.println(temp1);
  Serial.println("Humidity:");
  Serial.println(hum);
  Serial.println("Battery:");
  Serial.println(batt1);
}

