// ************************************************************************************************************************
// *** Washing machine sketch
// ***
// *** To detect the end of my old washing machine cycle, and then send an email and beeps. On the future may activate
// *** the washing machine with a servomotor through internet. (Create WebServer, Send eMail, Sense Light, Activate Servomotor)
// ***
// *** TTD: Check the LDR once every 5 minutes, and sleep after sending the mail to save battery. Send mail if battery is low
// *** Calculate the energy cost of sleeping and awakening inbetween sensor readings
// ***
// *** I will use the ESP8266 (with the Lolin NodeMCU V3)
// ***  
// *** In-Pin A0 (A0)       - Uses an LDR to check the blinking light of end of cicle
// *** In-Pin GPIO000 (D2)  - Uses a mercury vibration switch to check the end of centrifugation
// *** Out-Pin GPIO016 (D0) - Servomotor
// *** Out-Pin Beeper?
// *** 
// *** D0 ---(Servomotor)
// *** D2 ---(Vibration sw)---|(gnd)
// *** D3 ---(10kOhm)---------|(3V)
// *** A0 ---(LDR)------------|(3V)
// ***   \---(10KOhm)---------|(gnd)
// ***
// *** My board of NodeMCU loses the programation after a Power cycle. This is fixed connecting GPIO0(D3-FLASH) 
// *** with a 10kOhm resistor to 3V. As per https://github.com/esp8266/Arduino/blob/master/doc/boards.md#boot-messages-and-modes.
// ***
// *** Created at 07 of June 2018
// *** By Celatzur
// *** Modified at 21 of December 2018
// *** By Celatzur
// *** 
// *** https://github.com/celatzur/Washing-Machine
// *** 
// *** To run this board the first time, on the Arduino IDE: 
// *** 1) File>Preferences>“Additional Boards Manager URLs:”>“http://arduino.esp8266.com/stable/package_esp8266com_index.json“
// *** 2) Tools>Board>NodeMCU 1.0 (ESP-12E Module).
// *** 3) Tools>Board>Board Manager... "esp8266 by ESP8266 Community">Install
// *** 4) Tools>Board>NodeMCU 1.0 (ESP-12E Module). Upload Speed: "115200".
// *** 
// ************************************************************************************************************************

#include <ESP8266WiFi.h> //Downloadable from: http://arduino.esp8266.com/stable/package_esp8266com_index.json
#include <rBase64.h>  //rBase64 by Abhijit Bose. Helps to Encode and Decode in BASE64 form using simple String operations.
#include <Servo.h>

// **** Email and Http server definitions
const char* ssid = "your-sidd";             // WIFI network name
const char* password = "your-password";     // WIFI network password
const char* user_base64 = "yourEmail@gmail.com";
const char* user_password_base64 = "yourEmail_password";
const char* from_email = "MAIL From:<FromEmail@gmail.com>";
const char* to_email = "RCPT To:<yourEmail@gmail.com>"; 
const char* SMTP_server = "smtp.mail.gmail.com";                              // SMTP server address smtp.gmail.com
char* mail_subject = "Subject: The washing machine has just finished\r\n";    // email subject
const int SMTP_port = 465;                  // 587
uint8_t connection_state = 0;               // Connected to WIFI or not
uint16_t reconnect_interval = 10000;        // If not connected wait time to try again
WiFiServer server(80);                      //WiFiServer server(301);
char message_content[50];                   // Content of the body of the mail
const char* host = "192.168.*.***";         // The serial port will tell you the IP once it starts up
                                            // just write it here afterwards and upload
// **** Sensors definitions
int switch_pin = 0;           // GPIO00 = D3 Definition of mercury tilt switch sensor interface
int switch_val;               // Defines a numeric variable for the switch
const int LDR = A0;           // Defining LDR PIN 
int LDR_val = 0;              // Varible to store LDR values
int LDR_threshold_val = 500;  // Threshold for the LDR, 700 for light, 300 for darkness
                              //int LED_pin = 16;             // GPIO16 = D0 Correspondance between arduino and LoLin pins
int LED_pin = 2;              //#define D4 2 // Same as "LED_BUILTIN", but inverted logic
int sample_freq_ms = 175;     // My WM blinks at 0.52 blinks/s (520ms->1.92KHz). We sample at three time this frequency
                              
