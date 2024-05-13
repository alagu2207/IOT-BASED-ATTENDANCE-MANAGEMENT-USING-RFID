#include <WiFiClient.h> 
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>
#include <SimpleTimer.h>           
#include <Adafruit_Fingerprint.h>  

//************************************************************************
//Fingerprint scanner Pins
#define Finger_Rx 14 //D5
#define Finger_Tx 12 //D6

//************************************************************************
WiFiClient client;
SimpleTimer timer;
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

//************************************************************************
/* Set these to your desired credentials. */
const char *ssid = "alagu";
const char *password = "12345678";
const char* device_token  = "54d70956";

//************************************************************************
String getData, Link;
String URL = "http://192.168.29.127/iot/biometricattendancev2/getdata.php";

//************************************************************************
int FingerID = 0, t1, t2;                           // The Fingerprint ID from the scanner 
bool device_Mode = false;                           // Default Mode Enrollment
bool firstConnect = false;
uint8_t id;
unsigned long previousMillis = 0;
void connectToWiFi();
void CheckMode();
void ChecktoDeleteID();
void ChecktoAddID();

void SendFingerprintID(int finger);
int getFingerprintID();
uint8_t deleteFingerprint(int id);
void CheckFingerprint();



void setup() {
  connectToWiFi();
  Serial.begin(57600);
  delay(1000);

  
  //---------------------------------------------
  
  //---------------------------------------------
  // Set the data rate for the sensor serial port
  finger.begin(57600);
  Serial.println("\n\nAdafruit finger detect test");

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
   
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    
    while (1) { delay(1); }
  }
  //---------------------------------------------
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  //Timers---------------------------------------
    ChecktoAddID();
  ChecktoDeleteID();
 
   //Set an internal timer every 15sec to check wheater there an ID to delete in the website.
  /
  CheckMode();
}

//************************************************************************
void loop() {
  
         void connectToWiFi();
  if(!WiFi.isConnected()){
    if (millis() - previousMillis >= 10000) {
      previousMillis = millis();
      connectToWiFi();    
    }
  }
  CheckFingerprint();   
  delay(10);
}

//************************************************************************
void CheckFingerprint(){
  SendFingerprintID(FingerID);
  FingerID = getFingerprintID();

}


//************************************************************************
void SendFingerprintID( int finger ){
  Serial.println("Sending the Fingerprint ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?FingerID=" + String(finger) + "&device_token=" + device_token; // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request   //Specify content-type header
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
    
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
    Serial.println(finger);     //Print fingerprint ID
  
    if (payload.substring(0, 5) == "login") {
      String user_name = payload.substring(5);
  //  Serial.println(user_name);
      
     
    }
    else if (payload.substring(0, 6) == "logout") {
      String user_name = payload.substring(6);
  //  Serial.println(user_name);
      
    }
    delay(10);
    http.end();  //Close connection
  }
}

//************************************************************************
int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return -2;
    default:
      Serial.println("Unknown error");
      return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return -2;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return -2;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return -2;
    default:
      Serial.println("Unknown error");
      return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return -1;
  } else {
    Serial.println("Unknown error");
    return -2;
  }   
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
  delay(100);
   
}

  
  
//************************************************************************
void ChecktoDeleteID(){
  Serial.println("Check to Delete ID");
  if(WiFi.isConnected()){
    HTTPClient http;  
    getData = "?DeleteID=check&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request,
   Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
  
    if (payload.substring(0, 6) == "del-id") {
      String del_id = payload.substring(6);
      Serial.println(del_id);
      http.end();  //Close connection
      deleteFingerprint( del_id.toInt() );
      delay(1000);
    }
    http.end();  //Close connection
  }
}

//************************************************************************
uint8_t deleteFingerprint(int id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
     Serial.println("Deleted!");
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
     Serial.println("Could not delete in that location");
 
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); 
    Serial.println(p, HEX);
    
    return p; // Return p in the default case as well
  }   
}


