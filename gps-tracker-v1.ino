/********************************************************************************
 * gps-tracker v1
 * using Adafruit's FONA808 module to create a solar powered gps tracking device
 * 
 */

// #include "./tracker.h"
#include "Adafruit_FONA.h"

// #define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
    #define DEBUG_PRINT(...)        DebugStream.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...)      DebugStream.println(__VA_ARGS__)
#endif

#define RETRY_DELAY         5000            // default milliseconds between retries when operation fails
#define RETRY_LOOPS         5              // default number of times a process should be retried before failing
#define LOOP_INTERVAL       10000           // default milliseconds between two sightings reports

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// uint16_t battP, battV;          // Battery voltage and Battery percent

char GPSresponse[65];
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);       // initialization of the device object

String getIMEI(uint16_t st = 0, uint16_t en = 0);

/*
 * ****************************************************************************************************************
 * Setup function
 * ****************************************************************************************************************
 */
void setup() {

    // Using serial monitor for message & debugging output
    while (! Serial);
    Serial.begin(115200);
   
}


/*
 * ****************************************************************************************************************
 * Main Loop
 * ****************************************************************************************************************
 */
void loop() {

    char body[370];
    uint8_t retries = 0;
    uint8_t success = 0;
    uint8_t rdy = 0;

    // Initializing communication with the device if it wasn't done before.
    
    if (! rdy) 
        while (! rdy) {
            fonaSerial->begin(4800);
            rdy = fona.begin(*fonaSerial);
        }

    // Set up configuration for our communication
    fona.setHTTPSRedirect(true);
    fona.HTTP_ssl(true);

    // Turn on GPRS (without GPRS, there is no need to go any further)
    rdy = 0;
    rdy = initGPRS();

    if (rdy) {
        
        // Turn on GPS
        rdy = initGPS();
        while (! rdy) rdy = initGPS();

        // char *body = fona.replybuffer;
        strcpy(body, "{\n\"specversion\": \"1.0\",\n\"type\": \"com.papleux.api\",\n\"source\": \"geolocation/v1/sightings\",\n\"id\": \"");
        // id
        // appendYear(body);
        strcat(body, getCloudeventId().c_str());
        strcat(body, "\",\n");
        // time
        strcat(body, "\"time\": \"");
        strcat(body, getFullTime().c_str());
        strcat(body, "\",\n");
        // datacontenttype and data header
        strcat(body, "\"datacontenttype\": \"application/json\",\n\"data\": {\n");
        // Sighting ID
        strcat(body, "\"id\": \"");
        strcat(body, getSightingId().c_str());
        strcat(body, "\",\n");
        // device id
        strcat(body, "\"device_id\": \"");
        strcat(body, getIMEI().c_str());
        strcat(body, "\",\n");
        // timestamp
        strcat(body, "\"timestamp\": \"");
        strcat(body, getGPS(14,28).c_str());
        strcat(body, "\",\n");
        // latitude
        strcat(body, "\"latitude\": ");
        strcat(body, getLatitudeString().c_str());
        strcat(body, ",\n");
        // longitude
        strcat(body, "\"longitude\": ");
        strcat(body, getLongitudeString().c_str());
        strcat(body, "\n}\n}");
        
        Serial.print (F("main: buffer written - length: "));
        Serial.println (strlen(body));
        Serial.print(body);
        Serial.println(F("\n\n"));
        
        const char url[] = "https://api.papleux.com/geolocation/v1/sightings";
        uint16_t status;
        uint16_t datalen;
        if (fona.HTTP_POST_start(url, F("application/json"), body, strlen(body), &status, &datalen)) {
            delay(250);
            Serial.print(F("RETURN STATUS: ")); Serial.println(status);
            delay(500);
            Serial.println(F("RESPONSE:"));
            Serial.println(fona.replybuffer);
            // while (fona.available()) Serial.print((char) fona.read());

            /*
            if (getResponse (body, datalen)) {
                Serial.println(F("RESPONSE:"));
                Serial.println(body);
                success = 1;
            }
            */
            fona.HTTP_POST_end ();
        }
        else Serial.println("Posting data failed");
    
    }

    // Reset everything and wait for next loop
    // --------------------------------------------------------------------
    fona.enableGPRS(false);
    delay(RETRY_DELAY);
    fona.enableGPS(false);
    delay(RETRY_DELAY);
    delay(LOOP_INTERVAL);
}




