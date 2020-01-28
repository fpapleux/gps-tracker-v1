/********************************************************************************
 * gps-tracker v1
 * using Adafruit's FONA808 module to create a solar powered gps tracking device
 * 
 */

#include "./tracker.h"

#define RETRY_DELAY 1 // minutes between retries

Tracker *tracker;

void setup() {

    // Using serial monitor for message & debugging output
    while (! Serial);
    Serial.begin(115200);
    Serial.println("Serial output initialized.");

    // Initializes the FONA tracker and loops until it is successful
    tracker = new Tracker();
    while (tracker->error()) {
        Serial.print(tracker->errorMessage());
        Serial.println(" - retrying in a minute.");
        delay(RETRY_DELAY * 60000);
        tracker = new Tracker();
    }

    
}

void loop() {

}
