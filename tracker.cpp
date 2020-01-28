

#include <SoftwareSerial.h>
#include "./tracker.h"

/************************************************************************************
 * CONSTRUCTOR: Initializes the device
 ***********************************************************************************/
Tracker::Tracker() {
    // Properties initialization
    this->lastErrorCode = 0;
    this->lastErrorMessage = "";
    this->fonaReady = 0;
    this->fona = new Adafruit_FONA(FONA_RST);

    // Establishes the serial communication to the FONA device
    this->fonaSerial = new SoftwareSerial(FONA_TX, FONA_RX);
    fonaSerial->begin(9600);
    if (! fona->begin(*fonaSerial)) {
        this->lastErrorCode = 1;
    } 
    else {
        this->lastErrorCode = 0;
        this->fonaReady = 1;
    }
}

/************************************************************************************
 * enableGPS: turns on the GPS antenna to get location services
 ***********************************************************************************/
bool Tracker::enableGPS() {
    if (! this->fonaReady) {
        this->lastErrorCode = 1;
    }
    return 1;
}

/************************************************************************************
 * enableGPRS: sets up the GPRS connection for internet connection
 ***********************************************************************************/
bool Tracker:: enableGPRS() {

    return 1;
}

/************************************************************************************
 * error(): returns the last error code produced
 ***********************************************************************************/
int Tracker::error() {
    return this->lastErrorCode;    
}

/************************************************************************************
 * errorMessage(): returns the error message that went with the last error code
 * and resets the last error code now that the message has been read. Note that
 * the last message is not reset so you should only read it once, and only if
 * the error code is non-zero.
 ***********************************************************************************/
String Tracker::errorMessage() {
    // reset error code
    switch (lastErrorCode) {
        case 0: this->lastErrorMessage = "Success!"; break;
        case 1: this->lastErrorMessage = "FONA device is not ready"; break;
    }
    this->lastErrorCode = 0;
    return this->lastErrorMessage;
}