/*
 * ****************************************************************************************************************
 * int initConfig()
 * --> readies the fona device and serial communication to it. returns 0 if fails.
 * ****************************************************************************************************************
 */
 /*
int initConfig() {
    uint8_t retries = 0;
    uint8_t success = 0;
    
    
    // Get battery percentage
    // ----------------------------------------------------
    retries = 0;
    battP = 0;
    success = fona.getBattPercent(&battP);
    while ((! success) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        success = fona.getBattPercent(&battP);
    }
    
    // Get battery Voltage
    // ----------------------------------------------------
    retries = 0;
    battV = 0;
    success = fona.getBattVoltage(&battV);
    while ((! success) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        success = fona.getBattVoltage(&battV);
    }
    
    
    // Set up HTTP settings
    // ----------------------------------------------------
    fona.setHTTPSRedirect(true);
    fona.HTTP_ssl(true);
    return 1;        
}
*/ 
char* rawGPS() {
    // static char GPSresponse[65];
    static uint8_t GPSsuccess = 0;
    uint8_t retries = 0;
    if (! GPSsuccess) {
        while (fonaSS.available()) fonaSS.read(); // Flush the serial buffer
        while ((! GPSsuccess) && (retries < RETRY_LOOPS)) {
            fonaSS.println(F("AT+CGNSINF"));
            delay(250);
            GPSsuccess = getResponse(GPSresponse, 65);
            if ((GPSresponse[14]==',') || (GPSresponse[35]==',')) GPSsuccess = 0;
            delay(RETRY_DELAY);
            retries ++;        
        }
    }
    return GPSresponse;
}

/*
 * ****************************************************************************************************************
 * int initGPS()
 * --> Turns on the GPS antenna and wait for signal
 * ****************************************************************************************************************
 */
int initGPS() {
    uint8_t retries = 0, rdy = 0;
    rdy = fona.enableGPS(true);
    while ((! rdy) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        rdy = fona.enableGPS(true);
    }
    if (! rdy) return 0;

    retries = 0;
    rdy = fona.GPSstatus();
    while ((! rdy) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        rdy = fona.GPSstatus();
    }
    if (! rdy) return 0;
    rawGPS();
    return 1;
}


/*
 * ****************************************************************************************************************
 * int initGPRS()
 * --> Turns on the GPRS antenna and service and wait for the network
 * ****************************************************************************************************************
 */
int initGPRS() {
    uint8_t retries = 0, rdy = 0;
    fona.setGPRSNetworkSettings(F("wholesale"));
    rdy = fona.enableGPRS(true);
    while ((! rdy) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        rdy = fona.enableGPRS(true);
    }
    if (! rdy) return 0;

    retries = 0;
    rdy = fona.GPRSstate();
    while ((! rdy) && (retries < RETRY_LOOPS)) {
        retries++;
        delay(RETRY_DELAY);
        rdy = fona.GPRSstate();
    }
    if (! rdy) return 0;
    return 1;
}





/*
 * ****************************************************************************************************************
 * loadGPS
 * ****************************************************************************************************************
 */
int getGPS(uint16_t startOffset, uint16_t endOffset, char *buf) {
    for (uint8_t i = startOffset; i < endOffset; i++) strcat(buf, GPSresponse[i]);
    return 1;
}

int appendYear(char *buf) { return getGPS(14,18, buf); }

