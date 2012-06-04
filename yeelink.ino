/*
 Yeelink sensor client example
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <math.h>

int BH1750address = 0x23;
byte buff[2];

// for yeelink api
#define APIKEY         "9cdf51696fa9ddfacdf819033a5f2f63" // replace your yeelink api key here
#define DEVICEID       4 // replace your device ID
#define SENSORID       7 // replace your sensor ID

// assign a MAC address for the ethernet controller.
byte mac[] = { 0x00, 0x1D, 0x72, 0x82, 0x35, 0x9D};
// initialize the library instance:
EthernetClient client;
//char server[] = "api.yeelink.net";   // name address for yeelink API
IPAddress server(202,136,60,231);      // numeric IP for api.pachube.com

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 30*1000; // delay between 2 datapoints, 30s

void setup() {
  Wire.begin();
  // start serial port:
  Serial.begin(57600);
  // start the Ethernet connection with DHCP:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    for(;;)
      ;
  }
  else {
    Serial.println("Ethernet configuration OK");
  }
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
  
  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    // read sensor data, replace with your code
    int sensorReading = readLightSensor();
    Serial.print("yeelink:");
    Serial.println(sensorReading);
    //send data to server
    sendData(sensorReading);
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}

// this method makes a HTTP connection to the server:
void sendData(int thisData) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print("POST /v1.0/device/");
    client.print(DEVICEID);
    client.print("/sensor/");
    client.print(SENSORID);
    client.print("/datapoints");
    client.println(" HTTP/1.1");
    client.println("Host: api.yeelink.net");
    client.print("Accept: *");
    client.print("/");
    client.println("*");
    client.print("U-ApiKey: ");
    client.println(APIKEY);
    client.print("Content-Length: ");

    // calculate the length of the sensor reading in bytes:
    // 8 bytes for {"value":} + number of digits of the data:
    int thisLength = 10 + getLength(thisData);
    client.println(thisLength);
    
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    client.print("{\"value\":");
    client.print(thisData);
    client.println("}");
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
   // note the time that the connection was made or attempted:
  lastConnectionTime = millis();
}


// This method calculates the number of digits in the
// sensor reading.  Since each digit of the ASCII decimal
// representation is a byte, the number of digits equals
// the number of bytes:
int getLength(int someValue) {
  // there's at least one byte:
  int digits = 1;
  // continually divide the value by ten, 
  // adding one to the digit count for each
  // time you divide, until you're at 0:
  int dividend = someValue /10;
  while (dividend > 0) {
    dividend = dividend /10;
    digits++;
  }
  // return the number of digits:
  return digits;
}

///////////////////////////////////////////////////////////////////////////
// get data from light sensor
// you can replace this code for your sensor 
int readLightSensor()
{
  uint16_t val=0;
  BH1750_Init(BH1750address);
  delay(200);
  if(2==BH1750_Read(BH1750address))
  {
    val=((buff[0]<<8)|buff[1])/1.2;
  }

  Serial.print("Sensor value is: ");
  Serial.println((int)val);
  
  return val;
}

int BH1750_Read(int address) //
{
  int i=0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();  
  return i;
}

void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);//1lx reolution 120ms
  Wire.endTransmission();
}
 

