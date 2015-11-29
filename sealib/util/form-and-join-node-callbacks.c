
#include "config.h"
#include "ember.h"
#include "form-and-join.h"
#include "network.h"

void emberNetworkFoundHandler ( EmberZigbeeNetwork *networkFound )
{
    int8u lqi;
    int8s rssi;
    
    emberGetLastHopLqi(&lqi);
    emberGetLastHopRssi(&rssi);
    emberFormAndJoinNetworkFoundHandler(networkFound, lqi, rssi);
}

void emberScanCompleteHandler ( int8u channel, EmberStatus status )
{
    emberFormAndJoinScanCompleteHandler(channel, status);
}

void emberEnergyScanResultHandler ( int8u channel, int8s rssi )
{
    emberFormAndJoinEnergyScanResultHandler(channel, rssi);
}

