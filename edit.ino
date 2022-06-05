// Import required libraries
//#include <Arduino.h>
#include <ESP8266WiFi.h> //ba3araf el board bta3ty
//#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>//webserver library
#include <Adafruit_Sensor.h>//water sensor library
#include <DHT.h> // dht library

#define DHTPIN 5             // Digital pin D1 connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)


//Network credentials
const char* ssid = "AH";
const char* password = "family1998";


DHT dht(DHTPIN, DHTTYPE); //3arafna el dht 3ashan lama anady 3aleha


float t = 0.0;
float h = 0.0;
float s = 0.0;
float data;
//maping lel water level
int map_low = 1024;
int map_high = 200;

//ba-connect el webserver f port 80
AsyncWebServer server(80);           // Generally, you should use "unsigned long" for variables that hold time
                                     // The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated
const long interval = 10000;         // Updates DHT readings every 10 seconds

//webserver code here
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">°C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">~</sup>
  </p>
  <p>
    <i class="fa fa-leaf" style="color:#00add6;"></i> 
    <span class="dht-labels">Soil</span>
    <span id="soil">%SOIL%</span>
    <sup class="units">%</sup>
  </p>

</body>
<script>

//Exchange data with server and update parts in the webpage without reloading.
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest(); //Creating an XMLHttpRequest object
  //defines a function to be called when the readyState property
  xhttp.onreadystatechange = function() { 
    //readyState XMLHttpRequest; 4: Request finished and response is ready
    //Returns the status of a request, 200: "OK"
    if (this.readyState == 4 && this.status == 200) {
      //haydawar 3ala tag fi el html leeh Id esmo "temperature" we ba3deen lama yela2eeh haye2dar ye3adel feeh be eno haya5od 
      //el value el tel3et men el temperature sensor we hay-replace el text el gowa el tag bel value el tel3et el hya kanet %TEMPERATURE%
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  //specify request, method: the request type (get or post), url: file location, async: TRUE(asynchronous) False(Synchronous)
  xhttp.open("GET", "/temperature", true); 
  // sends the request to the server and it's used for GET requests
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("soil").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/soil", true);
  xhttp.send();
}, 10000 ) ;

</script>
</html>)rawliteral";


// Replaces placeholder with DHT values
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "SOIL"){
    return String(s);
  }
  return String();
}


void setup(){
  pinMode(4, OUTPUT);
  pinMode(17, INPUT);
  
  Serial.begin(115200); // Serial port for debugging purposes
  dht.begin();
 
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  //hena bey-load el webserver ka home page el hya el index_html ka awel step fi running el webserver
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){  
    request->send_P(200, "text/html", index_html, processor);
  });
  //beyeb3at fi section el temperature fi el webserver value el temperature momasal fi string
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  //beyeb3at fi section el humidity fi el webserver value el humidity momasal fi string
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  //beyeb3at fi section el soil fi el webserver value el soil momasal fi string
  server.on("/soil", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(s).c_str());
  });

  // Start server
  server.begin();
  Serial.println("Web server Started!");
}

 
void loop(){  
    unsigned long currentMillis = millis();//3ashan el time
    //3ashan a3raf el time el et2as fy el 7aga mazbout
    if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;      // save the last time you updated the DHT values
    float newT = dht.readTemperature();  // Read temperature as Celsius (the default)

    //returns true if newT isn't a number
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.print("Temperature: ");
      Serial.println(t);
    }


 // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.print("Humidity: ");
      Serial.println(h);
    }

    
    // Read Soil
    //unlike dht, ther is no function that gets the values of the soil automatically
    data = analogRead(A0); //So we read the values then map the input from 0 to 100
    float newS = map(data, map_low, map_high, 0, 100);
    
    if (newS>=40){
      Serial.println("Soil is good");
      digitalWrite(4,LOW);
      }
    else{
      Serial.println("Water the soil");
      digitalWrite(4,HIGH);}
      s = newS;
      Serial.print("Soil: ");
      Serial.println(s);

    Serial.println(".");
    delay(1000);
  }
}
