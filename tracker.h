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

class Tracker {

    public:
        Tracker();

        int error();
        String errorMessage();

        bool enableGPS();
        bool enableGPRS();

    private:
        Adafruit_FONA *fona;    
        SoftwareSerial *fonaSerial;
        int fonaReady = 0;
        int lastErrorCode = 0;
        String lastErrorMessage = "";
};


#endif
