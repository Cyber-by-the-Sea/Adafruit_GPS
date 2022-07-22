/**************************************************************************/
/*!
  @file Adafruit_GPS.cpp

  @mainpage Adafruit Ultimate GPS Breakout

  @section intro Introduction

  This is the Adafruit GPS library - the ultimate GPS library
  for the ultimate GPS module!

  Tested and works great with the Adafruit Ultimate GPS module
  using MTK33x9 chipset
  ------> http://www.adafruit.com/products/746

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  @section author Author

  Written by Limor Fried/Ladyada for Adafruit Industries.

  @section license License

  BSD license, check license.txt for more information
  All text above must be included in any redistribution
*/
/**************************************************************************/

#include <Adafruit_GPS.h>

static bool strStartsWith(const char *str, const char *prefix);

/**************************************************************************/
/*!
    @brief Is the field empty, or should we try conversion? Won't work
    for a text field that starts with an asterisk or a comma, but that
    probably violates the NMEA-183 standard.
    @param pStart Pointer to the location of the token in the NMEA string
    @return true if empty field, false if something there
*/
/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for latitude angle
    @param p Pointer to the location of the token in the NMEA string
*/
/**************************************************************************/
// void Adafruit_GPS::parseLat(char *p) {
//   char degreebuff[10];
//   if (!isEmpty(p)) {
//     strncpy(degreebuff, p, 2);
//     p += 2;
//     degreebuff[2] = '\0';
//     long degree = atol(degreebuff) * 10000000;
//     strncpy(degreebuff, p, 2); // minutes
//     p += 3;                    // skip decimal point
//     strncpy(degreebuff + 2, p, 4);
//     degreebuff[6] = '\0';
//     long minutes = 50 * atol(degreebuff) / 3;
//     latitude_fixed = degree + minutes;
//     latitude = degree / 100000 + minutes * 0.000006F;
//     latitudeDegrees = (latitude - 100 * int(latitude / 100)) / 60.0f;
//     latitudeDegrees += int(latitude / 100);
//   }
// }

/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for latitude direction
    @param p Pointer to the location of the token in the NMEA string
    @return True if we parsed it, false if it has invalid data
*/
/**************************************************************************/
// bool Adafruit_GPS::parseLatDir(char *p) {
//   if (p[0] == 'S') {
//     lat = 'S';
//     latitudeDegrees *= -1.0f;
//     latitude_fixed *= -1;
//   } else if (p[0] == 'N') {
//     lat = 'N';
//   } else if (p[0] == ',') {
//     lat = 0;
//   } else {
//     return false;
//   }
//   return true;
// }

/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for longitude angle
    @param p Pointer to the location of the token in the NMEA string
*/
/**************************************************************************/
// void Adafruit_GPS::parseLon(char *p) {
//   int32_t degree;
//   long minutes;
//   char degreebuff[10];
//   if (!isEmpty(p)) {
//     strncpy(degreebuff, p, 3);
//     p += 3;
//     degreebuff[3] = '\0';
//     degree = atol(degreebuff) * 10000000;
//     strncpy(degreebuff, p, 2); // minutes
//     p += 3;                    // skip decimal point
//     strncpy(degreebuff + 2, p, 4);
//     degreebuff[6] = '\0';
//     minutes = 50 * atol(degreebuff) / 3;
//     longitude_fixed = degree + minutes;
//     longitude = degree / 100000 + minutes * 0.000006F;
//     longitudeDegrees = (longitude - 100 * int(longitude / 100)) / 60.0f;
//     longitudeDegrees += int(longitude / 100);
//   }
// }

/**************************************************************************/
/*!
    @brief Parse a part of an NMEA string for longitude direction
    @param p Pointer to the location of the token in the NMEA string
    @return True if we parsed it, false if it has invalid data
*/
/**************************************************************************/
// bool Adafruit_GPS::parseLonDir(char *p) {
//   if (!isEmpty(p)) {
//     if (p[0] == 'W') {
//       lon = 'W';
//       longitudeDegrees *= -1.0f;
//       longitude_fixed *= -1;
//     } else if (p[0] == 'E') {
//       lon = 'E';
//     } else if (p[0] == ',') {
//       lon = 0;
//     } else {
//       return false;
//     }
//   }
//   return true;
// }

/**************************************************************************/
/*!
    @brief Constructor when there are no communications attached
*/
/**************************************************************************/
Adafruit_GPS::Adafruit_GPS() {
  common_init(); // Set everything to common state, then...
  noComms = true;
}