String getGPS(uint16_t startOffset, uint16_t endOffset) {
    // char *GPSresponse = rawGPS();
    return String(GPSresponse).substring(startOffset, endOffset);
}

String getYear()        { return getGPS(14,18); }
String getYear2()       { return getGPS(16,18); }
String getMonth()       { return getGPS(18,20); }
String getDay()         { return getGPS(20,22); }
String getDateYMD()     { return getGPS(14,18) + "-" + getGPS(18,20) + "-" + getGPS(20,22); }
String getHours()       { return getGPS(22,24); }
String getMinutes()     { return getGPS(24,26); }
String getSeconds()     { return getGPS(26,28); }
String getTimeHMS()     { return getGPS(22,24) + ":" + getGPS(24,26) + ":" + getGPS(26,28); }
// float getLatitude()     { return getGPS(33,42).toFloat(); }
String getLatitudeString()     { return getGPS(33,42); }
// float getLongitude()    { return getGPS(43,53).toFloat(); }
String getLongitudeString()    { return getGPS(43,53); }
String getCloudeventId()  { return "0001-" + getGPS(16, 20) + "-" + getGPS(22, 26); }
String getFullTime()    { return getDateYMD() + "T" + getTimeHMS(); }
String getSightingId()  { return getIMEI(0,4) + getGPS(14,28); }


String getIMEI(uint16_t startOffset = 0, uint16_t endOffset = 0) {
    static char IMEIresponse[16];
    static uint8_t IMEIsuccess = 0;
    uint8_t retries = 0;
    if (! IMEIsuccess) {
        while (fonaSS.available()) fonaSS.read(); // Flush the serial buffer
        while ((! IMEIsuccess) && (retries < RETRY_LOOPS)) {
            fonaSS.println(F("AT+GSN"));
            delay(250);
            IMEIsuccess = getResponse(IMEIresponse, 16);
            delay(RETRY_DELAY);
            retries ++;        
        }
    }
    if (! endOffset) return String(IMEIresponse);
    else return String(IMEIresponse).substring(startOffset, endOffset);
}





/*
 * ****************************************************************************************************************
 * int getResponse(char *buf, uint16_t timeout)
 * --> gets the device's current location using GPS or GPRS method based on availability
 * ****************************************************************************************************************
 */
int getResponse(char *buf, uint16_t maxlength) {
    uint16_t index = 0, iteration = 0;
    const uint16_t maxIterations = 100000;
    uint8_t readSuccess = 0;
    for (index = 0; index < maxlength; index++) buf[index] = 0;
    index = 0;
    while ((iteration < maxIterations) && (index < maxlength)) {
        if (fonaSS.available()) {
            buf[index] = fonaSS.read();
            if (buf[index] >= 32) {
                readSuccess = 1;
                index++;
            }
        }
        else if (readSuccess) break;
        iteration++;
    }
    if (readSuccess) buf[index-1] = 0;
    while (fonaSS.available()) fonaSS.read();
    return readSuccess;
}

int printResponse() {
    uint16_t index = 0, iteration = 0;
    const uint16_t maxIterations = 100000;
    char c;
    while (fonaSS.available()) {
        c = fonaSS.read();
        Serial.print(c);
        index++;
    }
    return 1;
}



/*
 * ****************************************************************************************************************
 * int getResponse(char *buf, uint16_t timeout)
 * --> gets the device's current location using GPS or GPRS method based on availability
 * ****************************************************************************************************************
 */
 /*
int http_get(char *url, char *response, uint16_t maxlength) {
    uint16_t returnStatus, length, i, success;
    success = fona.HTTP_GET_start (&url[0], &returnStatus, &length);
    if (success) {
        delay(250);
        Serial.print("URL: "); Serial.println(url);
        Serial.print("RETURN STATUS: "); Serial.println(returnStatus);
        if (getResponse (response, maxlength)) success = 1;
        fona.HTTP_GET_end ();
    }
    return success;
}
*/
