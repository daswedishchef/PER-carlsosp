#include <TinyGPS++.h>

#include <SPI.h>
#include <SD.h>



/////////////////////////
// Log File Defintions //
/////////////////////////
// Keep in mind, the SD library has max file name lengths of 8.3 - 8 char prefix,
// and a 3 char suffix.
// Our log files are called "PERlogXX.csv, so "PERlog99.csv" is our max file.
#define LOG_FILE_PREFIX "PERlog" // Name of the log file.
#define MAX_LOG_FILES 100 // Number of log files that can be made
#define LOG_FILE_SUFFIX "csv" // Suffix of the log file
char logFileName[13]; // Char string to store the log file name
// Data to be logged:
#define LOG_COLUMN_COUNT 12
char * log_col_names[LOG_COLUMN_COUNT] = {
  "event", "tag id", "longitude", "latitude", "altitude", "speed", "course", "date", "time", "satellites", "temperature", "depth"
}; // log_col_names is printed at the top of the file.

/////////////////
// Definitions //
/////////////////
#define REED_PIN 4
#define CAMERA_PIN 3
#define POWER_PIN 2
#define TEMP_PIN 0
#define ARDUINO_USD_CS 10
unsigned long lastLog = 0;
int fishCnt = 0;
bool tagRead = false;
byte tagID[12];

TinyGPSPlus tinyGPS; // tinyGPSPlus object to be used throughout
#define GPS_BAUD 9600 // GPS module's default baud rate
#define RFID_BAUD 9600 // ID12la baud rate

/////////////////////////////
// Serial Port Definitions //
/////////////////////////////
#include <SoftwareSerial.h>
// Serial Pins
#define ARDUINO_GPS_RX 9 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_TX 8 // GPS RX, Arduino TX pin
SoftwareSerial ssGPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX);
#define ID12LA_RX 6 //RFID RX
#define ID12LA_TX 5 //RFID TX
SoftwareSerial rf(ID12LA_TX, ID12LA_RX);
#define BAR30_I2C 0x1110110x
// Ports
#define gpsPort ssGPS
#define rfidPort rf
#define SerialMonitor Serial

void setup()
{
  SerialMonitor.begin(9600);  // Serial initializations
  rfidPort.begin(RFID_BAUD);
  gpsPort.begin(GPS_BAUD);
  pinMode(REED_PIN, INPUT_PULLUP);  // Pin initializations
  pinMode(CAMERA_PIN,OUTPUT);
  digitalWrite(CAMERA_PIN,LOW);   // Make sure shutter transitor is off
  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);   // Turn on camera
  SerialMonitor.println("Setting up SD card."); // see if the card is present and can be initialized:
  if (!SD.begin(ARDUINO_USD_CS))
  {
    SerialMonitor.println("Error initializing SD card.");
  }
  updateFileName(); // Each time we start, create a new file, increment the number
  printHeader(); // Print a header at the top of the new file
  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  MS5837();
  SPI.begin();      // Initiate  SPI bus
  SPI.setBitOrder(MSBFIRST);  // Pressure sensor stuff
  SPI.setClockDivider(SPI_CLOCK_DIV32); // divide 16 MHz to communicate on 500 kHz  
  delay(100);
}

void loop()
{
  while(!digitalRead(REED_PIN)){
      // do nothing until reed switch is triggered
  }
    digitalWrite(CAMERA_PIN,HIGH);
    fishCnt+=1;
    tinyGPS.encode(gpsPort.read());
    if (tinyGPS.location.isUpdated()) // If the GPS data is vaild
    {
      if (logGPSData()) // Log the GPS data
      {
        SerialMonitor.println("GPS logged."); // Print a debug message
      }
      else // If we failed to log GPS
      { // Print an error, don't update lastLog
        SerialMonitor.println("Failed to log new GPS data.");
      }
    }
    else // If GPS data isn't valid
    {
      // Print a debug message. Maybe we don't have enough satellites yet.
      SerialMonitor.print("No GPS data. Sats: ");
      SerialMonitor.println(tinyGPS.satellites.value());
    }
}

byte logGPSData()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile)
  { // Print longitude, latitude, altitude (in feet), speed (in mph), course
    // in (degrees), date, time, and number of satellites.
    logFile.print(fishCnt);
    logFile.print(',');
    while(!Read_tag(tagRead)){ // keep checking RFID until positive read
    }
    for(int index=0;index<10;index++)
    {
    logFile.print(tagID[index],HEX);  // log tagID
    }
    logFile.print(',');
    logFile.print(tinyGPS.location.lng(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.location.lat(), 6);
    logFile.print(',');
    logFile.print(tinyGPS.altitude.feet(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.speed.mph(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.course.deg(), 1);
    logFile.print(',');
    logFile.print(tinyGPS.date.value());
    logFile.print(',');
    logFile.print(tinyGPS.time.value());
    logFile.print(',');
    logFile.print(tinyGPS.satellites.value());
    logFile.print(',');
    logFile.print(get_temp_C());
    logFile.print(',');
    logFile.print()
    logFile.println();
    logFile.close();
    return 1; // Return success
  }
  return 0; // If we failed to open the file, return fail
}

// printHeader() - prints our column names to the top of our log file
void printHeader()
{
  File logFile = SD.open(logFileName, FILE_WRITE); // Open the log file

  if (logFile) // If the log file opened, print our column names to the file
  {
    int i = 0;
    for (; i < LOG_COLUMN_COUNT; i++)
    {
      logFile.print(log_col_names[i]);
      if (i < LOG_COLUMN_COUNT - 1) // If it's anything but the last column
        logFile.print(','); // print a comma
      else // If it's the last column
        logFile.println(); // print a new line
    }
    logFile.close(); // close the file
  }
}

// updateFileName() - Looks through the log files already present on a card,
// and creates a new file with an incremented file index.
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // Clear logFileName string
    // Set logFileName to "gpslogXX.csv":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // If a file doesn't exist
    {
      break; // Break out of this loop. We found our index
    }
    else // Otherwise:
    {
      SerialMonitor.print(logFileName);
      SerialMonitor.println(" exists"); // Print a debug statement
    }
  }
  SerialMonitor.print("File name: ");
  SerialMonitor.println(logFileName); // Debug print the file name
}

//////////////////////
// Sensor Functions //
//////////////////////

float get_bar30(){
    if(init()){
      
    }                                       
}

bool Read_tag(bool tagRead){
  rfidPort.listen();
  if(rfidPort.available()>=13)      //Make sure all the frame is received
  {
    if(rfidPort.read()==0x02)       //Check for the start byte = 0x02
    {
      tagread=true;                //New tag is read
      for(int index=0;index<sizeof(tagID);index++)
      {
        byte val=rfidPort.read();
        
        //convert from ASCII value to value range from 0 to 9 and A to F
        if( val >= '0' && val <= '9')
        val = val - '0';
        else if( val >= 'A' && val <= 'F')
        val = 10 + val - 'A';
        tagID[index]=val;
      }
    }
    else
    {
      tagread=false;                //Discard and skip
    }
  }
  if(tagread==true){
      return true;                  //New tag is read
  }              
}
