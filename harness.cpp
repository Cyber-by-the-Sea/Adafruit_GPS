#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <Adafruit_GPS.h>

Adafruit_GPS GPS;

#define FUZZER_MAX_BUFFER_LEN MINMEA_MAX_SENTENCE_LENGTH + 3 + 1



extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if(size > 4096) {
        return 0;
    }
    char sentence[5000];
    //char* sentence = (char*) calloc(size + 3, sizeof(uint8_t));
    memcpy(sentence, data, size);
    sentence[size] = 0;
    GPS.addChecksum(sentence);
    //printf("%s\n", sentence);
    GPS.parse(sentence); // this also sets the newNMEAreceived() flag to false
    //free(sentence);
    return 0;
}