//************************************************************************
void ChecktoAddID(){
 Serial.println("Check to Add ID");
  if(WiFi.isConnected()){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?Get_Fingerid=get_id&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin(client, Link); //initiate HTTP request,
  Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
  
   if (payload.substring(0, 6).equalsIgnoreCase("add-id")) {
    String add_id = payload.substring(6);
    Serial.println(add_id);
    id = add_id.toInt();
    http.end();  //Close connection
    getFingerprintEnroll();
}
 else {
    Serial.println("Unexpected response from server: " + payload);
}
    http.end();  //Close connection
  }
}
//*

//************************************************************************


void CheckMode() {
    // Your function implementation here
    Serial.println("Check Mode");
    if(WiFi.isConnected()){
        HTTPClient http;    //Declare object of class HTTPClient
        //GET Data
        getData = "?Check_mode=get_mode&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
        //GET method
        Link = URL + getData;
        http.begin(client, Link); //initiate HTTP request,
        Serial.print("Checking Mode at: ");
        Serial.println(Link); // Print the URL for debugging
        int httpCode = http.GET();   //Send the request
        String payload = http.getString();    //Get the response payload

        Serial.print("HTTP Code: ");
        Serial.println(httpCode);
        Serial.print("Response payload: ");
        Serial.println(payload);
        
        if (payload.substring(0, 4) == "mode") {
            String dev_mode = payload.substring(4);
            int devMode = dev_mode.toInt();
            if (!firstConnect) {
                device_Mode = devMode;
                firstConnect = true;
            }
            Serial.println(dev_mode);
            if (device_Mode && devMode) {
                device_Mode = false;
              
                Serial.println("Device Mode: Attendance");
               CheckFingerprint();
                
              
            }
             else if (!device_Mode && !devMode) {
                device_Mode = true;
             
             
                Serial.println("Device Mode: Enrollment");
                 ChecktoDeleteID();
                 ChecktoAddID();
            }
             http.end();//Close connection
        }
          http.end();  
    }
}

uint8_t getFingerprintEnroll() {
  int p = -1;

  while (p != FINGERPRINT_OK) {
          
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
    
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      
      break;
    case FINGERPRINT_IMAGEMESS:
      
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
     
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
     
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
   
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
      Serial.println("Fingerprints did not match");
      
    return p;
  } else {
      Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    
 uint8_t confirmResult = confirmAdding(id);
if (confirmResult != FINGERPRINT_OK) {
    return confirmResult;
}
   
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  } 
   return FINGERPRINT_OK; 
}
uint8_t confirmAdding(int id){
  Serial.println("confirm Adding");
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;    //Declare object of class HTTPClient
    //GET Data
    getData = "?confirm_id=" + String(id) + "&device_token=" + String(device_token); // Add the Fingerprint ID to the Post array in order to send it
    //GET method
    Link = URL + getData;
    
    http.begin(client, Link); //initiate HTTP request,
    //Serial.println(Link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
    if(httpCode == 200){  
      Serial.println(payload);
      delay(2000);
      http.end();  //Close connection
      return FINGERPRINT_OK; // or return any other appropriate success code
    }
    else{
      Serial.println("Error Confirm!!");
       //Close connection
      return FINGERPRINT_PACKETRECIEVEERR; // or return any other appropriate error code
    }
     http.end();
  }
  return FINGERPRINT_PACKETRECIEVEERR; // Return a default error code if WiFi is not connected
}
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    uint32_t periodToConnect = 30000L;
    for(uint32_t StartToConnect = millis(); (millis()-StartToConnect) < periodToConnect;){
        if ( WiFi.status() != WL_CONNECTED ){
            delay(500);
            Serial.print(".");
        } else{
            break;
        }
    }
    
    if(WiFi.isConnected()){
        Serial.println("");
        Serial.println("Connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    }
    else{
        Serial.println("");
        Serial.println("Not Connected");
        WiFi.mode(WIFI_OFF);
        delay(1000);
    }
    delay(1000);
}
