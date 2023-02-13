#include "LilyGo-EPD47.h"


void setup() {
    if(psramInit()){
        printf("\nThe PSRAM is correctly initialized\n");
    } else{
        printf("\nPSRAM does not work\n");
    }

    idf_setup();
}

void loop() {
    idf_loop();
}

