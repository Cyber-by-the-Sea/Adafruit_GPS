#include <Adafruit_GPS.h>

Adafruit_GPS GPS;

int main(int argc, char *argv[]) {
    GPS.parse(argv[1]); // this also sets the newNMEAreceived() flag to false
}
