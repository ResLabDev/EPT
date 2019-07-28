//==========================
// Common Layer Functions
//==========================

#include "common.h"

// String copy function
void stringCopy (char *target, char *source)
{
    while (*source)
    {
        *target = *source;
        target++;
        source++;
    }

    *target = '\0';     // Closing the string
}
