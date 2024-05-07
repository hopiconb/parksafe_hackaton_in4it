#include <ESP8266WebServer.h>

const char* ssid = "Fernando";
const char* password = "fredy1997";

//Pinii pentru senzor
const int echoPin = D5; 
const int trigPin = D3;

//Pinii pentru led
const int red = D4;
const int blue = D1;
const int green = D2;

//Pinii pentru display-ul led
const int dataPin = D8;
const int latchPin = D7;
const int clockPin = D6;

int counter = 0;

int ip_x4;
int digit_1;
int digit_2;
int digit_3;

//declare a byte variable to store the current digits active
byte digit;

//declararea a doua variabile pentru determinarea distantei
long duration;
double distance;

ESP8266WebServer server(80);

String page =
R"(
<html>  
  <head> 
    <script src='https://code.jquery.com/jquery-3.3.1.min.js'></script>
    <title>Plusivo</title> 
  </head> 

  <body> 
    <h2>Hello from Plusivo!</h2> 
    <table style='font-size:20px'>  
      <tr>  
        <td> 
          <div>Distance:  </div>
        </td>
        <td> 
          <div id='Distance'></div> 
        </td>
       </tr> 
    </table>  
  </body> 
  
  <script> 
   $(document).ready(function(){ 
     setInterval(refreshFunction,1000); 
   });

   function refreshFunction(){ 
     $.get('/refresh', function(result){  
       $('#Distance').html(result);  
     }); 
   }      
  </script> 
</html> 
)";

void refresh()
{ 
  //create a char array
  char messageFinal[100];

  //put the distance value in buffer
  sprintf(messageFinal, "%.1f", distance);

  //send data to user
  server.send(200, "application/javascript", messageFinal);
}

//the htmlIndex() is called everytime somebody access the address
//of the board in the browser and sends back a message.
void htmlIndex() 
{
  //send the message to the user
  server.send(200, "text/html", page);
}

void fade_off (int ledPin)
{
  for (int fade = 1024; fade > 0; fade--)
  {
    analogWrite(ledPin, fade);
  }
}
void fade_in (int ledPin)
{
  for (int fade = 0; fade < 1024; fade++)
  {
    analogWrite(ledPin, fade);
  }
}

void Digit(int x)
{
  switch(x)
  {
    case 1:
      digit = 14; //B00001110
      break;
    case 2:
      digit = 13; //B00001101
      break;
    case 3:
      digit = 11; //B00001011
      break;
    case 4:
      digit = 7; //B00000111
      break;
    case 5:
      digit = 0; //B00000000
      break;
  }
}

void number(int dot, int number) {
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  if(dot == 1) {
    number = number + 128;
  }
  shiftOut(dataPin, clockPin, MSBFIRST, number);
  digitalWrite(latchPin, 1);
}

int decimal_to_binar(int numar_zecimal) {
  if(numar_zecimal == 0) return 63;
  if(numar_zecimal == 1) return 6;
  if(numar_zecimal == 2) return 91;
  if(numar_zecimal == 3) return 79;
  if(numar_zecimal == 4) return 86;
  if(numar_zecimal == 5) return 109;
  if(numar_zecimal == 6) return 125;
  if(numar_zecimal == 7) return 7;
  if(numar_zecimal == 8) return 127;
  if(numar_zecimal == 9) return 111;
  return 64; //show "-" if no number from 0-9 is selected
}

void connectToWiFi()
{ 
  Serial.println("Connecting to the WiFi");
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  Serial.println("Waiting for connection");
  while (WiFi.status() != WL_CONNECTED) {
    fade_in(red);
    delay(500);
    fade_off(red);
    Serial.print(".");
  }
  fade_in(green);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupServer()
{ 
  server.on("/", htmlIndex);
  server.on("/refresh", refresh);

  server.begin();

  Serial.println("HTTP server started");
}

void setup() 
{
  //rgb led pins
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  //4 digit 7 led display pins
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  //sensor pins
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  Serial.begin(115200);
  
  delay(2000);

  connectToWiFi();
  setupServer();
  
  for(int j=0; j < 4; j++) {
    ip_x4 = WiFi.localIP()[j];
    if(ip_x4 >= 100) {
      digit_3 = ip_x4 % 10;
      ip_x4 = ip_x4/10;

      digit_2 = ip_x4 % 10;
      ip_x4 = ip_x4/10;

      digit_1 = ip_x4 % 10;
    }
    else if(ip_x4 >= 10) {
      digit_3 = ip_x4 % 10;
      ip_x4 = ip_x4/10;

      digit_2 = ip_x4 % 10;
      digit_1 = 10;
    }
    else {
      digit_3 = ip_x4 % 10;
      digit_2 = 10;
      digit_1 = 10;
    }
    for(int i=0; i < 500; i++) {
      Digit(1);
      number(0, decimal_to_binar(digit_1));
      delay(2);

      Digit(2);
      number(0, decimal_to_binar(digit_2));
      delay(2);

      Digit(3);
      number(1, decimal_to_binar(digit_3));
      delay(2);
    }
    Digit(5);
    number(0, decimal_to_binar(10));
  }
}
 
void loop() {

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected");
    Serial.println("Waiting for reconnection");
    connectToWiFi();
  }
  while (WiFi.status() != WL_CONNECTED) {
    fade_in(blue);
    delay(500);
    fade_off(blue);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
      analogWrite(green, 1024);
  }
  delay(50);
  //the method below is used to manage the incoming requests 
  //from the user
  server.handleClient();

  // Set the trigPin LOW in order to prepare for the next reading
  digitalWrite(trigPin, LOW);
  
  //delay for 2 microseconds
  delayMicroseconds(2);

  //generate a ultrasound for 10 microseconds then turn off the transmitter.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  //reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  //using the formula shown in the guide, calculate the distance
  distance = duration*0.034/2;
  //Serial.println(distance);
}
