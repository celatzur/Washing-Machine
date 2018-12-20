// ************************************************************************************************************************
// *** Washing machine sketch
// ***
// *** To detect the end of my old washing machine cycle, and then send an email and beeps. On the future may activate
// *** the washing machine with a servomotor through internet.
// ***
// *** I will use the ESP8266 (with the Lolin NodeMCU V3)
// ***  
// *** In-Pin A0 (A0)       - Uses an LDR to check the blinking light of end of cicle
// *** In-Pin GPIO000 (D3)  - Uses a mercury vibration switch to check the end of centrifugation
// *** Out-Pin GPIO016 (D0) - LED to indicate change of state
// *** Out-Pin Beeper?
// *** Servomotor? 
// ***
// *** Created at 07 of June 2018
// *** By Celatzur
// *** Modified at 23 of September 2018
// *** By Celatzur
// *** 
// *** https://github.com/celatzur/Washing-Machine
// *** 
// ************************************************************************************************************************

// *** NodeMCU to Arduino GPIO pin correspondance mapping from pins_arduino.h
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;  // I2C Bus SCL (clock)
static const uint8_t D2   = 4;  // I2C Bus SDA (data)
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;  // Same as "LED_BUILTIN", but inverted logic
static const uint8_t D5   = 14; // SPI Bus SCK (clock)
static const uint8_t D6   = 12; // SPI Bus MISO 
static const uint8_t D7   = 13; // SPI Bus MOSI
static const uint8_t D8   = 15; // SPI Bus SS (CS)
static const uint8_t D9   = 3;  // RX0 (Serial console)
static const uint8_t D10  = 1;  // TX0 (Serial console)

static const uint8_t D11 = 9;
static const uint8_t SD2 = 9;
  
static const uint8_t D12 = 10;
static const uint8_t SD3 = 10;

#include <ESP8266WiFi.h>

// Erase my pwd and SSID before publishing !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 
const char* ssid = "****";
const char* password = "****";
const char* host = "192.168.*.***"; //The serial port will tell you the IP once it starts up
                                    //just write it here afterwards and upload
 
int LED_pin = 16;             // GPIO16 = D0 Correspondance between arduino and LoLin pins

int switch_pin = 0;           // GPIO00 = D3 Definition of mercury tilt switch sensor interface
int switch_val;               // Defines a numeric variable for the switch

const int LDR = A0;           // Defining LDR PIN 
int LDR_val = 0;              // Varible to store LDR values
int LDR_threshold_val = 500;  // Threshold for the LDR, 700 for light, 300 for darkness

//My Washing Machine blinks at 0.52 blinks/second. (520ms)
int sample_freq_ms = 150;     //By Nyquist Teorem we should sample at least at double the frequency, in our case four times 
                              //the frequency is enough
////WiFiServer server(80);
WiFiServer server(301);
 
void setup() {
  Serial.begin(115200);       //Baud ratefo0r serial port communication
  delay(10);

  //Output LED
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);

  //Mercury switch as input
  pinMode(switch_pin, INPUT);
 
  //Connect to Wifi network, starts the server and prints the IP address
  setupWifi();
}

// ************************************************************************************************************************
// *** Main loop: waits to the LED to be blinkning, checks the vibration and send a beep and a mail
// ************************************************************************************************************************
void loop() {

  //httpServer(); //Creates and uses an http server to change the state of the led over a web page
  LDRSensor(); //Read the values of the LDR and sends them over serial port
  vibrationSwitch(); // Reads the value of the mercury switch and lights the led
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
int sample_freq_ms = 150; //By Nyquist Teorem we should sample at double frequency, in our case four times thre frequency is enough

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
 
  client.print("Led pin is now: ");
 
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
  Serial.println("Client disonnected");
  Serial.println("");
}
