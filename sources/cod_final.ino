#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>  

// Definire pini
#define DHTPIN 17
#define DHTTYPE DHT11
#define PROXIMITY_SENSOR_PIN 15
#define TRIG_PIN 26
#define ECHO_PIN 27
#define PIR_SENSOR_PIN 16

#define SD_CS_PIN 5  

// Conectare WiFi
const char* ssid = "DIGI-g4WP";
const char* password = "4t2MC8wK";

// Pini LED-uri
const int output1 = 32; // R
const int output2 = 33; // G
const int output3 = 25; // B
const int output4 = 4;  // Yellow
const int output5 = 0;  // Red

// Variabile slider
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";
String sliderValue4 = "0";
String sliderValue5 = "0";

// Variabile control mode
String controlMode = "auto";

const char* PARAM_INPUT_1 = "value1";
const char* PARAM_INPUT_2 = "value2";
const char* PARAM_INPUT_3 = "value3";
const char* PARAM_INPUT_4 = "value4";
const char* PARAM_INPUT_5 = "value5";

// Creare obiecte
DHT dht(DHTPIN, DHTTYPE);
RTC_PCF8523 rtc;
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP Web Server</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; }
    .button { padding: 10px 20px; font-size: 1.2rem; cursor: pointer; }

    .alert {
    padding: 20px;
    background-color: #f44336;
    color: white;
    }

    .closebtn {
    margin-left: 15px;
    color: white;
    font-weight: bold;
    float: right;
    font-size: 22px;
    line-height: 20px;
    cursor: pointer;
    transition: 0.3s;
    }

    .closebtn:hover {
      color: black;
    }

  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  <p>Current Time: <span id="time">%TIME%</span></p>
  <p>Temperature: <span id="temperature">%TEMPERATURE%</span> &deg;C</p>
  <p>Humidity: <span id="humidity">%HUMIDITY%</span> &#37;</p>

  <div class="alert" id="alertLivingRoom">
    <span class="closebtn" onclick="this.parentElement.style.display='none';" >&times;</span> 
    <strong>Alert!</strong> Motion detected in the living room.
  </div>

  <div class="alert" id="alertBathroom">
    <span class="closebtn" onclick="this.parentElement.style.display='none';" >&times;</span> 
    <strong>Alert!</strong> Motion detected in the bathroom.
  </div>

  <div class="alert" id="alertBedroom">
    <span class="closebtn" onclick="this.parentElement.style.display='none';" >&times;</span> 
    <strong>Alert!</strong> Motion detected in the bedroom.
  </div>

  <button class="button" onclick="toggleMode()">Toggle Control Mode: <span id="mode">%MODE%</span></button>
  
  <p>LED R Value: <span id="textSliderValue1">%SLIDERVALUE1%</span></p>
  <p><input type="range" onchange="updateSliderPWM1(this)" id="pwmSlider1" min="0" max="255" value="%SLIDERVALUE1%" step="1" class="slider"></p>
  <p>LED G Value: <span id="textSliderValue2">%SLIDERVALUE2%</span></p>
  <p><input type="range" onchange="updateSliderPWM2(this)" id="pwmSlider2" min="0" max="255" value="%SLIDERVALUE2%" step="1" class="slider"></p>
  <p>LED B Value: <span id="textSliderValue3">%SLIDERVALUE3%</span></p>
  <p><input type="range" onchange="updateSliderPWM3(this)" id="pwmSlider3" min="0" max="255" value="%SLIDERVALUE3%" step="1" class="slider"></p>
  <p>LED Yellow  Value: <span id="textSliderValue4">%SLIDERVALUE4%</span></p>
  <p><input type="range" onchange="updateSliderPWM4(this)" id="pwmSlider4" min="0" max="255" value="%SLIDERVALUE4%" step="1" class="slider"></p>
  <p>LED Red Value: <span id="textSliderValue5">%SLIDERVALUE5%</span></p>
  <p><input type="range" onchange="updateSliderPWM5(this)" id="pwmSlider5" min="0" max="255" value="%SLIDERVALUE5%" step="1" class="slider"></p>

