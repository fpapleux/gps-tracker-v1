/********************************************************************************
 * gps-tracker v1
 * using Adafruit's FONA808 module to create a solar powered gps tracking device
 * 
 */

#include "./tracker.h"

#define RETRY_DELAY 1 // minutes between retries
#define LOOP_INTERVAL 2 // minutes to wait between reports of GPS coordinates

Tracker *tracker;
// String body;
// char bodyChar[256];
char IMEI[13];
uint16_t    return_status, return_datalength;

void setup() {

    // Using serial monitor for message & debugging output
    while (! Serial);
    Serial.begin(115200);
    Serial.println(F("main: Serial output initialized."));

    tracker = new Tracker();

    // Initialize tracker device
    // --------------------------------------------------------------------
    int retries = 0;
    while ((! tracker->init()) && (retries < 2)) {
        Serial.println(F("main: Error enabling tracker device - retrying..."));
        delay(5000);
        retries++;
    } // Try 3 times with 5 seconds in between
    
    if (tracker->getLastErrorCode()) {
        Serial.print(F("main: failed to initialize tracker device. ERROR: "));
        Serial.println(tracker->getLastErrorMessage());
        // will then skip to the end of the loop
    }

}

void loop() {

    if (tracker->getGeolocation()) {
        tracker->printStatus();
        Serial.println(F("\n\nTrying to communicate information to an API"));
        
        // building the cloudevents json body (lots to clean up here)
        
        // header = F("content-type: application/cloudevents+json");
        /*
        char body = F("{"
                        "\"specversion\": \"1.0\","
                        "\"type\": \"com.example.someevent\","
                        "\"source\": \"/geolocation\","
                        "\"id\": \"0000-0000-0001\","
                        "\"time\": \"2020-02-15T19:50:00\","
                        "\"datacontenttype\": \"application/json\","
                        "\"data\": "
                            "{"
                            "\"id\": \"123456789ABC\","
                            "\"longitude\": \"-84.34286\","
                            "\"latitude\": \"33.97453\""
                            "}"
                        "}");

        
        // tracker->getIMEI(&IMEI[0]);
        // body += IMEI;
        // body += "\",\"longitude\": \"-84.34286\",\"latitude\": \"33.97453\"}}";
        // int headerLength = header.length()+1;
        // int bodyLength = body.length()+1;
        int bodyLength=128;
        // header.toCharArray(headerChar, headerLength);
        // body.toCharArray(bodyChar, bodyLength);
        */

        Serial.println("All that string setup stuff is ok");

        // Call the API
        // if (tracker->HTTP_POST_start((char*) &POST_API_URL[0], F("application/json"), &body, (uint16_t) bodyLength, &return_status, &return_datalength)) {
        if (tracker->HTTP_GET_start((char*) &GET_API_URL[0], &return_status, &return_datalength)) {
            // wait and receive response
            while (return_datalength > 0) {
                char c = tracker->read();
                loop_until_bit_is_set(UCSR0A, UDRE0);
                UDR0 = c;
                Serial.print(c);
                return_datalength--;
                // if (! return_datalength); break;
            }
            // close communication
            // tracker->HTTP_POST_end();
            tracker->HTTP_GET_end();
                
        } // Execute all this code provided the POST was successful. Otherwise close here.
        else Serial.println("THE HTTP POST call did not work");
    }

    
    // Reset everything and wait for next loop
    // --------------------------------------------------------------------
    delay(LOOP_INTERVAL / 2 * 60000);
    // delete tracker;
    // tracker = 0;
    delay(LOOP_INTERVAL / 2 * 60000);
}
