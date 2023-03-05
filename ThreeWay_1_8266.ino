/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPLJd3EI3uQ"
#define BLYNK_TEMPLATE_NAME "ThreeWay"
#define BLYNK_AUTH_TOKEN "NI5cEj1kMras8DqVDBkDZMvSWwn5DVFi"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial/* Configuration of NTP */





#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
//#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
//#include <HTTPClient.h>
#include <FS.h>
// Set web server port number to 80
WiFiServer server(80);
int currentTimeArr[5];

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";
#define RELAY_PIN  4
//int currentRelayPin = 0;

int led_gpio = 2;

WiFiManager wm;
//AsyncWebServer server(80);
//DNSServer dns;
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");

int buttonPressed = 0;
int buttonOn = 0;
int oldButtonValue = 99;
bool firstTime = true;
int * retVal = NULL; 
int ii = 0;
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};




int* setCurrentTime();
    String readFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char * path, const char * message);

float moData[120];
long notificationTimer = 3600000;
long beginNotificationTimer = 0;
bool firstTimeNotification = true;
    long utcOffsetInSeconds = 0; 

//int randomTemp = 0;
bool sent = false;
int tab2 = 0;
int tab1 = 0;
int slider1 = 0;
int slider2 = 0;
int x = 0;
float averageValue = 0.0;

// You should get Auth Token in the Blynk App.
//// Go to the Project Settings (nut icon).
char auth[] = BLYNK_AUTH_TOKEN;



int *curTimeArr;
int currentHour;
int currentMinute;
int currentDay;
int readValue = 0;
int maxValue = 0;
int minValue = 0;

const int current_sensor_pin = A0;  //four pins down on the left
int sensor_max = 0;
float amplitude_current = 0.0; //amplitude current
float effective_value = 0.0; //effective current
int deviceState = 0;
int device_on_current_threshold = 0;
bool lightStarted = false;
bool lightStopped = false;
bool wifiConnected = false;
//HTTPClient https;
bool blinking = false;
bool relayOff = true;
int startAMPM = 0;
int stopAMPM = 0;
int startHour= 0;
int stopHour = 0;
bool schedulingOn = false;

// Define NTP Client to get time
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");
time_t now;                         // this is the epoch
tm tm;                              // the structure tm holds time information in a more convenient way

NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600);
BlynkTimer timer;

bool isDaylightSavingsTime();
void setTime();
int lightsState();
void flipSwitch();
void setStartTime(int amPm, int hr);
void setStopTime(int amPm, int hr);
void setWeb();

  BLYNK_WRITE(V0)  // button attached to Virtual Pin 1 in SWITCH mode
  {
    Serial.print("V0 is: ");
    buttonPressed = 1;
    
    int i = param.asInt();
    Serial.println(i);
    if (i == 1) {
      buttonOn = 1;
      Serial.println("Button is ON");
    }
    else {
      buttonOn = 0;
      Serial.println("Button is OFF");
    }
  }

  BLYNK_WRITE(V4) {
    stopAMPM = param.asInt();
    String strStopAMPM = String(stopAMPM);
    writeFile(SPIFFS,"stopAMPM",strStopAMPM.c_str());
    Serial.print("stopAMPM is: ");
    Serial.println(stopAMPM);
  }  
  BLYNK_WRITE(V3) {
    startAMPM = param.asInt();
    String strStartAMPM = String(startAMPM);
    writeFile(SPIFFS,"startAMPM",strStartAMPM.c_str());

    Serial.print("startAMPM is: ");
    Serial.println(startAMPM);
    Serial.print("Hour is; ");
    Serial.println(currentTimeArr[3]);
  }  

  BLYNK_WRITE(V2) {
    tab2 = param.asInt();
    Serial.print("tab2 is: ");
    Serial.println(tab2);
      setTime();
      setStopTime(stopAMPM, tab2);
    String strTab2 = String(tab2);
    writeFile(SPIFFS,"tab2",strTab2.c_str());
    String strSlider2 = String(slider2);
    writeFile(SPIFFS,"slider2",strSlider2.c_str());
    Serial.print("Slider2 is ");
      Serial.println(slider2);
  }

  BLYNK_WRITE(V1) {
    tab1 = param.asInt();
    Serial.print("tab1 is: ");
    Serial.println(tab1);
      setTime();
        setStartTime(startAMPM,tab1);
     
      String strTab1 = String(tab1);
      writeFile(SPIFFS, "tab1", strTab1.c_str());
    
      String strSlider1 = String(slider1);
      writeFile(SPIFFS, "slider1", strSlider1.c_str());
        Serial.print("Slider1 is ");
      Serial.println(slider1);
  }

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
 // Blynk.virtualWrite(V2, millis() / 1000);
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(led_gpio, OUTPUT);
  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN,LOW);
  delay(10);
  //configTime(MY_TZ, MY_NTP_SERVER); // --> Here is the IMPORTANT ONE LINER needed in your sketch!

