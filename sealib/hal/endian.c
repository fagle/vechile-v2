
#include "config.h"
#include "endian.h"
#include "ember-types.h"

// Endian conversion APIs.  
// Network byte order is big endian, so these APIs are only necessary on 
//  platforms which have a natural little endian byte order.  On big-endian
//  platforms, the APIs are macro'd away to nothing

// Network to Host Short
int16u NTOHS ( int16u val )
{
    int8u *fldPtr;

    fldPtr = (int8u *)&val;
    return (HIGH_LOW_TO_INT(*fldPtr, *(fldPtr+1)));
}

// Network to Host Long
int32u NTOHL ( int32u val )
{
    int8u *fldPtr;
    int16u field;
    int16u field2;

    fldPtr = (int8u *)&val;
    field = HIGH_LOW_TO_INT(*fldPtr, *(fldPtr+1));
    field2 = HIGH_LOW_TO_INT(*(fldPtr+2), *(fldPtr+3));
    return ((int32u)field << 16) | ((int32u)field2);
}

//////////////////////////////////////////////////////////////////////////////////////////


