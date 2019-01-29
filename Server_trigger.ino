/*
Server code to get temperature and humidity from clients and trigger the relay
*/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

// IoT platform Credentials
String apiKey = "JO6TA09TG1OMZIYV";
const char* logServer = "api.thingspeak.com";

// Internet router credentials
const char* ssid = "ESP_Sensor_NTW";
const char* password = "espsensorntw";

// Relay ESP web-server address
int temp = 28;
String serverHost = "http://192.168.1.254:255/RELAY";
String data;
int relay=0;
String webString="";     // String to display
ESP8266WebServer server(80);

// Buffer for latest values
String btemp1 = "";
String bhum1 = "";
String btemp2 = "";
String bhum2 = "";



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  setupAccessPoint();
}
// Handling the / root web page from my server
void handle_index() {
  webString="Temperature Sensor 1: "+String(btemp1)+" C\n";   // Arduino has a hard time with float to string
  webString+="Humidity Sensor 1: "+String(bhum1)+"%\n";
  webString+="Temperature Sensor 2: "+String(btemp2)+" C\n";   // Arduino has a hard time with float to string
  webString+="Humidity Sensor 2: "+String(bhum2)+"%";
  server.send(200, "text/plain", webString);  
  //server.send(200, "text/plain", "Welcome to J.A.R.V.I.S : Just A Rather Very Intelligent System");
}

// Handling the /feed page from my server
void handle_feed() {
  String t1 = server.arg("temp1");
  String h1 = server.arg("hum1");
  String t2 = server.arg("temp2");
  String h2 = server.arg("hum2");

  btemp1 = t1;
  bhum1 = h1;
  btemp2 = t2;
  bhum2 = h2;

  
  server.send(200, "text/plain", "J.A.R.V.I.S acknowledges the client");
  setupStMode(t1, h1, t2 ,h2);
}

void setupAccessPoint(){
  Serial.println("** SETUP ACCESS POINT **");
  Serial.println("- disconnect from any other modes");
  WiFi.disconnect();
  Serial.println("- start ap with SID: "+ String(ssid));
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("- AP IP address is :");
  Serial.print(myIP);
  setupServer();
}

void setupServer(){
  Serial.println("** SETUP SERVER **");
  Serial.println("- starting server :");
  server.on("/", handle_index);
  server.on("/feed", handle_feed);
  server.begin();
};

void setupStMode(String t1, String v1,String t2, String v2){
  Serial.println("** SETUP STATION MODE **");
  Serial.println("- disconnect from any other modes");
  WiFi.disconnect();
  Serial.println();
  Serial.println("- connecting to Home Router SID: **********");
  WiFi.begin("TIM-29404227", "TYf4DYqTAJ9WlomglG5XgvYd");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("- succesfully connected");
  Serial.println("- starting client");
  
  WiFiClient client;

  Serial.println("- connecting to Database server: " + String(logServer));
  if (client.connect(logServer, 80)) {
    Serial.println("- succesfully connected");
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t1);
    postStr += "&field2=";
    postStr += String(v1);
    postStr += "&field3=";
    postStr += String(t2);
    postStr += "&field4=";
    postStr += String(v2);
    postStr += "\r\n\r\n";
    Serial.println("- sending data...");
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  Serial.println("- stopping the client");
  Serial.println("- check if triger is required");
  Serial.println(t1);
  btemp1.trim();
  int n = btemp1.toInt();
  Serial.println(n);
  triggerRequest(n);
  /** If your ESP does not respond you can just
  *** reset after each request sending 
  Serial.println("- trying reset");
  ESP.reset();
  **/
}


void loop() {
  server.handleClient();
}




void triggerRequest(int n) {
  if (n != 0){
    if (n >= temp && relay == 0) {
      Serial.println("Turning on the relay..");
      relay =1; 
      }
    else if(n < temp && relay == 1) {
        relay =0;
        Serial.println("Turning off the relay.."); 
      }
    else{
      return;
    }
  Serial.println(n);
  Serial.println(relay);
  Serial.println("- build DATA stream string");
  buildDataStream(relay);
  Serial.println("- send GET request");
  sendHttpRequest();
  }
}

void sendHttpRequest() {
  HTTPClient http;
  http.begin(serverHost);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  Serial.println(data);
  http.POST(data);
  http.writeToStream(&Serial);
  http.end();
}

void buildDataStream(int r) {
  data = "relay=";
  data += String(r);
  Serial.println("- data stream: "+data);
}
