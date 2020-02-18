/********************************************************************
 * 
 * tracker.h
 * 
 * Logic simplification class for the Adafruit_FONA class to use
 * in the gps tracker.
 * 
 */

#ifndef FP_TRACKER_H
#define FP_TRACKER_H

#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_TRANSMISSION_SPEED 9600
#define POST_API_URL "https://d2kh72sh76.execute-api.us-east-1.amazonaws.com/v1/location"
#define GET_API_URL "https://api.papleux.com/geolocation/v1/"

class Tracker : public Adafruit_FONA {

    public:
        Tracker();
        ~Tracker();
        int init();
        
        int getGeolocation();
        
        int getLastErrorCode();
        String getLastErrorMessage();
        int printStatus();

    private:
        // Hardware access properties
        SoftwareSerial *fonaSerial;
        
        // Status properties
        int fonaReady;
        int lastErrorCode;

        // Geolocation properties
        float latitude, longitude, speed_kph, heading, speed_mph, altitude;

};


#endif
