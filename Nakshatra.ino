// Code for Nakshatra depth-58 feet
#include <Stepper.h>
#include <Wire.h>
//#include <SoftwareSerial.h>
#include <SoftwareWire.h>  
#include <RtcDS1307.h>
#include <SPI.h>
#include <SD.h>
#include <String.h>
// RTC code
//ThreeWire myWire(3,2,4); // IO, SCLK, CE
//RtcDS1302<ThreeWire> Rtc(myWire);
#define SDA 3
#define SCL 2
SoftwareWire myWire(SDA, SCL);
RtcDS1307<SoftwareWire> Rtc(myWire);
//SoftwareSerial gprsSerial(6,5); // GSM code
#define gprsSerial Serial

//int Node_ID = 2023;
//int sent_status = 0;
float windup_ckt = 0;
//Motor code
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
#define IN1 7
#define IN2 6
#define IN3 5
#define IN4 4
#define forcePin A7
#define Switch A5
#define chipSelect 10
#define countof(a) (sizeof(a) / sizeof(a[0]))

// Let the radius of the spool be 20 mm
//float r = 14;
//float pi = 3.14159;
//float dist_per_rev = 2 * pi * r;// =81.68
//float dist_per_step = dist_per_rev / stepsPerRevolution;//0.0398
byte state;
float dist_per_step = 0.0398;
int stepval = 162;
int reading = 0;
float distance = 0;
unsigned long int reset_time = millis();
unsigned long int timeout = millis();
float force;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

//void check_SD(){
//  Serial.print("Initializing SD card...");
//  // see if the card is present and can be initialized:
//  if (!SD.begin(chipSelect)) {
//    Serial.println("SD failed");
//    send_data(-100);
//  }
//  else{
//    Serial.println("card initialized.");
//  }
//}

void power_down_stepper(){
  for (byte i = 4 ; i <= 7 ; i++){
    state = (state << 1) + digitalRead(i);
    pinMode (i, INPUT) ;
  }
}

void power_up_stepper(){
  for (byte i = 4 ; i <= 7 ; i++) {
    pinMode (i, OUTPUT) ;
    digitalWrite (i, (state & 0x8) == 0x8);
    state <<= 1 ;
  } 
}

void writeData(float distance, const RtcDateTime& dt){
  File dataFile;
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
  char datestring[26];
  snprintf_P(datestring,  
      countof(datestring),
      PSTR("%04u-%02u-%02uT%02u:%02u:%02u+00:00"),
      dt.Year(),
      dt.Month(),
      dt.Day(),
      dt.Hour(),
      dt.Minute(),
      dt.Second());
  dataFile.print(datestring);
  dataFile.print(" ");
  dataFile.print("distance : ");
  dataFile.println(distance);
  dataFile.close();
  // print to the serial port too:
  //Serial.println(dataString);
  }
//  else {
//    Serial.println("error opening datalog.txt");
//  }
  delay(2000);
  //reset();
}

//void windUp() {
//  do {
//    windup_ckt = analogRead(Switch);
//    Serial.println("windup switch : ");
//    Serial.print(windup_ckt);
//    myStepper.step(-162);
//  } while (windup_ckt < 500.0);
//  distance = 0;
//}

void send_data(float Distance_S) {           
//  if (gprsSerial.available())
//    Serial.write(gprsSerial.read());
//  gprsSerial.println("AT");
//  delay(1000);
//  gprsSerial.println("AT+CPIN?");
//  delay(1000);
//  gprsSerial.println("AT+CREG?");
//  delay(1000);
//  gprsSerial.println("AT+CGATT?");
//  delay(1000);
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);
  ShowSerialData();
  gprsSerial.println("AT+CSTT=\"cmnet\"");//start task and setting the APN,
  delay(1000);
  ShowSerialData();
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
  ShowSerialData();
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
  ShowSerialData();
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);
  ShowSerialData();
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
  ShowSerialData();
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
//String str="GET https://api.thingspeak.com/update?api_key=FWV3D8ZLP6LXP5K3&field1=" + String(Distance_S) +"&created_at=" + String("2014-12-31T23:59:59+00:00");
//  Serial.println("GET https://api.thingspeak.com/update?api_key=6FU7YFXWVWCNCECH&field1=" + String(Distance_S));
  gprsSerial.println("GET https://api.thingspeak.com/update?api_key=6FU7YFXWVWCNCECH&field1=" + String(Distance_S));//begin send data to remote server
  delay(8000);
  ShowSerialData();
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
  ShowSerialData();
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
} 

