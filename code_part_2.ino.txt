#include <ESP8266WiFi.h>

// Network Information
const char* ssid     = "ardent";
const char* password = "12345678";

String ledState = "";
String brightness = "";

char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "NUEBLW9OA58DLL4N";          // Be sure to change this to your channel Write API key
WiFiClient client;

void setup()
{
  Serial.begin( 115200 );   // You may need to adjust the speed depending on your hardware.
  connectWifi();
}
void loop()
{
  filterData();
  HTTPPost( );
  delay( 15000 );
  // If you remove the sleep, be sure to add more delay so you don't post to ThingSpeak too often.

}



int connectWifi()
{
  WiFi.begin( ssid , password );
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.println( "Connecting to WiFi" );
    delay( 2500 );
  }
  //Serial.println( "Connected" );  // Inform the serial monitor
}

void HTTPPost() {

  // This function builds the data string for posting to ThingSpeak and provides the correct format for the wifi client to communicate with ThingSpeak.
  // It will post "numFields" worth of data entries, and take the data from the fieldData parameter passed to it.
  // Be sure to increase numFields to the number of fields you need, and activate the fields in your channel view.

  if (client.connect( thingSpeakAddress , 80 )) {

    // Build the Posting data string.  If you have multiple fields, make sure the sting does not exceed 1440 characters.
    String PostData = "api_key=" + writeAPIKey ;

    PostData += "&field1=" + ledState;
    PostData += "&field2=" + brightness;

    // POST data via HTTP
    client.println( "POST /update HTTP/1.1" );
    client.println( "Host: api.thingspeak.com" );
    client.println( "Connection: close" );
    client.println( "Content-Type: application/x-www-form-urlencoded" );
 
   client.println( "Content-Length: " + String( PostData.length() ) );
    client.println();
    client.println( PostData );
    client.stop();
  }
}

void filterData() {
  if (Serial.available())
  {
    String buffer = "";
    buffer = Serial.readStringUntil(';');
    int i1 = buffer.indexOf(',');
    ledState = buffer[0];
    buffer.remove(0, i1 + 1);
    brightness = buffer;
  }
}
