//Includes

#include <RestServer.h>
//#include <UIPEthernet.h>
//#include <Log.h>

// Include the required Arduino libraries:
#include <OneWire.h>
#include <DallasTemperature.h>

// Create a new instance of the oneWire class to communicate with any OneWire device:

// Pass the oneWire reference to DallasTemperature library:
OneWire oneWire (5) ;
OneWire oneWireB (6) ;

DallasTemperature sensors (&oneWire) ;
DallasTemperature sensorsB (&oneWireB) ;


EthernetServer server(80);
RestServer rest(server);  // You need to pass the EthernetServer reference to the RestServer

#define NUM_DEVICES 6
#define TEMP_DIR "/t"
float tempC[NUM_DEVICES];
 
///////////////////
#define _DEBUG 
//////////////////

#ifdef _DEBUG
    #define DEBUG_PRINTLN Serial.println
    #define DEBUG_PRINT Serial.print
    #define DEBUG_STR "Begin, debug ON"
#else
    #define DEBUG_PRINTLN 
    #define DEBUG_PRINT
    #define DEBUG_STR "Begin, debug OFF"
#endif

int locateDevices()
{
  int deviceCount = 0;
  
  // Locate the devices on the bus:
  DEBUG_PRINTLN("Locating devices...");
  DEBUG_PRINT("Found ");

  deviceCount = sensors.getDeviceCount();
  deviceCount += sensorsB.getDeviceCount();

  DEBUG_PRINT(deviceCount);
  DEBUG_PRINTLN(" devices");

  return deviceCount;
}

void temperature(char * params = "") {
  rest.addData("0", tempC[0]);
  rest.addData("1", tempC[1]);
  rest.addData("2", tempC[2]);
  rest.addData("3", tempC[3]);
  rest.addData("4", tempC[4]);
  rest.addData("5", tempC[5]);
}

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields

   // Begin serial communication at a baud rate of 9600:
  Serial.begin(9600);
  // Start up the library:
  sensors.begin();
  sensorsB.begin();
   
  Serial.println(DEBUG_STR);
   
  // Start Ethernet lib
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  IPAddress ip(10,0,0,2);
 
  // start the Ethernet connection and the server:
  //Ethernet.begin(mac, ip);
    Ethernet.begin(mac);
 
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  
  // Add routes to the REST Server
    rest.addRoute(GET, TEMP_DIR, temperature);
    
  // Start Ethernet server
  server.begin();
 // Serial.print("Server is at ");
 // Serial.println( Ethernet.localIP() );

 //  Serial.print("info is at ");
   Serial.print( Ethernet.localIP() );  
   Serial.println( TEMP_DIR );
}

void getTemp(int cnt)
{
  // Send the command for all devices on the bus to perform a temperature conversion:
  sensors.requestTemperatures();
  sensorsB.requestTemperatures();
  
  // Display temperature from each sensor
  for (int i = 0;  i < cnt;  i++) 
  { 
     tempC[i] = sensors.getTempCByIndex(i);
     if (tempC[i] != -127)
     {
       // Serial.print("Sensor 1-3  ");
        DEBUG_PRINT(i);
        DEBUG_PRINT(" : ");
       
        DEBUG_PRINT(tempC[i]);
        DEBUG_PRINT(" \xC2\xB0"); // shows degree symbol
        DEBUG_PRINTLN("C");
     }

     tempC[i+3] = sensorsB.getTempCByIndex(i);
     if (tempC[i+3] != -127)
     {
   //   Serial.print("Sensor 4-6  ");
      DEBUG_PRINT(i + 3);
      DEBUG_PRINT(" : ");
      
      DEBUG_PRINT(tempC[i+3]);
      DEBUG_PRINT(" \xC2\xB0"); // shows degree symbol
      DEBUG_PRINTLN("C");
     }
  }
}

static long int tStamp = 0;
void loop() {
  // Run the RestServer
  rest.run();

   long int now = millis();

   if (now > tStamp + 1000)
   {
      DEBUG_PRINT("interval = ");
      DEBUG_PRINTLN(now - tStamp);
      
      int cnt = locateDevices();
      if(cnt != NUM_DEVICES)
      {
        DEBUG_PRINT(NUM_DEVICES - cnt);
        DEBUG_PRINTLN(" Devices are missing");
      }
         
       getTemp(cnt);
       tStamp = now;
   }
}
