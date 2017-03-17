/*--------------------------------------------------------------
  NodeMCU AJAX Demo

  Description:  NodeMCU web server that displays analog input,
                the state of 5 switches and controls 2 outputs
                and 2 on-board LEDs 2 using checkboxes and 2 
                buttons. The web page is stored as file on 
                internal flash file system.
  
  Hardware:     NodeMCU board. Should work with other ESP8266
                boards.
                A0 Analog input
                D0 NodeMCU LED
                D1 Input
                D2 Input
                D3 Input. Pull-up
                D4 ESP-12 LED
                D5 Output
                D6 Output
                D7 Input
                D8 Input. Pull-down
                
  Software:     Developed using Arduino 1.8.1 software
                ESP8266/Arduino 2.3.0
                Should be compatible with Arduino 1.0 + and
                newer ESP8266/Arduino releases if any
                Internal flash File system should contain web
                page called /index.htm. Use ESP8266FS Tool
                to upload File system contents via Arduino IDE.
  
  References:   - WebServer example by David A. Mellis and 
                  modified by Tom Igoe
                - ESP8266/Arduino bundeled examples
                
  Date:         17 March 2017
 
  Author:       a.m.emelianov@gmail.com
                https://gitbub.com/emelianov
--------------------------------------------------------------*/

#include <ESP8266WiFi.h>          // ESP8266 base Wi-Fi library 
#include <ESP8266WebServer.h>           // Implementation of standatd Arduino network routines
#include <FS.h>                   // SFIFFS local flash storge File system. Implements the same API as SD card library

#define SSID "SSID"               // Wi-Fi Access point SSID
#define PASSWORD "PASSWORD"       // Wi-Fi password

IPAddress ip(192, 168, 30, 129);  // IP address
IPAddress mask(255, 255, 255, 0); // Netmask
IPAddress gw(192, 168, 30, 4);    // Default gateway
IPAddress dns(192, 168, 30, 4);   // DNS
ESP8266WebServer server(80);            // create a server at port 80
boolean LED_state[4] = {0};       // stores the states of the LEDs


// checks if received HTTP request is switching on/off LEDs
// also saves the state of the LEDs
void SetLEDs(void)
{
    // LED 1 (pin D4)
    if (server.hasArg("LED1")) {
        LED_state[0] = server.arg("LED1").toInt();       // save LED state
        digitalWrite(D4, (LED_state[0]==1)?LOW:HIGH);    // For D4 and D0 use LOW output for 'on' state  
    }
    // LED 2 (pin D5)
    if (server.hasArg("LED2")) {
        LED_state[1] = server.arg("LED2").toInt();       // save LED state
        digitalWrite(D5, (LED_state[1]==1)?HIGH:LOW);    // For D5 and D6 use HIGH output for 'on' state  
    }
    // LED 3 (pin D6)
    if (server.hasArg("LED3")) {
        LED_state[2] = server.arg("LED3").toInt();       // save LED state
        digitalWrite(D6, (LED_state[2]==1)?HIGH:LOW);    // For D5 and D6 use HIGH output for 'on' state  
    }
    // LED 4 (pin D0)
    if (server.hasArg("LED4")) {
        LED_state[3] = server.arg("LED4").toInt();       // save LED state
        digitalWrite(D0, (LED_state[3]==1)?LOW:HIGH);    // For D4 and D0 use LOW output for 'on' state  
    }
}

