

#include <SoftwareSerial.h>
#include "./tracker.h"


/************************************************************************************
 * CONSTRUCTOR: Instantiates the tracker object
 ***********************************************************************************/
Tracker::Tracker() : Adafruit_FONA(FONA_RST) {
    // Properties initialization
    lastErrorCode = 0;
    fonaReady = 0;
    latitude = 0;
    longitude = 0;
    speed_kph = 0;
    speed_mph = 0;
    heading = 0;
    altitude = 0;
    fonaSerial = 0;
}


/************************************************************************************
 * DESTRUCTOR: destroys the tracker object
 ***********************************************************************************/
Tracker::~Tracker() {
    enableGPS(false);
    enableGPRS(false);
    delete fonaSerial;
    fonaSerial=0;
}




/************************************************************************************
 * init: Initializes the device
 ***********************************************************************************/
int Tracker::init() {

    int retries;
    int netStatus;
    
    // Instantiate the fonaSerial object through which we communicate withe the fona device
    // --------------------------------------------------------------------------------------
    if (fonaSerial) { delete fonaSerial; fonaSerial = 0; }
    fonaSerial = new SoftwareSerial(FONA_TX, FONA_RX);
    if (! fonaSerial) { lastErrorCode = 100; return 0; }
    Serial.println(F("init: Software Serial Module activated"));


    // Establish the serial communication to the FONA device
    // --------------------------------------------------------------------------------------
    fonaSerial->begin(FONA_TRANSMISSION_SPEED);
    if (! begin(*fonaSerial)) { lastErrorCode = 100; return 0; }
    Serial.println(F("init: Software Serial Module loaded into device"));

    // Enable GPS module
    // --------------------------------------------------------------------------------------
    retries = 0;
    while ((! enableGPS(true)) && (retries < 2)) {
        Serial.print(F("init: GPS enablement failed. GPS Status: "));
        Serial.println(GPSstatus());
        delay(250);
        retries++;
    } // try to enable the GPS module 3 times
    retries = 0;
    while ((! GPSstatus()) && (retries < 119)) {
        Serial.print(F("init: Waiting for GPS to come online - retry "));
        Serial.println(retries);
        delay(2000);
        retries++;
    } // retries every 2 seconds for up to 2 minutes
    if (! GPSstatus()) { lastErrorCode = 200; return 0; }
    Serial.println(F("init: GPS module online"));
    
    // Enable GPRS module
    // --------------------------------------------------------------------------------------
    retries = 0;
    while ((! enableGPRS(true)) && (retries < 2)) {
        Serial.print(F("init: GPRS enablement failed. GPRS Status: "));
        Serial.println(GPRSstate());
        delay(250);
        retries++;
    } // try to enable the GPRS module 3 times
    retries = 0;
    while ((! GPRSstate()) && (retries < 119)) {
        Serial.print(F("init: Waiting for GPRS to come online - retry "));
        Serial.println(retries);
        delay(2000);
        retries++;
    } // retries every 2 seconds for up to 2 minutes
    if (! GPRSstate()) { lastErrorCode = 300; return 0; }
    Serial.println(F("init: GPRS module online"));


    // Making sure network is up
    // --------------------------------------------------------------------------------------
    retries = 0;
    netStatus = getNetworkStatus();
    while ((netStatus != 1) && (netStatus != 4) && (netStatus != 5) && (retries < 119)) {
        Serial.print(F("init: cellular network not available - retry "));
        Serial.println(retries);
        delay(2000);
        retries++;
        netStatus = getNetworkStatus();
    } // retries every 2 seconds for up to 2 minutes
    if ((netStatus != 1) && (netStatus != 4) && (netStatus != 5)) { lastErrorCode = 301; return 0; }
    Serial.println(F("init: cellular network is up"));

    // Configure system to execute commands
    // --------------------------------------------------------------------------------------
    // getIMEI(&imei[0]);
    HTTP_ssl(true);
    setHTTPSRedirect(true);
    
    
    // If everything went well, the fona device is ready to be used
    // --------------------------------------------------------------------------------------
    fonaReady = 1;
    lastErrorCode = 0;
    return 1;
}