<script>
  function checkSensors() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/getMode", true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200) {
        var controlMode = xhr.responseText;
        var motionLivingRoom = digitalRead(PROXIMITY_SENSOR_PIN); // Simulează valoarea senzorului
        var motionBathroom = digitalRead(PIR_SENSOR_PIN);         // Simulează valoarea senzorului
        var motionBedroom = measureDistance();                    // Simulează valoarea senzorului

        if (controlMode == "auto") {
          document.getElementById('alertLivingRoom').style.display = (motionLivingRoom == HIGH) ? 'block' : 'none';
          document.getElementById('alertBathroom').style.display = (motionBathroom == HIGH) ? 'block' : 'none';
          document.getElementById('alertBedroom').style.display = (motionBedroom <= 20) ? 'block' : 'none';
        } else {
          document.getElementById('alertLivingRoom').style.display = 'none';
          document.getElementById('alertBathroom').style.display = 'none';
          document.getElementById('alertBedroom').style.display = 'none';
        }
      }
    };
    xhr.send();
  }

  function updateSliderPWM1(element) {
    var sliderValue = document.getElementById("pwmSlider1").value;
    document.getElementById("textSliderValue1").innerHTML = sliderValue;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider1?value1="+sliderValue, true);
    xhr.send();
  }

  function updateSliderPWM2(element) {
    var sliderValue = document.getElementById("pwmSlider2").value;
    document.getElementById("textSliderValue2").innerHTML = sliderValue;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider2?value2="+sliderValue, true);
    xhr.send();
  }

  function updateSliderPWM3(element) {
    var sliderValue = document.getElementById("pwmSlider3").value;
    document.getElementById("textSliderValue3").innerHTML = sliderValue;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider3?value3="+sliderValue, true);
    xhr.send();
  }

  function updateSliderPWM4(element) {
    var sliderValue = document.getElementById("pwmSlider4").value;
    document.getElementById("textSliderValue4").innerHTML = sliderValue;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider4?value4="+sliderValue, true);
    xhr.send();
  }

  function updateSliderPWM5(element) {
    var sliderValue = document.getElementById("pwmSlider5").value;
    document.getElementById("textSliderValue5").innerHTML = sliderValue;
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/slider5?value5="+sliderValue, true);
    xhr.send();
  }

  function toggleMode() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/toggleMode", true);
    xhr.onreadystatechange = function() {
      if (xhr.readyState == 4 && xhr.status == 200) {
        // Actualizează textul modului după toggle
        var modeElement = document.getElementById("mode");
        if (modeElement.innerHTML == "auto") {
          modeElement.innerHTML = "manual";
        } else {
          modeElement.innerHTML = "auto";
        }
      }
    };
    xhr.send();
  }

  // Refresh la fiecare 3 secunde fără a afecta starea butonului de toggle
  setInterval(function() {
    location.reload();
  }, 3000);
</script>
</body>
</html>
)rawliteral";

// Funcție pentru înlocuirea variabilelor în HTML
String processor(const String& var){
  if (var == "SLIDERVALUE1"){
    return sliderValue1;
  }
  if (var == "SLIDERVALUE2"){
    return sliderValue2;
  }
  if (var == "SLIDERVALUE3"){
    return sliderValue3;
  }
  if (var == "SLIDERVALUE4"){
    return sliderValue4;
  }
  if (var == "SLIDERVALUE5"){
    return sliderValue5;
  }
  if (var == "TIME") {
    DateTime now = rtc.now();
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    return String(timeStr);
  }
  if (var == "TEMPERATURE") {
    float t = dht.readTemperature();
    if (isnan(t)) {
      return "--";
    } else {
      return String(t);
    }
  }
  if (var == "HUMIDITY") {
    float h = dht.readHumidity();
    if (isnan(h)) {
      return "--";
    } else {
      return String(h);
    }
  }
  if (var == "MODE") {
    return controlMode;
  }
  return String();
}

// Funcție pentru măsurarea distanței cu senzorul ultrasonic
long measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration / 2) / 29.1; // Conversie la centimetri
  return distance;
}