/////////////////////////////8266 wifi //////////////////////
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Under_Cabinet_Lights");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  
  server.begin();
///////////////////////////////////////////////////////////
//////////time ///////////

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
 // NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600);
  //timeClient.setTimeOffset(-21600);  //IST is UTC+5:30 Hrs
/*  if(isDaylightSavingsTime()){
    timeClient.setTimeOffset(-21600+3600);
  }else{
    Serial.println("time offset -216000");
    timeClient.setTimeOffset(-21600);
  }*/
  setTime();
/////////////////////////
  Blynk.config(BLYNK_AUTH_TOKEN);
    while (Blynk.connect() == false) {
  }
    if (Blynk.connected()) {
    Serial.println("Wifi/Blynk started");
  } else {
    Serial.println("Check Router");
  }
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  
  // mount and check the filesystem
  bool ok = SPIFFS.begin();
  if (ok) {
    Serial.println("SPIFFS initialized successfully");
    }
  else{
    Serial.println("SPIFFS intialization error");
    }
    /////////////////
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,0);
  //currentRelayPin = 0;
  //EEPROM.get(1, tab1);
  String strTab1 = readFile(SPIFFS,"tab1");
  tab1 = strTab1.toInt();
  Serial.print("tab1 is ");
  Serial.println(tab1);
Blynk.virtualWrite(V1, tab1);  
   
     String strSlider1 = readFile(SPIFFS,"slider1");
  slider1 = strSlider1.toInt();
  Serial.print("slider1 from eeprom is ");
  Serial.println(slider1);
  
  String strTab2 = readFile(SPIFFS,"tab2");
  tab2 = strTab2.toInt();
  Blynk.virtualWrite(V2, tab2);
  
   String strSlider2 = readFile(SPIFFS,"slider2");
  slider2 = strSlider2.toInt();
   
  String strStartAMPM = readFile(SPIFFS,"startAMPM");
  startAMPM = strStartAMPM.toInt();
  Blynk.virtualWrite(V3, startAMPM);
  
  String strStopAMPM = readFile(SPIFFS,"stopAMPM");
  stopAMPM = strStopAMPM.toInt();
  Serial.print("in set up stopAMPM is ");
  Serial.println(stopAMPM);
  Blynk.virtualWrite(V4, stopAMPM);

 
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("LOST WiFi restarting");
    //atoAwcUtil->webSerialWriteLine("LOST WiFi restarting");
    ESP.restart();
  }
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
  if(blinking){
    digitalWrite(led_gpio, 0);
    blinking = false;
  }else{
    digitalWrite(led_gpio, 1);
    blinking = true;
  }

    if(buttonPressed){ //turn light on
    //Serial.println("Button WAS pressed!!!!!!!!");
    //currentState();
    int state = lightsState();
   // Serial.print("currentState is ");
    //Serial.print(deviceState);


    if(state){
      deviceState = 1;
    }else{
      deviceState = 0;
    }
    buttonPressed = 0;
    if(buttonOn == 0 && deviceState == 1){
        //Serial.print("buttOn is ");
       // Serial.println(buttonOn);
       // Serial.println("DO NOTHING");
      flipSwitch();
    }else if(buttonOn == 1 && deviceState == 1){
       // Serial.print("buttOn is ");
        //Serial.println(buttonOn);
       // Serial.println("FLIP SWITCH");
      //do nothing

    }else if(buttonOn == 1 && deviceState == 0){
        //Serial.print("buttOn is ");
        //Serial.println(buttonOn);
        //Serial.println("DO NOTHING");
      flipSwitch();
    }else if(buttonOn == 0 && deviceState == 0){
        //Serial.print("buttOn is ");
        //Serial.println(buttonOn);
        //Serial.println("FLIP SWITCH");
        //do nothing
    }
  
    }
    /////////////////// do schedule //////////////////////
    setTime();
  //setStartTime(startAMPM, tab1);
  //setStopTime(stopAMPM,tab2);
  /*Serial.print("HOUR_________________");
  Serial.print(currentTimeArr[3]);
  Serial.print("SLIDERS____________________");
  Serial.print(slider1);
   Serial.print("SLIDERS____________________");
 Serial.println(slider2);*/
  if(slider1 == currentTimeArr[3] && !schedulingOn){
    schedulingOn = true;
    
    if(deviceState == 0 && !lightStarted){
      Serial.println("SCHEDULER STARTED");
      flipSwitch();
      lightStarted = true;
    }else{
      //do nothing
    }
  }
  if(slider2 == currentTimeArr[3] && schedulingOn){
    
    if(deviceState == 1 && !lightStopped){
      Serial.println("SCHEDULER STOPPED");
      flipSwitch();
      lightStopped = true;    
      }else{
      //do nothing
      }
  }
      if(currentHour == slider2+1){
      lightStarted = false;
      lightStopped = false;
    }
  
    ////////////////////////////////////////////////////
    setWeb();
    delay(1000);
}