/**************************************************************************/
/*!
    @brief Initialization code used by all constructor types
*/
/**************************************************************************/
void Adafruit_GPS::common_init(void) {
#if (defined(__AVR__) || defined(ESP8266)) && defined(USE_SW_SERIAL)
  gpsSwSerial = NULL; // Set both to NULL, then override correct
#endif
  recvdflag = false;
  paused = false;
  lineidx = 0;
  currentline = line1;
  lastline = line2;

  hour = minute = seconds = year = month = day = fixquality = fixquality_3d =
      satellites = antenna = 0; // uint8_t
  lat = lon = mag = 0;          // char
  fix = false;                  // bool
  milliseconds = 0;             // uint16_t
  latitude = longitude = geoidheight = altitude = speed = angle = magvariation =
      HDOP = VDOP = PDOP = 0.0; // nmea_float_t
  data_init();
}

/**************************************************************************/
/*!
    @brief    Destroy the object.
    @return   none
*/
/**************************************************************************/
Adafruit_GPS::~Adafruit_GPS() {
  for (int i = 0; i < (int)NMEA_MAX_INDEX; i++)
    removeHistory((nmea_index_t)i); // to free any history mallocs
}

/**************************************************************************/
/*!
    @brief How many bytes are available to read - part of 'Print'-class
   functionality
    @return Bytes available, 0 if none
*/
/**************************************************************************/
size_t Adafruit_GPS::available(void) {
  return 0;
}

/**************************************************************************/
/*!
    @brief Write a byte to the underlying transport - part of 'Print'-class
   functionality
    @param c A single byte to send
    @return Bytes written - 1 on success, 0 on failure
*/
/**************************************************************************/
size_t Adafruit_GPS::write(uint8_t c) {
  return 0;
}

/**************************************************************************/
/*!
    @brief Read one character from the GPS device.

    Call very frequently and multiple times per opportunity or the buffer
    may overflow if there are frequent NMEA sentences. An 82 character NMEA
    sentence 10 times per second will require 820 calls per second, and
    once a loop() may not be enough. Check for newNMEAreceived() after at
    least every 10 calls, or you may miss some short sentences.
    @return The character that we received, or 0 if nothing was available
*/
/**************************************************************************/
char Adafruit_GPS::read(void) {    // as close as we can get to time char was sent
  char c = 0;
  return c;
}

/**************************************************************************/
/*!
    @brief Send a command to the GPS device
    @param str Pointer to a string holding the command to send
*/
/**************************************************************************/
void Adafruit_GPS::sendCommand(const char *str) { return; }

/**************************************************************************/
/*!
    @brief Check to see if a new NMEA line has been received
    @return True if received, false if not
*/
/**************************************************************************/
bool Adafruit_GPS::newNMEAreceived(void) { return recvdflag; }

/**************************************************************************/
/*!
    @brief Pause/unpause receiving new data
    @param p True = pause, false = unpause
*/
/**************************************************************************/
void Adafruit_GPS::pause(bool p) { paused = p; }

/**************************************************************************/
/*!
    @brief Returns the last NMEA line received and unsets the received flag
    @return Pointer to the last line string
*/
/**************************************************************************/
char *Adafruit_GPS::lastNMEA(void) {
  recvdflag = false;
  return (char *)lastline;
}

/**************************************************************************/
/*!
    @brief Wait for a specified sentence from the device
    @param wait4me Pointer to a string holding the desired response
    @param max How long to wait, default is MAXWAITSENTENCE
    @param usingInterrupts True if using interrupts to read from the GPS
   (default is false)
    @return True if we got what we wanted, false otherwise
*/
/**************************************************************************/
bool Adafruit_GPS::waitForSentence(const char *wait4me, uint8_t max,
                                   bool usingInterrupts) {
  uint8_t i = 0;
  while (i < max) {
    if (!usingInterrupts)
      read();

    if (newNMEAreceived()) {
      char *nmea = lastNMEA();
      i++;

      if (strStartsWith(nmea, wait4me))
        return true;
    }
  }

  return false;
}

/**************************************************************************/
/*!
    @brief Start the LOCUS logger
    @return True on success, false if it failed
*/
/**************************************************************************/
bool Adafruit_GPS::LOCUS_StartLogger(void) {
  sendCommand(PMTK_LOCUS_STARTLOG);
  recvdflag = false;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK);
}

/**************************************************************************/
/*!
    @brief Stop the LOCUS logger
    @return True on success, false if it failed
*/
/**************************************************************************/
bool Adafruit_GPS::LOCUS_StopLogger(void) {
  sendCommand(PMTK_LOCUS_STOPLOG);
  recvdflag = false;
  return waitForSentence(PMTK_LOCUS_STARTSTOPACK);
}

