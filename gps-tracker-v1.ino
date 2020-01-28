/********************************************************************************
 * gps-tracker v1
 * using Adafruit's FONA808 module to create a solar powered gps tracking device
 * 
 */

#include "./tracker.h"

#define RETRY_DELAY 1 // minutes between retries
#define REPORT_DELAY 0 // minutes to wait between to reports of GPS coordinates

Tracker *tracker;

void setup() {

    // Using serial monitor for message & debugging output
    while (! Serial);
    Serial.begin(115200);
    Serial.println("Serial output initialized.");

    // Initializes the FONA tracker and loops until it is successful
    // Since we are in the setup, there is no point in executing the loop as long
    // as the tracker is not working.
    tracker = new Tracker();
    while (tracker->error()) {
        Serial.print(tracker->errorMessage());
        Serial.println(" - retrying in a minute.");
        delay(RETRY_DELAY * 60000);
        tracker = new Tracker();
    }

    
}

void loop() {

    // Turn on GPS device
    int retries = 0;
    while ((! tracker->enableGPS()) && (retries < 2)) {
        Serial.print(tracker->errorMessage());
        Serial.println(" - retrying...");
        retries++;
    }
    if (tracker->error()) {
        Serial.println("failed to activate GPS device");
    }
    
    delay(REPORT_DELAY * 60000);
}
