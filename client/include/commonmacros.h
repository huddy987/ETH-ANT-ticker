// Contains common non-hardware macros

#ifndef COMMONMACROS_H
#define COMMONMACROS_H

#include <Arduino.h>
#include "hardwareconfig.h"

/////////////////////////////////////////////////////////
// Build type dependant macros
/////////////////////////////////////////////////////////
#ifdef DEBUG // Debug mode macros

#define DEBUG_LED_BLINK                                 \
{                                                       \
    digitalWrite(DEBUG_LED, LOW);                       \
    delay(250);                                         \
    digitalWrite(DEBUG_LED, HIGH);                      \
    delay(250);                                         \
}

// This implementation waits for USB CDC to become available before continuing.
// On PCA10056 this is the nRF USB port
#define SERIAL_BEGIN(baud)                              \
{                                                       \
    Serial.begin(baud);                                 \
    while(!Serial)                                      \
    {                                                   \
        DEBUG_LED_BLINK                                 \
    }                                                   \
}
#define SERIAL_PRINT(...)   Serial.print(__VA_ARGS__);
#define SERIAL_PRINTLN(...) Serial.println(__VA_ARGS__);

/* Print file and line number on assert */
/* TODO: WE SHOULDNT BE INLINING THIS EVERYWHERE */
#define ASSERT()                                          \
{                                                         \
    char message[40] = {0};                               \
    sprintf(message, "Assert %s:%d", __FILE__, __LINE__); \
    Serial.println(message);                              \
}
#else // Release mode macros
#define DEBUG_LED_BLINK {}
#define SERIAL_PRINTLN(...) {}
#define SERIAL_BEGIN(baud)   {}
#define SERIAL_PRINT(...)   {}
#define SERIAL_PRINTLN(...) {}
#define ASSERT()
#endif // DEBUG

/////////////////////////////////////////////////////////
// Build type agnostic macros
/////////////////////////////////////////////////////////
// Macro for checking API error codes. Assumes 0 is success.
#define ERROR_CODE_CHECK(ulErrCode)                     \
{                                                       \
    if (err_code != 0)                                  \
    {                                                   \
        return err_code;                                \
    }                                                   \
}

/* First populated byte in ANT message containing prices */
#define ANT_MESSAGE_START_IDX 3

#endif // COMMONMACROS_H
