/*
 * Copyright (c) 2021 Noam Inbari, weizmann institute of science
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Title:           Temperature monitoring using rest server
 * Author:          Noam Inbari <noam.inbari@weizmann.ac.il>
 * Version:         v1.1.0
 */

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <RestServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
///////////////////
//#define _DEBUG 
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

#define NUM_DEVICES 6
#define TEMP_PATH "/readout"
#define RESET_PATH "/reset"
#define NO_READOUT_VAL -127.0

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/
// Create a new instance of the oneWire class to communicate with any OneWire device:
OneWire oneWire(5);
OneWire oneWireB(6);

// Pass the oneWire reference to DallasTemperature library:
DallasTemperature sensors(&oneWire);
DallasTemperature sensorsB(&oneWireB);

float tempC[NUM_DEVICES];
static long int tStamp = 0;

/*----------------------------------------------------------------------------*/
/* Prototypes                                                                 */
/*----------------------------------------------------------------------------*/
EthernetServer server(80);
RestServer rest(server);    // You need to pass the EthernetServer reference to the RestServer
void(*resetFunc) (void) = 0;  // Declare reset function at address 0
void getData(char * params = "");
void softReset(char * params = "") ;

/*----------------------------------------------------------------------------*/
/* Functions                                                                  */
/*----------------------------------------------------------------------------*/
/*
 * Initilize
 */
void setup() 
{
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
  IPAddress ip(10, 0, 0, 2);

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
  rest.addRoute(GET, TEMP_PATH, getData);
  rest.addRoute(GET, RESET_PATH, softReset);

  // Start Ethernet server
  server.begin();

  Serial.print(Ethernet.localIP());
  Serial.print(TEMP_PATH);
  Serial.println("    <GET, Temperatures>");
  Serial.print(Ethernet.localIP());
  Serial.print(RESET_PATH);
  Serial.println("    <SET, Soft Reset>");
}

/*
 * Main loop
 */
void loop() 
{
  // Run the RestServer
  rest.run();

  long int now = millis();
  if (now > tStamp + 1000)
  {
    DEBUG_PRINT("interval = ");
    DEBUG_PRINTLN(now - tStamp);

    int cnt = locateDevices();
    if (cnt != NUM_DEVICES)
    {
      DEBUG_PRINT(NUM_DEVICES - cnt);
      DEBUG_PRINTLN(" Devices are missing");
    }

    getTemp(cnt / 2);
    tStamp = now;
  }
}

/*
 * Misc
 */
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

void getData(char * params = "") 
{
  int i;
  char inxStr[2];
  
  for(i=0; i< NUM_DEVICES; i++)
  {
    itoa(i, inxStr, 10);
    if(tempC[i] != NO_READOUT_VAL)
    { 
       rest.addData(inxStr, tempC[i]);
    }
  }
}

void softReset(char * params = "") 
{
  resetFunc();
}

void getTemp(int cnt)
{
  // Send the command for all devices on the bus to perform a temperature conversion:
  sensors.requestTemperatures();
  sensorsB.requestTemperatures();

  // Display temperature from each sensor
  for (int i = 0; i < cnt; i++)
  {
    tempC[i] = sensors.getTempCByIndex(i);
    if (tempC[i] != NO_READOUT_VAL)
    {
      // Serial.print("Sensor 1-3  ");
      DEBUG_PRINT(i);
      DEBUG_PRINT(" : ");

      DEBUG_PRINT(tempC[i]);
      DEBUG_PRINT(" \xC2\xB0"); // shows degree symbol
      DEBUG_PRINTLN("C");
    }

    tempC[i + 3] = sensorsB.getTempCByIndex(i);
    if (tempC[i + 3] != NO_READOUT_VAL)
    {
      //   Serial.print("Sensor 4-6  ");
      DEBUG_PRINT(i + 3);
      DEBUG_PRINT(" : ");

      DEBUG_PRINT(tempC[i + 3]);
      DEBUG_PRINT(" \xC2\xB0"); // shows degree symbol
      DEBUG_PRINTLN("C");
    }
  }
}