void ShowSerialData(){
  while(gprsSerial.available()!=0)
  gprsSerial.read();
  delay(5000);   
}

void printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
//    Serial.print(datestring);
}
//char DateTime(const RtcDateTime& dt){
//  char datestring[26];
//// 2014-12-31T23:59:59+00:00
//  snprintf_P(datestring, 
//          countof(datestring),
//          PSTR("%04u-%02u-%02uT%02u:02u:02u+00:00"),
//          dt.Year(),
//          dt.Month(),
//          dt.Day(),
//          dt.Hour(),
//          dt.Minute(),
//          dt.Second() );
//  return datestring;
//}
void rtc_setup(){
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
//  Serial.println();
  if (!Rtc.IsDateTimeValid()) {
//    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning()){
//    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled){
//      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
//  else if (now > compiled) {
//      Serial.println("RTC is newer than compile time. (this is expected)");
//  }
//  else if (now == compiled){
//      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
//  }
}

void setup() {
  // set the speed at 12 rpm
  // Max rpm at 5 V is around 15 rpm
  myStepper.setSpeed(15);
  // initialize the serial port
//  Serial.begin(9600);
  gprsSerial.begin(9600);
  pinMode(Switch, INPUT);
  pinMode(forcePin, INPUT);
//  check_SD();
  delay(10000);
 if (!SD.begin(chipSelect)) {
//    Serial.println("SD failed");
    send_data(-100);
  }
//  windUp();
 timeout = millis();
 do {
    windup_ckt = analogRead(Switch);
//    Serial.println("windup switch : ");
//    Serial.print(windup_ckt);
    myStepper.step(-162);
    if(millis() - timeout > 1200000L)break;
  } while (windup_ckt < 500.0);
  distance = 0;
}

void loop() {
  // step one revolution in one direction:
  delay(300);
  for (int i = 0; i<11 ; i++) reading = (reading + analogRead(forcePin))/2;
  
//  Serial.print("Force reading = ");
//  Serial.println(reading);
//  Serial.println("");
  // This is the case when the bob is hanging in the air
  power_up_stepper();
  while (reading > 200) {
//    Serial.println("lowering bob down");
  myStepper.step(162);
  delay(200);
  distance += stepval * dist_per_step;

  for (int i = 0; i<11 ; i++) reading = (reading + analogRead(forcePin))/2;
//    Serial.println(reading);
  }
  // This is the case when the bob is floating on the water level
  while (reading <= 200) {
//    Serial.println("Pulling bob up");
  myStepper.step(-162);
  power_down_stepper();
  delay(200);
  distance -= stepval * dist_per_step;
  // Serial.println(distance);
  for (int i = 0; i<11 ; i++) reading = (reading + analogRead(forcePin))/2;
//    Serial.println(reading);
  }
  power_down_stepper();
//  Serial.print("\nDistance = ");
//  Serial.println(distance);
//  Serial.println("------------");
  send_data(distance);
  rtc_setup();
  Rtc.Begin();
  RtcDateTime now = Rtc.GetDateTime();
//  RtcDateTime now = Rtc.GetDateTime();
  writeData(distance,now);
//  windUp();
  delay(15 * 60000);
//  Serial.println("Data Sent Successfully");

  //////////////////////////////////////////////////////////////

  if (millis() - reset_time >= 24 * 60 * 60000) {
//  windUp();
    timeout = millis();
    do {
      windup_ckt = analogRead(Switch);
//    Serial.println("windup switch : ");
//    Serial.print(windup_ckt);
      myStepper.step(-162);
      if(millis() - timeout > 1200000L)break;
  } while (windup_ckt < 200);
  distance = 0;
}
}
