#include "DHT.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);

float temp = 0.0;
float voltage = 0.0;
float rt = 0.0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
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
  <h2>Measurement Project</h2>
  <p>
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="dht-labels">Resistance</span> 
    <span id="resistance">%RESISTANCE%</span>
    <span>Ohms</sup>
  </p>
  <p>
    <span class="dht-labels">Voltage</span> 
    <span id="voltage">%VOLTAGE%</span>
    <span>V</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("resistance").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/resistance", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("voltage").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/voltage", true);
  xhttp.send();
}, 1000 ) ;

</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(temp-273.15);
  }
  if(var == "RESISTANCE"){
    return String(rt);
  }
  if(var == "VOLTAGE"){
    return String(voltage);
  }
  return String();
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temp-273.15).c_str());
  });
  server.on("/resistance", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(rt).c_str());
  });
  server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(voltage).c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  float vin = 3.28;
  // Steinhart-Hart Coefficients for the thermistor
  float A = 2.1083E-3;
  float B = 7.9586E-5;
  float C = 6.7213E-7;
  int sensorValue = analogRead(A0);  // Read the analog voltage
  voltage = sensorValue * (vin / 1023.0);
  
  rt = 9830*((vin/voltage)-1);

  temp = 1/(A+(B*log(rt)+(C*(pow(log(rt),3)))));
  Serial.print("Analog Value: ");
  Serial.print(sensorValue);
  Serial.print(", Voltage: ");
  Serial.println(voltage, 3);  // Print voltage with 3 decimal places
  Serial.println(rt, 3);
  Serial.println("Temp (K): ");
  Serial.println(temp, 3);
  Serial.println("Temp (C): ");
  Serial.println(temp-273.15, 3);
  
  delay(1000);  // Wait for a second before reading again
}