/**************************************************************************/
/*!
    @brief Read the logger status
    @return True if we read the data, false if there was no response
*/
/**************************************************************************/
bool Adafruit_GPS::LOCUS_ReadStatus(void) {
  sendCommand(PMTK_LOCUS_QUERY_STATUS);

  if (!waitForSentence("$PMTKLOG"))
    return false;

  char *response = lastNMEA();
  uint16_t parsed[10];
  uint8_t i;

  for (i = 0; i < 10; i++)
    parsed[i] = -1;

  response = strchr(response, ',');
  for (i = 0; i < 10; i++) {
    if (!response || (response[0] == 0) || (response[0] == '*'))
      break;
    response++;
    parsed[i] = 0;
    while ((response[0] != ',') && (response[0] != '*') && (response[0] != 0)) {
      parsed[i] *= 10;
      char c = response[0];
      if (isdigit(c))
        parsed[i] += c - '0';
      else
        parsed[i] = c;
      response++;
    }
  }
  LOCUS_serial = parsed[0];
  LOCUS_type = parsed[1];
  if (isalpha(parsed[2])) {
    parsed[2] = parsed[2] - 'a' + 10;
  }
  LOCUS_mode = parsed[2];
  LOCUS_config = parsed[3];
  LOCUS_interval = parsed[4];
  LOCUS_distance = parsed[5];
  LOCUS_speed = parsed[6];
  LOCUS_status = !parsed[7];
  LOCUS_records = parsed[8];
  LOCUS_percent = parsed[9];

  return true;
}

/**************************************************************************/
/*!
    @brief Standby Mode Switches
    @return False if already in standby, true if it entered standby
*/
/**************************************************************************/
bool Adafruit_GPS::standby(void) {
  if (inStandbyMode) {
    return false; // Returns false if already in standby mode, so that you do
                  // not wake it up by sending commands to GPS
  } else {
    inStandbyMode = true;
    sendCommand(PMTK_STANDBY);
    // return waitForSentence(PMTK_STANDBY_SUCCESS);  // don't seem to be fast
    // enough to catch the message, or something else just is not working
    return true;
  }
}

/**************************************************************************/
/*!
    @brief Wake the sensor up
    @return True if woken up, false if not in standby or failed to wake
*/
/**************************************************************************/
bool Adafruit_GPS::wakeup(void) {
  if (inStandbyMode) {
    inStandbyMode = false;
    sendCommand(""); // send byte to wake it up
    return waitForSentence(PMTK_AWAKE);
  } else {
    return false; // Returns false if not in standby mode, nothing to wakeup
  }
}

/**************************************************************************/
/*!
    @brief Time in seconds since the last position fix was obtained. The
    time returned is limited to 2^32 milliseconds, which is about 49.7 days.
    It will wrap around to zero if no position fix is received
    for this long.
    @return nmea_float_t value in seconds since last fix.
*/
/**************************************************************************/
nmea_float_t Adafruit_GPS::secondsSinceFix() {
  return (time(NULL) - lastFix) / 1000.;
}

/**************************************************************************/
/*!
    @brief Time in seconds since the last GPS time was obtained. The time
    returned is limited to 2^32 milliseconds, which is about 49.7 days. It
    will wrap around to zero if no GPS time is received for this long.
    @return nmea_float_t value in seconds since last GPS time.
*/
/**************************************************************************/
nmea_float_t Adafruit_GPS::secondsSinceTime() {
  return (time(NULL) - lastTime) / 1000.;
}

/**************************************************************************/
/*!
    @brief Time in seconds since the last GPS date was obtained. The time
    returned is limited to 2^32 milliseconds, which is about 49.7 days. It
    will wrap around to zero if no GPS date is received for this long.
    @return nmea_float_t value in seconds since last GPS date.
*/
/**************************************************************************/
nmea_float_t Adafruit_GPS::secondsSinceDate() {
  return (time(NULL) - lastDate) / 1000.;
}

/**************************************************************************/
/*!
    @brief Fakes time of receipt of a sentence. Use between build() and parse()
    to make the timing look like the sentence arrived from the GPS.
*/
/**************************************************************************/
void Adafruit_GPS::resetSentTime() { sentTime = time(NULL); }

/**************************************************************************/
/*!
    @brief Checks whether a string starts with a specified prefix
    @param str Pointer to a string
    @param prefix Pointer to the prefix
    @return True if str starts with prefix, false otherwise
*/
/**************************************************************************/
static bool strStartsWith(const char *str, const char *prefix) {
  while (*prefix) {
    if (*prefix++ != *str++)
      return false;
  }
  return true;
}