// send the XML file with analog values, switch status
//  and LED status
String xmlResponse()
{
    String res = "";
    int analog_val;                     // stores value read from analog inputs
    int sw_arr[] = {D1, D2, D3, D7, D8};// pins interfaced to switches
    
    res += "<?xml version = \"1.0\" ?>\n";
    res +="<inputs>\n";
    // read analog input
    // ESP8266 has only one Analog input. It's 10bit (1024 values) Measuring range is 0-1V.
        analog_val = analogRead(A0);
        res += "<analog>";
        res += String(analog_val);
        res += "</analog>\n";
    // read switches
    for (int count = 0; count < sizeof(sw_arr)/sizeof(sw_arr[0]); count++) {
        res += "<switch>";
        if (digitalRead(sw_arr[count])) {
            res += "ON";
        }
        else {
            res += "OFF";
        }
        res += "</switch>";
    }
    // checkbox LED states
    // LED1
    res += "<LED>";
    if (LED_state[0]) {
        res += "checked";
    }
    else {
        res += "unchecked";
    }
    res += "</LED>\n";
    // LED2
    res += "<LED>";
    if (LED_state[1]) {
        res += "checked";
    }
    else {
        res += "unchecked";
    }
     res += "</LED>\n";
    // button LED states
    // LED3
    res += "<LED>";
    if (LED_state[2]) {
        res += "on";
    }
    else {
        res += "off";
    }
    res += "</LED>\n";
    // LED4
    res += "<LED>";
    if (LED_state[3]) {
        res += "on";
    }
    else {
        res += "off";
    }
    res += "</LED>\n";
    
    res += "</inputs>";
    return res;
}

void ajaxInputs() {
  SetLEDs();
  server.sendHeader("Connection", "close");
  server.sendHeader("Cache-Control", "no-store, must-revalidate");
  server.send(200, "text/xml", xmlResponse());
}
void indexFile() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-store, must-revalidate");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    File file = SPIFFS.open("/index.htm", "r");
    size_t sent = server.streamFile(file, "text/html");
    file.close();
}

void setup()
{
    
    Serial.begin(9600);           // for debugging
    
    // initialize File system
    Serial.println("Initializing File system...");
    if (!SPIFFS.begin()) {
        Serial.println("ERROR - File system initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - File system initialized.");
    // check for /index.htm file
    // Note each file must begins with slash.
    if (!SPIFFS.exists("/index.htm")) {
        Serial.println("ERROR - Can't find /index.htm file!");
        return;                 // can't find index file
    }
    Serial.println("SUCCESS - Found /index.htm file.");
    // switches on pins D1, D2, D3, D7 and D8
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    pinMode(D3, INPUT);         // Pull-Up. Flash button switches pin to LOW.
    pinMode(D7, INPUT);
    pinMode(D8, INPUT);         // Pull-Down
    // LEDs
    pinMode(D4, OUTPUT);        // ESP-12 LED
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D0, OUTPUT);        // NodeMCU LED
    // LEDs is on in LOW state that is default when we switching pin to OUTPUT. So we have to switch thay off.
    digitalWrite(D4, HIGH);
    digitalWrite(D0, HIGH);
  
    //WiFi.mode(WIFI_AP);       // Initialize Wi-Fi in AP mode. Node MCU will act as Access Point. So You can
                                // to it directly. Address of AP is 192.168.4.1. No password. AP is open.
    WiFi.mode(WIFI_STA);        // Initialize Wi-Fi in STAtion mode. NodeMCU will act as Wi-Fi Client.
    //WiFi.config(ip, gw, mask, dns); // Use static IP settings  
    WiFi.begin(SSID, PASSWORD); // Start connecting to AP with ssid and password
    Serial.print("Connecting Wi-Fi.");
    // Wait until connection succesfull
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
    Serial.println();
    Serial.print("SUCCESS - Local IP: ");
    Serial.println(WiFi.localIP());     // Prints IP address got
    // Set callback function to handle http requests with different URLs
    server.on("/ajax_inputs", HTTP_GET, ajaxInputs);  // call function ajaxInputs() if Web Server gets request http://192.168.1.20/ajax_inputs?LED1=0...
    server.onNotFound(indexFile);       // call function indexFile() on any other requests
    server.begin();                     // start to listen for clients
}

void loop()
{
    server.handleClient();              // Checks if incoming client connection, get data and query callback function corresponding to request URL
    delay(100);                         // Lets ESP networking routines to work. 
}