/************************************************************************************
 * printStatus: Send a status report to the Serial port
 ***********************************************************************************/
int Tracker::printStatus() {
    int buf = 0;
    // char str[256];
    
    Serial.println(F("\n\n\n\n"));
    Serial.println(F("System Status"));
    Serial.println(F("---------------------------------------------------"));
/*
    Serial.print(F("Device IMEI: \t\t"));
    getIMEI(&str[0]);
    Serial.println(str);

    Serial.print(F("ADC Voltage: \t\t"));
    if (getADCVoltage(&buf)) Serial.println(buf);
    else Serial.println(F("Could not read ADC"));

    Serial.print(F("Battery Voltage: \t"));
    if (getBattVoltage(&buf)) Serial.println(buf);
    else Serial.println(F("Could not read Battery"));
*/
    Serial.print(F("Battery %: \t\t"));
    if (getBattPercent(&buf)) Serial.println(buf);
    else Serial.println(F("Could not read Battery"));
/*
    buf = getNetworkStatus();
    Serial.print(F("Network status:\t\t"));
    if (buf == 0) Serial.println(F("Not registered"));
    if (buf == 1) Serial.println(F("Registered (home)"));
    if (buf == 2) Serial.println(F("Not registered (searching)"));
    if (buf == 3) Serial.println(F("Denied"));
    if (buf == 4) Serial.println(F("Unknown"));
    if (buf == 5) Serial.println(F("Registered roaming"));
*/
    Serial.print(F("Current Longitude:\t"));
    Serial.println(longitude, 5);
    
    Serial.print(F("Current Latitude:\t"));
    Serial.println(latitude, 5);

    Serial.print(F("Current Speed (mph):\t"));
    Serial.println(speed_mph);

    Serial.print(F("Current Speed (Km/h):\t"));
    Serial.println(this->speed_kph);

    Serial.print(F("Current heading:\t"));
    Serial.println(this->heading);

    Serial.print(F("Current Altitude:\t"));
    Serial.println(this->altitude);

    Serial.println(F("\n\n\n\n"));
}




/************************************************************************************
 * getGeolocation: loads GPS coordinates, either directly from GPS or from GPRS
 ***********************************************************************************/
int Tracker::getGeolocation() {
    if (getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) {
        speed_mph = speed_kph * 0.621371192;
        return 1;
    }
    else {
        lastErrorCode = 201;
    }
    if (getGSMLoc(&latitude, &longitude)) {
        speed_kph = 0;
        speed_mph = 0;
        heading = 0;
        altitude = 0;
        return 1;
    }
    longitude = 0;
    latitude = 0;
    speed_kph = 0;
    speed_mph = 0;
    heading = 0;
    altitude = 0;
    return 0;
}






/************************************************************************************
 * error(): returns the last error code produced
 ***********************************************************************************/
int Tracker::getLastErrorCode() {
    return lastErrorCode;    
}




/************************************************************************************
 * errorMessage(): returns the error message that went with the last error code
 * and resets the last error code now that the message has been read. Note that
 * the last message is not reset so you should only read it once, and only if
 * the error code is non-zero.
 ***********************************************************************************/
String Tracker::getLastErrorMessage() {
    // reset error code
    switch (lastErrorCode) {
        case 0:     lastErrorCode = 0; return "Success!";
        case 100:   lastErrorCode = 0; return "Tracker device: not ready";
        case 200:   lastErrorCode = 0; return "GPS module: Failed to start GPS device";
        case 201:   lastErrorCode = 0; return "GPS location was not obtained from the GPS antenna";
        case 300:   lastErrorCode = 0; return "GPRS module: Failed to engage GPRS";
        case 301:   lastErrorCode = 0; return "Cellular network not available";
    }
    return "";
}