void setTime(){
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  //Serial.print("Epoch Time: ");
  //Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  //Serial.print("Formatted Time: ");
  //Serial.println(formattedTime);  
  int currentHour = timeClient.getHours();
  currentTimeArr[3] = currentHour;
  //Serial.print("Hour: ");
  //Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  currentTimeArr[4] = currentMinute;
  //Serial.print("Minutes: ");
  //Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  //Serial.print("Seconds: ");
  //Serial.println(currentSecond);  
  String weekDay = weekDays[timeClient.getDay()];
  //Serial.print("Week Day: ");
  //Serial.println(weekDay);    
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int monthDay = ptm->tm_mday;
  currentTimeArr[2] = monthDay;
  //Serial.print("Month day: ");
  //Serial.println(monthDay);
  int currentMonth = ptm->tm_mon+1;
  currentTimeArr[1] = currentMonth;
  //Serial.print("Month: ");
  //Serial.println(currentMonth);
  String currentMonthName = months[currentMonth-1];
  //Serial.print("Month name: ");
  //Serial.println(currentMonthName);
  int currentYear = ptm->tm_year+1900;  
  currentTimeArr[0] = currentYear;
  
  //Serial.print("Year: ");
  //Serial.println(currentYear);
  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  //Serial.print("Current date: ");
  //Serial.println(currentDate);
  //Serial.println("");


}

bool isDaylightSavingsTime(){
  setTime();
  int yr = currentTimeArr[0];
  //Serial.print(yr);
  int dayOfMonth = currentTimeArr[2];
  //Serial.print(dayOfMonth);
  int hr = currentTimeArr[3];
  //Serial.print(hr);
  int mo = currentTimeArr[1];
  //Serial.print(mo);
  int DST = 0;

  // ********************* Calculate offset for Sunday *********************
  int y = yr;                          // DS3231 uses two digit year (required here)
  int x = (y + y / 4 + 2) % 7;    // remainder will identify which day of month
  // is Sunday by subtracting x from the one
  // or two week window.  First two weeks for March
  // and first week for November
  // *********** Test DST: BEGINS on 2nd Sunday of March @ 2:00 AM *********
  if (mo == 3 && dayOfMonth == (14 - x) && hr >= 2)
  {
    DST = 1;                           // Daylight Savings Time is TRUE (add one hour)
  }
  if ((mo == 3 && dayOfMonth > (14 - x)) || mo > 3)
  {
    DST = 1;
  }
  // ************* Test DST: ENDS on 1st Sunday of Nov @ 2:00 AM ************
  if (mo == 11 && dayOfMonth == (7 - x) && hr >= 2)
  {
    DST = 0;                            // daylight savings time is FALSE (Standard time)
  }
  if ((mo == 11 && dayOfMonth > (7 - x)) || mo > 11 || mo < 3)
  {
    DST = 0;
  }
  if (DST == 1)                       // Test DST and add one hour if = 1 (TRUE)
  {
    hr = hr + 1;
  }
  if (DST == 1) {
    return true;
  } else {
    return false;
  }
}

int lightsState(){
uint32_t start_time = millis();
int state1 =0;
float result = 0;




   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
     if(ii == 0){
       averageValue = 0;
     }
     ii++;
       readValue = analogRead(current_sensor_pin);
       averageValue = (averageValue + readValue)/ii;
       //Serial.print(averageValue);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the minimum sensor value*/
           minValue = readValue;
       }
       
   }
   result = ((maxValue - minValue) * 3.3)/4096.0;
   ii = 0;
   
