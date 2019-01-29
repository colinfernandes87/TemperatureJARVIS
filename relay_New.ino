#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


ESP8266WebServer server(255);    // Create a webserver object that listens for HTTP request on port 255


const char* ssid     = "TIM-29404227";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* password = "TYf4DYqTAJ9WlomglG5XgvYd";     // The password of the Wi-Fi network

const int pin = 2;

void handleRoot();              // function prototypes for HTTP handlers
void handleLED();
void handleNotFound();

void setup(void){
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  WiFi.mode(WIFI_STA);
  pinMode(pin, OUTPUT);

  WiFi.begin(ssid, password);             // Connect to the network
  
  IPAddress ip(192,168,1,254);   
  IPAddress gateway(192,168,1,1);   
  IPAddress subnet(255,255,255,0);   
  WiFi.config(ip, gateway, subnet);

  delay(5000);
    
  while (WiFi.status() != WL_CONNECTED) 
  {
  delay(500);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");
  
  // Print the IP address
  Serial.print("Enter this URL to control ESP8266: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println(":255"); //"/255" is the Server Port. 

  server.on("/",  handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/RELAY",  handleRELAY);  // Call the 'handleRELAY' function when a POST request is made to URI "/RELAY"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();                    // Listen for HTTP requests from clients
}

void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/test", "Greetings from J.A.R.V.I.S");
}

void handleRELAY() {                          // If a POST request is made to URI /LED
  String t = server.arg("relay");
  Serial.println("-value for relay"+String(t));               
  if (t == "1")  {                            // Change the state of the Relay
    digitalWrite(pin, HIGH);
    server.send(200, "text/test", "Relay is OFF");
  }
  else {
    digitalWrite(pin, LOW);
      server.send(200, "text/test", "Relay is ON");
  }
  server.sendHeader("relay","/");           // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleNotFound(){
  server.send(404, "text/plain", "Well!! I am not a Search Engine"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
