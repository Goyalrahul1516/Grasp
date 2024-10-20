#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <SPI.h>
#include <time.h>

// RFID variables
#define RXp2 16
#define TXp2 17
#define SS_PIN 21                  //Slave Select Pin/
#define RST_PIN 22                 //Reset Pin for RC522/
#define LED_G 12                   //Pin 8 for LED/
MFRC522 mfrc522(SS_PIN, RST_PIN);  //Create MFRC522 initialized
String rfidTag = "";

// WiFi credentials
const char* ssid = "TU";
const char* password = "tu@inet1";

// Firestore Variables
String rfidTAG;
String packageDate;
double weight;
String grainentriesdata;
const char* locationID = "5";
const char* batch = "A1";

// Firebase project details
const char* firebase_project_id = "grasp-6b2ea";                  // Replace with your Firebase project ID
const char* api_key = "AIzaSyAm0LK6poFHw06ZrYaXzNXvZqv5D9sdMOo";  // Replace with your Firebase Web API Key

// Firestore URL (including API key for authentication)
String firestoreURL = "https://firestore.googleapis.com/v1/projects/" + String(firebase_project_id) + "/databases/(default)/documents/grainentries?key=" + String(api_key);

// Setup
void setup() {
  Serial.begin(9600);                           //Serial Communication begin
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);  //RFID serial
  WiFi.begin(ssid, password);

  // Wait for wifi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Synchronize time
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // Use NTP servers
  Serial.println("Waiting for time synchronization...");
  while (time(nullptr) < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTime synchronized");

  // begin RFID
  SPI.begin();             //SPI communication initialized/
  mfrc522.PCD_Init();      //RFID sensor initialized/
  pinMode(LED_G, OUTPUT);  //LED Pin set as output/
  Serial.println("Put your card to the reader...");
  Serial.println();
}

// Get RFID Card details
String readCard() {
  // Check for new card presence
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return "";  // Return empty string if no card is detected
  }

  // Read the UID bytes and construct the RFID tag string
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag += String(mfrc522.uid.uidByte[i], HEX);  // Concatenate bytes as a hex string
  }

  rfidTag.toUpperCase();  // Optional: convert to uppercase for consistency
  Serial.print("RFID Tag: ");  // Print the tag to the Serial Monitor for debugging
  Serial.println(rfidTag);     // Debugging line to check the RFID tag value
  return rfidTag;  // Return the constructed RFID tag
}

// Get current Date
String getcurrentdate() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  return String(dateStr);
}

// Create a firestore document
void createDocument() {
  rfidTAG = readCard();  // Read the RFID tag
  if (rfidTAG != "") {   // Only create a document if an RFID tag was read
    int moisture = analogRead(2);
    packageDate = getcurrentdate();                         // Get the current date and time
    weight = (Serial2.readString().toFloat());  // Read weight from Serial2
    grainentriesdata = "{\"fields\": {\"rfidtag\": {\"stringValue\": \"" + rfidTAG + "\"}, "
                        "\"packagedate\": {\"stringValue\": \"" + packageDate + "\"}, "
                        "\"storageLocationID\": {\"stringValue\": \"" + locationID + "\"}, "
                        "\"batchnumber\": {\"stringValue\": \"" + batch + "\"}, "
                        "\"initialweight\": {\"doubleValue\": \"" + weight + "\"}}}";


    Serial.println(grainentriesdata);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(firestoreURL);
      http.addHeader("Content-Type", "application/json");

      // Send the document data as a POST request
      int httpResponseCode = http.POST(grainentriesdata);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Document created successfully");
        Serial.println("Response: " + response);
      } else {
        Serial.println("Error creating document");
        Serial.println("HTTP Response code: " + String(httpResponseCode));
      }
      http.end();
    } else {
      Serial.println("Error connecting to WiFi");
    }
  }
}

void loop() {
  createDocument();
}