/*
// Funcție pentru salvarea alertelor în fișiere pe cardul SD
void saveAlert(const String& alertMessage) {
  // Obține ora curentă de la RTC
  DateTime now = rtc.now();
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  // Combină ora și mesajul de alertă într-un singur șir
  String fullMessage = String(timeStr) + " - " + alertMessage;

  // Deschide fișierul și scrie mesajul complet
  File file = SD.open("/alerts.txt", FILE_APPEND);
  if (file) {
    file.println(fullMessage); // Scrie mesajul complet pe o singură linie
    file.close();
    Serial.println("Alert saved to SD card: " + fullMessage);
  } else {
    Serial.println("Error opening file on SD card.");
  }
}
*/

void setup(){
  // Initialize serial
  Serial.begin(115200);


/*
  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");
*/
  // Initialize senzori
  pinMode(PROXIMITY_SENSOR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_SENSOR_PIN, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  // Initialize DHT sensor
  dht.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle slider changes
  server.on("/slider1", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_1)) {
      sliderValue1 = request->getParam(PARAM_INPUT_1)->value();
      if (controlMode == "manual") {
        analogWrite(output1, sliderValue1.toInt());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/slider2", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_2)) {
      sliderValue2 = request->getParam(PARAM_INPUT_2)->value();
      if (controlMode == "manual") {
        analogWrite(output2, sliderValue2.toInt());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/slider3", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_3)) {
      sliderValue3 = request->getParam(PARAM_INPUT_3)->value();
      if (controlMode == "manual") {
        analogWrite(output3, sliderValue3.toInt());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/slider4", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_4)) {
      sliderValue4 = request->getParam(PARAM_INPUT_4)->value();
      if (controlMode == "manual") {
        analogWrite(output4, sliderValue4.toInt());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/slider5", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam(PARAM_INPUT_5)) {
      sliderValue5 = request->getParam(PARAM_INPUT_5)->value();
      if (controlMode == "manual") {
        analogWrite(output5, sliderValue5.toInt());
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // Handle mode toggle
  server.on("/toggleMode", HTTP_GET, [] (AsyncWebServerRequest *request) {
    controlMode = (controlMode == "auto") ? "manual" : "auto";
    request->send(200, "text/plain", "OK");
  });

  server.on("/getMode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", controlMode);
  });

  // Start server
  server.begin();
}

void loop() {
  if (controlMode == "auto") {
    DateTime now = rtc.now();
    int currentHour = now.hour();
    int intensity = 0;

    // Setare intensitate lumini în funcție de intervalul orar
    if (currentHour >= 7 && currentHour < 16) {
      intensity = 64; // Între orele 7:00 și 16:00
    } else if (currentHour >= 16 && currentHour < 21) {
      intensity = 128; // Între orele 16:00 și 21:00
    } else {
      intensity = 0; // Între orele 21:00 și 7:00
    }

    // Automatizare LED-uri bazată pe senzori și intervale de timp
    if (digitalRead(PROXIMITY_SENSOR_PIN) == HIGH) {
      analogWrite(output1, 0);   // Red LED off
      analogWrite(output2, 255); // Green LED off
      analogWrite(output3, 0);   // Blue LED on
      Serial.println("Miscare in dormitor");
      //saveAlert("Motion detected in the bedroom");
    } else {
      analogWrite(output1, intensity);  // Red LED
      analogWrite(output2, intensity);  // Green LED
      analogWrite(output3, intensity);  // Blue LED
    }

    long distance = measureDistance();
    if (distance <= 10) {
      analogWrite(output5, 255);  // Red LED on
      Serial.println("Miscare in sufragerie");
      //saveAlert("Motion detected in the living room");
    } else {
      analogWrite(output5, intensity);  // Control LED based on interval
    }

    if (digitalRead(PIR_SENSOR_PIN) == HIGH) {
      analogWrite(output4, 255);  // Yellow LED on
      Serial.println("Miscare in baie");
     // saveAlert("Motion detected in the bathroom");
    } else {
      analogWrite(output4, intensity);  // Control LED based on interval
    }
  }
}