// **** Servomotor definitions
int servoStart_pin = 1;        // Servomotor to start the washing-machine
Servo servoStart;
int ServoOffPosition = 90;
int ServoOnPosition = 180;

// ************************************************************************************************************************
// *** Setup
// ************************************************************************************************************************
void setup() {
  
  Serial.begin(115200);       //Baud rate for serial port communication
  delay(10);

  Serial.print("Setup...");

  //Output LED
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);

  //LDR sensor as input
  pinMode(LDR_pin, INPUT);
  
  //Mercury switch as input
  pinMode(switch_pin, INPUT);
 
  //Servomotor to start the washing-machine
  servoStart.attach(servoStart_pin);
  servoStart.write(ServoOffPosition);
  
  //Connect to Wifi network, starts the server and prints the IP address
  setupWifi();

  //Send a mail informing of the reboot
  if (sendEmail("Board Reboot"))  {
    Serial.println(F("Email sent"));
  }
  else  {
    Serial.println(F("Email failed"));
  }
}

// ************************************************************************************************************************
// *** Main loop: waits to the LED to be blinkning, checks the vibration and send a beep and a mail
// ************************************************************************************************************************
void loop() {

  httpServer(); //Creates and uses an http server to change the state of the led over a web page
  LDRSensor(); //Read the values of the LDR and sends them over serial port
  //vibrationSwitch(); // Reads the value of the mercury switch and lights the led
}

// ************************************************************************************************************************
// *** Connect to Wifi network, starts the server and prints the IP address on the serial port
// ************************************************************************************************************************
void setupWifi() {
    
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address on the serial port
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  }

// ************************************************************************************************************************
// *** Reads the value of the mercury switch and lights the led
// ************************************************************************************************************************
void vibrationSwitch(){

  switch_val = digitalRead(switch_pin); // check mercury switch state
  if(switch_val == HIGH) {
    digitalWrite(LED_pin, HIGH);
    }
  else {
    digitalWrite(LED_pin, LOW);
    }
  delay(1000); //Arbitrari, for the bouncing
}

// ************************************************************************************************************************
// *** Read the values of the LDR and sends them over serial port
// ************************************************************************************************************************
int LDRSensor(){
   //LDR constantly High for near end of the process. Blinking when really finished
  
   LDR_val = analogRead(LDR);      // Reading Input

  // If the Light is OFF, whait one minute and check again  
   while (LDR_val > LDR_threshold_val ) {
    for (int i=0; i<60; i++){
      delay(1000);
      }
      LDR_val = analogRead(LDR);      // Reading Input
    }

// Method 1: If we know the original frequency, apply Nyquist sampling theorem
// The Washing Machine blinks at 0.52 blinks/second. (520ms)
//By Nyquist Teorem we should sample at double frequency, in our case four times thre frequency is enough
int sample_freq_ms = 150; 

// Method 2: Sample at random intervals, hoping to catch different levels
  //Now the Light is ON, check if the light is OFF five times in case we check when it's blinking
  for (int i=0; i<5; i++){
    if (LDR_val < LDR_threshold_val) {
      return(1);//Returns High, meaning, the LED is blinking
    }
    int time=random(500)*i; //Randomize the lectures
    delay(time);
    LDR_val = analogRead(LDR);      // Reading Input
  }
   
   //Serial.print("LDR value is : " );                        
   //Serial.println(LDR_val);        // Writing input on serial monitor.
   //delay(1000); 
}

// ************************************************************************************************************************
// *** Creates and uses an http server to change the state of the led over a web page
// ************************************************************************************************************************
void httpServer(){

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  
  // Match the request
   int value = LOW;

//if (req.indexOf("") != -10) {  //checks if you're on the main page

  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(LED_pin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(LED_pin, LOW);
    value = LOW;
  }

// Set LED_pin according to the request
//digitalWrite(LED_pin, value);
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("<head><title>");
  client.print("Marc's Washing Machine");
  client.print("</title>");
  client.print("<style>body{background-color:black;text-align:center;color:white;}</style>");
  client.print("</head>");
  
  client.println("<body><h1>Marc's Washing Machine <br>Web Server</h1>");
  client.println("<a href=\"/START=ON\"\"><button>Start Washing</button></a><br />");  
  
  client.print("<br /> Whashing Machine is now: ");
 
  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
}

/* Next functions are from the basic ESP8266 HelloServer example*/

void handleRoot() {
  server.send(200, "text/plain", "Hello from Washing Machine!");
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
