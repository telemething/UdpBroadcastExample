/*
  File: UdpBroadcastExample.ino
  By: Mark West
  Date: July 11 2020
  License: MIT. See license file for more information but you can basically do 
  whatever you want with this code.
  Description: Arduino demo of how to send and receive UDP broadcast messages. 
  Tested on ESP32.
*/

#include <WiFi.h>
#include <WiFiUdp.h>

//WiFi network name and password:
const char * networkName = "---";
const char * networkPswd = "---";

//IP Address ports:
const char* udpOutAddress = "192.168.1.255";
const int udpOutPort = 45679;
const char* udpInAddress = "192.168.1.255";
const int udpInPort = 45678;

//Are we currently connected?
boolean connected = false;

// In buffer
const unsigned int inDataBufferSize = 1024;
uint8_t inBuffer[inDataBufferSize];
const unsigned int outDataBufferSize = 1024;
char outBuffer[outDataBufferSize];

//UDP Connections
WiFiUDP udpOut;
WiFiUDP udpIn;

const char* formatPrefix = "{\"Type\":\"telem\",\"From\":\"%s\",\"To\":\"%s\",\"Time\":%lu";

char* fromId = "self";
char* toId = "*";

//Simple local timer
unsigned long _lastSendTime = 0; 
//Once per second
unsigned long _timeBetweenSendMs = 1000; 

//*****************************************************************************
//
//*****************************************************************************
void connectToWiFi(const char * ssid, const char * pwd)
{
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//*****************************************************************************
//
//*****************************************************************************
void WiFiEvent(WiFiEvent_t event)
{
    switch(event)
	{
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
		  udpOut.begin(WiFi.localIP(), udpOutPort);
		  udpIn.begin(WiFi.localIP(), udpInPort);

		  connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}

//*****************************************************************************
//
//*****************************************************************************
void setup()
{
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println("UdpBroadcastExample");

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

//*****************************************************************************
// Build a JSON string
//*****************************************************************************
void buildOutString(char* buffer)
{
	sprintf(outBuffer, formatPrefix, fromId, toId, millis());
    strcat(outBuffer, "}");
}

//*****************************************************************************
//
//*****************************************************************************
void loop()
{
    if (connected)
    {
        // throttle send frequency
        if (millis() - _lastSendTime > _timeBetweenSendMs)
        {
            _lastSendTime = millis(); //Update the timer

            //build and display payload
            buildOutString(outBuffer);
            Serial.println(outBuffer);

            //Send a packet
            udpOut.beginPacket(udpOutAddress, udpOutPort);
            udpOut.print(outBuffer);
            udpOut.endPacket();
        }

        // if data is available, read a packet
        int packetSize = udpIn.parsePacket();
        if (packetSize)
        {
            Serial.print("> UDP read size: ");
            Serial.print(packetSize);
            Serial.print(", From: ");
            IPAddress remote = udpIn.remoteIP();

            for (int i = 0; i < 4; i++)
            {
                Serial.print(remote[i], DEC);
                if (i < 3)
                {
                    Serial.print(".");
                }
            }

            Serial.print(":");
            Serial.println(udpIn.remotePort());

            // read the packet into packetBufffer
            int readLen = udpIn.read(inBuffer, inDataBufferSize);
            //Serial.println("Contents:");
            //Serial.println(inBuffer);

            //******************
            Serial.print("--- UDP Read Len: ");
            Serial.println(readLen);
            //*******************
        }
    }
}

