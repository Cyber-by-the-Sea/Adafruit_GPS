#include <Adafruit_GPS.h>

Adafruit_GPS GPS;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 0;
    }
    int len = strlen(argv[1]);
    char* sentence = (char*)  calloc(len + 3, sizeof(char));
    memcpy(sentence, argv[1], len);
    GPS.addChecksum(sentence);
    printf("%s\n", sentence);
    GPS.parse(argv[1]); // this also sets the newNMEAreceived() flag to false
}