Serial.print("maxValue is ");
Serial.println(maxValue);
Serial.print("minValue is ");
Serial.println(minValue);
Serial.print("result  is ");
Serial.print(result);
  if(result > 0.25){
//Serial.print(sensorValue);
//start_time = 0;
    state1 = 1;
  }else{
    state1 = 0;
  }
  Serial.print("State is ");
  Serial.println(state1);
return state1;
}

void flipSwitch(){
      if(relayOff){
      digitalWrite(RELAY_PIN,HIGH);
      Serial.println("Relay is on");
      delay(500);
      digitalWrite(RELAY_PIN,LOW);
      relayOff = true;
    }else{
      digitalWrite(RELAY_PIN,LOW);
      Serial.println("Relay is off");
      relayOff = true;
    }
}

void setStartTime(int amPm,int hr){
  Serial.println("in setStartTime");
  Serial.print("hr is ");
  Serial.print(hr);
  if(amPm == 1){
    //it is pm
    if(hr == 12){
      slider1 = 0;
    }else if(hr == 1){
      slider1 = 13;      
    }else if(hr == 2){
      slider1 = 14;      
    }else if(hr == 3){
      slider1 = 15;      
    }else if(hr == 4){
      slider1 = 16;      
    }else if(hr == 5){
      slider1 = 17;      
    }else if(hr == 6){
      slider1 = 18;      
    }else if(hr == 7){
      slider1 = 19;      
    }else if(hr == 8){
      Serial.print("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
      slider1 = 20;      
    }else if(hr == 9){
      slider1 = 21;      
    }else if(hr == 10){
      slider1 = 22;      
    }else if(hr == 11){
      slider1 = 23;      
    }
  }else{
    slider1 = hr;
  }
  //Serial.print(slider1);
}

void setStopTime(int amPm, int hr){
  if(amPm == 1){
    //it is pm
    if(hr == 12){
      slider2 = 0;
    }else if(hr == 1){
      slider2 = 13;      
    }else if(hr == 2){
      slider2 = 14;      
    }else if(hr == 3){
      slider2 = 15;      
    }else if(hr == 4){
      slider2 = 16;      
    }else if(hr == 5){
      slider2 = 17;      
    }else if(hr == 6){
      slider2 = 18;      
    }else if(hr == 7){
      slider2 = 19;      
    }else if(hr == 8){
      slider2 = 20;      
    }else if(hr == 9){
      slider2 = 21;      
    }else if(hr == 10){
      slider2 = 22;      
    }else if(hr == 11){
      slider2 = 23;      
    }
  }else{
    slider2 = hr;
  }
  //Serial.print(slider2);
}

////////////////////////////////////////////////////////////////////
//
//  Fuction: readFile
//
//  Input:  SPIFFS,
//          char[]  filename path i.e. "/String.txt"
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
String readFile(fs::FS &fs, const char * path) {
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    //Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  //Serial.println(fileContent);
  file.close();
  return fileContent;
}

////////////////////////////////////////////////////////////////////
//
//  Fuction: writeFile
//
//  Input:  SPIFFS,
//          char[] filename path i.e. "/String.txt"
//          String to store
//
//  Output:  String of what was strored
//
//  Discription:  Stores a string in the /path in SPIFFS
//
/////////////////////////////////////////////////////////////////////
void writeFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Writing file: %s\r\n", path);
  //Serial.print("path is : ");
  //Serial.println(path);
  //fs.remove(path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    //WebSerial.println("?EW"); //TODO change to right code
    return;
  }
  
  if (file.print(message)) {
    //Serial.println("- file written");
    if ((strcmp(path, "/timezone.txt") == 0)) {
      //Serial.println("Timezone changed!!!!!!!!11");
    }

  } else {
    Serial.println("- write failed");
  }
}

void setWeb(){
  String t1 = readFile(SPIFFS, "tab1");
  Blynk.virtualWrite(V1,t1.toInt());
  String t2 = readFile(SPIFFS, "tab2");
  Blynk.virtualWrite(V2,t2.toInt());
  String start1 = readFile(SPIFFS, "startAMPM");
  Blynk.virtualWrite(V3,start1.toInt());
  String stop1  = readFile(SPIFFS,"stopAMPM");
  Blynk.virtualWrite(V4,stop1.toInt());
}
