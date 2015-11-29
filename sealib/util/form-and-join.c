
#include "config.h"

#define __FORM_AND_JOIN_C__

#include "ember-types.h"
#include "error.h"
#include "hal.h" 
#include "mem.h"
#include "serial.h"
#include "form-and-join.h"
#include "form-and-join-adapter.h"
#include "sysinfo.h"
#include "network.h"

//////////////////////////////////////////////////////////////////////////////////////////
// We can't include ember.h or ezsp.h from this file since it is used
// for both host and node-size libraries.  However the emberStartScan()
// API is identical in both.
extern EmberStatus emberStartScan ( int8u scanType, int32u channelMask, int8u duraiton );

extern panid_t panid_info[];

enum {
  FORM_AND_JOIN_NOT_SCANNING,
  FORM_AND_JOIN_NEXT_NETWORK,
  FORM_AND_JOIN_ENERGY_SCAN,      
  FORM_AND_JOIN_PAN_ID_SCAN,      
  FORM_AND_JOIN_JOINABLE_SCAN,    
  FORM_AND_JOIN_DUAL_CHANNEL_SCAN 
};

#define SCAN_START    0x00
#define SCAN_DUAL     0x01
#define SCAN_COMPLETE 0x02
#define SCAN_LEISURE  0x03

#if 1
static int8u formAndJoinScanType = FORM_AND_JOIN_NOT_SCANNING;

static int8u *dataContents;
#define panIdCandidates ((int16u *)dataContents)
#define channelEnergies (dataContents)

static int8u extendedPanIdCache[EXTENDED_PAN_ID_SIZE];
static int8u scanStatus = SCAN_LEISURE;
static int8u  scanFail;
static int8u  scanWait;
static int8u  currentChannel;
static int8u  lastChannel;

static int8u  scanSingnal;

static int8u  scanChannel[18];
static int8u  channelOut;
static int8u  channelIn;
static int8u  readError;


static int8u channelCache;
//static int8u lastChannelCache = 0;
static boolean ignoreExtendedPanId;

int8u networkCount;  // The number of NetworkInfo records.

NetworkInfo2 networkInfo2[MAXNETBUF];

#else
typedef struct
{
    u8   type;
    u8   channel;
    u8   ignore;
    u8   count;
    u32  mask;
    u8 * content;    
    u8   expanid[EXTENDED_PAN_ID_SIZE];
} formjion_t;

static formjion_t formjion1 = { FORM_AND_JOIN_NOT_SCANNING, 0x00, 0x00, 0x00, 0x00, 0x00, };

#define panIdCandidates ((int16u *)formjion1.content)
#define channelEnergies (formjion1.content)

#endif

//////////////////////////////////////////////////////////////////////////////////////////
// The dual channel code is only present on the EM250 and EM260.
boolean emberEnableDualChannelScan = TRUE;

//////////////////////////////////////////////////////////////////////////////////////////
// The minimum significant difference between energy scan results.
// Results that differ by less than this are treated as identical.
#define ENERGY_SCAN_FUZZ        (25)
#define NUM_PAN_ID_CANDIDATES   (16)          // Must fit into one packet buffer on the node
#define INVALID_PAN_ID          (0xFFFF)

//////////////////////////////////////////////////////////////////////////////////////////
// ZigBee specifies that active scans have a duration of 3 (138 msec).
// See documentation for emberStartScan in include/network-formation.h
// for more info on duration values.
#define ACTIVE_SCAN_DURATION    (3)

//////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations for the benefit of the compiler.
static void saveNetwork    ( EmberZigbeeNetwork *network, int8u lqi, int8s rssi );
static boolean setup       ( int8u scanType );
static void startScan      ( EmberNetworkScanType type, int32u mask, int8u duration );
static void startPanIdScan ( void );

//////////////////////////////////////////////////////////////////////////////////////////
// Finding unused PAN ids.
void emberScanForUnusedPanId ( int32u channelMask, int8u duration )
{
    if (setup(FORM_AND_JOIN_ENERGY_SCAN)) 
    {
        sea_memset(channelEnergies, 0xFF, EMBER_NUM_802_15_4_CHANNELS);
        startScan(EMBER_ENERGY_SCAN, channelMask, duration);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Pick a channel from among those with the lowest energy and then look for
// a PAN ID not in use on that channel.
// 
// The energy scans are not particularly accurate, especially as we don't run
// them for very long, so we add in some slop to the measurements and then pick
// a random channel from the least noisy ones.  This avoids having several
// coordinators pick the same slightly quieter channel.
static void energyScanComplete ( void )
{
    int8u cutoff = 0xFF;
    int8u candidateCount = 0x00;
    int8u channelIndex;
    int8u i;

    for (i = 0x00; i < EMBER_NUM_802_15_4_CHANNELS; i ++) 
    {
        if (channelEnergies[i] < cutoff - ENERGY_SCAN_FUZZ) 
        {
            cutoff = channelEnergies[i] + ENERGY_SCAN_FUZZ;
        }
    }
    for (i = 0x00; i < EMBER_NUM_802_15_4_CHANNELS; i ++)   // There must be at least one channel, so there will be at least one candidate.
    {
        if (channelEnergies[i] < cutoff) 
        {
            candidateCount += 0x01;
        }
    }
    channelIndex = halCommonGetRandom() % candidateCount;

    for (i = 0x00; i < EMBER_NUM_802_15_4_CHANNELS; i ++) 
    {
        if (channelEnergies[i] < cutoff) 
        {
            if (channelIndex == 0x00) 
            {
                channelCache = EMBER_MIN_802_15_4_CHANNEL_NUMBER + i;
                break;
            }
            channelIndex -= 0x01;
        }
    }
    startPanIdScan();
}

#if 0
//////////////////////////////////////////////////////////////////////////////////////////
//
static boolean expanisnull ( int8u * array, int8u size )
{
    for (int8u i = 0x00; i < size; i ++) 
        if (array[i] != 0x00) 
            return FALSE;
    return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// PAN IDs can be 0..0xFFFE.  We pick some trial candidates and then do a scan
// to find one that is not in use.
static void startPanIdScan ( void )
{
    int8u i;

    for (i = 0x00; i < NUM_PAN_ID_CANDIDATES;) 
    {
        int16u panId = halCommonGetRandom() & 0xFFFF;
        if (panId != 0xFFFF) 
        {
            panIdCandidates[i] = panId;
            i++;
        }
    }
    formAndJoinScanType = FORM_AND_JOIN_PAN_ID_SCAN;
    startScan(EMBER_ACTIVE_SCAN, BIT32(channelCache), ACTIVE_SCAN_DURATION);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Form a network using one of the unused PAN IDs.  If we got unlucky we
// pick some more and try again.
static void panIdScanComplete ( void )
{
    int8u i;

    for (i = 0x00; i < NUM_PAN_ID_CANDIDATES; i ++) 
    {
        EmberStatus status = EMBER_SUCCESS;
        if (panIdCandidates[i] != 0xFFFF) 
        {
            emberUnusedPanIdFoundHandler(panIdCandidates[i], channelCache);
            emberFormAndJoinCleanup(status);
            return;
        }
    }
    startPanIdScan();     // Start over with new candidates.
}

boolean isScanRunning(void)
{
    if(scanStatus==SCAN_START||scanStatus==SCAN_DUAL)
        return TRUE;
    else
        return FALSE;
}

boolean isScanComplete(void)
{
    if(scanStatus==SCAN_COMPLETE)
        return TRUE;
    else
        return FALSE;
}

void scanPrintStatus( void )
{
    switch(scanStatus)
    {
        case SCAN_START:
          Debug("[status]SCAN_START");
          break;
        case SCAN_DUAL:
          Debug("[status]SCAN_DUAL");
          break;
        case SCAN_COMPLETE:
          Debug("[status]SCAN_COMPLETE");
          break;
        case SCAN_LEISURE:
          Debug("[status]SCAN_LEISURE");
          break;
        default:
          Debug("[status]unknow status");
          break;
    }
}

int8u readArray( void )
{
    int8u temp;
    if( channelIn == channelOut )
    {
        readError = 1;
        return 0;
    }
    else
    {
        temp = scanChannel[channelOut];
        channelOut ++;
        channelOut %= 18;
        return temp;
    }
}

void writeArray( int8u temp )
{
    scanChannel[channelIn] = temp;
    channelIn ++;
    channelIn %= 18;
}

void scanErrorHandler( void )
{
    scanSingnal = 1;
    writeArray( currentChannel );
}

void scanInitial( void )
{
    int8u i;
    scanOpenTimeOutEvent();             
    Debug("\r\n[scan]scan Initial");
    
    for(i = 0;i < 16;i++)
    {
        scanChannel[i] = i + 11;
    }
    channelOut = 0;
    channelIn = 16;
    readError = 0;
    
    scanFail = 0;   
    scanWait = 0;
    
    networkCount = 0;
    
    currentChannel = 0;
    lastChannel = 0;
    
    formAndJoinScanType = FORM_AND_JOIN_JOINABLE_SCAN;
    
    scanStatus = SCAN_DUAL;
    
    scanSingnal = 1;        // 1 scan avail
    
    sea_memcpy(extendedPanIdCache, DEF_EXPAN, EXTENDED_PAN_ID_SIZE);
                                   
    ignoreExtendedPanId = 1;  //!!
    
    emberScanForNextJoinableNetwork();
}

void scanDual( void )
{
    if( readError == 1 )
    {
        scanStatus = SCAN_COMPLETE;
        scanCloseTimeOutEvent();
        return;
    }
    
    if( scanFail >= 3 )    
    {
        halReboot();
    }
    
    Debug("\r\n[scan]currentChannel =%d, lastChannel =%d",currentChannel,lastChannel);
    if( lastChannel == currentChannel )
    { 
        
        scanWait ++;
        Debug("\r\n[scan]scan error,times = %d",scanWait);
    }
    else
    {
        lastChannel = currentChannel;
        scanWait = 0;
    }
    
    if( scanWait >= 2 )   
    {
        scanWait = 0;
        scanFail++;
        Debug("[scan]goto scan continue");
        scanErrorHandler();
        emberScanForNextJoinableNetwork();
    } 
}

void scanSet( void )
{
    if( scanStatus == SCAN_COMPLETE || scanStatus == SCAN_LEISURE )
    {
        scanStatus = SCAN_START;
    }
    else
    {
        Debug("\r\n[scan]scan is running, could not start");
    }
}

int8u scaningStatus( void )
{
    return scanStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Finding joinable networks
void emberScanForJoinableNetwork ( int32u channelMask, int8u* extendedPanId )
{
    switch (scanStatus) 
    {
        case SCAN_START:
          Debug("\r\n[scan]goto scan_start");
          scanInitial();
          break;
          
        case SCAN_DUAL:
          Debug("\r\n[scan]goto scan_dual");
          scanDual();
          break;
          
        case SCAN_COMPLETE:
          break;
          
        case SCAN_LEISURE:
          break;
          
        default:
          break;
    }
    /*
    Debug("\r\nemberScanForJoinableNetwork START!!");
    if (!setup(FORM_AND_JOIN_NEXT_NETWORK)) 
        return;
    Debug("\r\n emberScanForJoinableNetwork END!!");
    channelCache = EMBER_MIN_802_15_4_CHANNEL_NUMBER - 0x01;  // Init the channel to 10, gets incremented in call to next joinable network.
    channelMaskCache = channelMask;
    if (extendedPanId == NULL || expanisnull(extendedPanId, EXTENDED_PAN_ID_SIZE)) 
        ignoreExtendedPanId = TRUE;
    else 
    {
        ignoreExtendedPanId = FALSE;
        sea_memcpy(extendedPanIdCache, extendedPanId, EXTENDED_PAN_ID_SIZE);
    }
    emberScanForNextJoinableNetwork();*/
}


//////////////////////////////////////////////////////////////////////////////////////////
void emberScanForNextJoinableNetwork ( void )
{
  /*Debug("\r\nemberScanForNextJoinableNetwork START!!  formAndJoinScanType = %d", formAndJoinScanType);
    if (formAndJoinScanType != FORM_AND_JOIN_NEXT_NETWORK) 
    {
        emberScanErrorHandler(EMBER_INVALID_CALL);
        return;
    }
    while (networkCount && (channelCache == EMBER_MAX_802_15_4_CHANNEL_NUMBER))     
    {
        NetworkInfo *finger = formAndJoinGetNetworkPointer(0);  
        if (finger->network.panId != 0xFFFF) 
        {
            formAndJoinSetBufferLength(0);
            formAndJoinSetCleanupTimeout();
            return;
        }
        formAndJoinSetBufferLength(0);
    }
    channelCache += 0x01;  // Find the next channel in the mask and start scanning.
    for (; channelCache <= EMBER_MAX_802_15_4_CHANNEL_NUMBER; channelCache ++) 
    {
        int32u bitMask = BIT32(channelCache);
        if (bitMask & channelMaskCache) 
        {
                 
            if(lastChannelCache == channelCache)
            {
                network_info.scanError++;
            }
            else
            {
                network_info.scanError = 0;
            }
            lastChannelCache = channelCache;
            formAndJoinScanType = FORM_AND_JOIN_JOINABLE_SCAN;

            Debug("\r\n[scan]: start channelCache %d ...", channelCache);
            startScan(EMBER_ACTIVE_SCAN, bitMask, ACTIVE_SCAN_DURATION);
            Debug("completed!\n");
            return;
        }
    }
    emberFormAndJoinCleanup(EMBER_NO_BEACONS);  */
    int8u scanEnable = 0;
    ATOMIC(
        if( scanSingnal == 1 )
        {
            scanSingnal = 0;
            scanEnable = 1;
        }
    )
    
    if( scanEnable == 1 )
    {
        Debug("\r\n[scan]scan TRUE,scan");
        scanEnable = 0;
    }
    else
    {
        Debug("\r\n[scan]scan FALSE,return");
        return;
    }

      
    currentChannel = readArray();
    
    if( currentChannel < 11 ||currentChannel > 26 )
    {
        return;
    }
    
    Debug("\r\n[scan]startScan channel =%d",currentChannel);
    startScan(EMBER_ACTIVE_SCAN, BIT32(currentChannel), ACTIVE_SCAN_DURATION);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
boolean emberFormAndJoinScanCompleteHandler ( int8u channel, EmberStatus status )
{ 
    Debug("\r\n[scan]CompleteHandler");
    if (status != EMBER_SUCCESS) 
    {
        emberFormAndJoinCleanup(status);
        return TRUE;
    }
    
    int8u completeTrue = 0;
    
    ATOMIC(
        if( scanSingnal == 0 )
        {
            scanSingnal = 1;
            completeTrue = 1;
        }
    )
      
    if(completeTrue == 1)
    {
        completeTrue = 0;
        Debug("\r\n[scan]complete TRUE,continue");
    }
    else
    {
        Debug("\r\n[scan]complete FALSE,return");
        return FALSE;
    }
      
    switch (formAndJoinScanType) 
    {
        case FORM_AND_JOIN_ENERGY_SCAN:    
            energyScanComplete();    
            break;
        case FORM_AND_JOIN_PAN_ID_SCAN:    
            panIdScanComplete();     
            break;
        case FORM_AND_JOIN_JOINABLE_SCAN: 
/*#if defined(XAP2B) || defined(EZSP_HOST) || defined(EMBER_TEST)
            if (emberEnableDualChannelScan && (channelCache < 15 || channelCache > 22) && networkCount) 
            {
                int8u dualChannel = (channelCache + (channelCache < 15 ? 12 : -12));
                formAndJoinScanType = FORM_AND_JOIN_DUAL_CHANNEL_SCAN;
                startScan(EMBER_ACTIVE_SCAN, BIT32(dualChannel), ACTIVE_SCAN_DURATION);
                break;
            }
#endif      // If no dual scan, fall through to save a little flash.
        case FORM_AND_JOIN_DUAL_CHANNEL_SCAN:   */
            //formAndJoinScanType = FORM_AND_JOIN_NEXT_NETWORK;
            emberScanForNextJoinableNetwork(); 
            break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// We are either looking for PAN IDs or for joinable networks.  In the first
// case we just check the found PAN ID against our list of candidates. 
boolean emberFormAndJoinNetworkFoundHandler ( EmberZigbeeNetwork *networkFound, int8u lqi, int8s rssi )
{
    int8u i;
  
    Debug("\r\nNetworkFoundHandler ");
    switch (formAndJoinScanType) 
    {
        case FORM_AND_JOIN_PAN_ID_SCAN:
            for (i = 0x00; i < NUM_PAN_ID_CANDIDATES; i ++)
                if (panIdCandidates[i] == networkFound->panId)
            panIdCandidates[i] = 0xFFFF;
            break;
    
        case FORM_AND_JOIN_JOINABLE_SCAN:    // check for a beacon with permit join on...the same stack profile as this application...
        {                                    // ...a matching Extended PAN ID...and ignore the Extended PAN ID, or...
            u8 status, index;
            
            status = formAndJoinStackProfile();
            Debug("\r\nfound network allowingJoin %02x, stackProfile %02x, formAndJoinStackProfile %02x", 
                       networkFound->allowingJoin, networkFound->stackProfile, status);
             //if (networkFound->allowingJoin && networkFound->stackProfile == formAndJoinStackProfile()
             //   && (ignoreExtendedPanId || (MEMCOMPARE(extendedPanIdCache, networkFound->extendedPanId, EXTENDED_PAN_ID_SIZE) == 0x00))) 
              if (networkFound->stackProfile == status && (ignoreExtendedPanId || 
                 (sea_memcomp(extendedPanIdCache, networkFound->extendedPanId, EXTENDED_PAN_ID_SIZE) == 0x00))) 
            {
                index = sea_findchannelindex(networkFound->channel, networkFound->panId);
                if (index < MAXCHANNEL)
                {
                    Debug("\r\n[SCAN]: nwk found ch %d, panid %04x, lqi %d, rssi %d, xpan %s", 
                               networkFound->channel, networkFound->panId, lqi, rssi, networkFound->extendedPanId);
                    if (sys_info.channel.mask & (0x01 << index) && networkFound->channel != 26)
                    {
                        Debug("\r\n[SCAN]: save this channel %d network information", networkFound->channel); 
                        saveNetwork(networkFound, lqi, rssi);
                    }
                }
                else
                    Debug("\r\n[SCAN]: can't find channel index %d", index);
            } 
            break;
        }
#if defined(XAP2B) || defined(EZSP_HOST) || defined(EMBER_TEST)
        case FORM_AND_JOIN_DUAL_CHANNEL_SCAN: 
        {
            NetworkInfo *finger;
            for (i = 0x00; i < networkCount; i ++) 
            {
                finger = formAndJoinGetNetworkPointer(i);
                if (MEMCOMPARE(finger->network.extendedPanId, networkFound->extendedPanId, EXTENDED_PAN_ID_SIZE) == 0x00) 
                {
                    if (lqi > finger->lqi) 
                    {
                        finger->network.panId = 0xFFFF;  // Invalid network;
                    }
                }
            }
            break;
        }
#endif
    }
    return emberFormAndJoinIsScanning();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Just remember the result.
boolean emberFormAndJoinEnergyScanResultHandler ( int8u channel, int8s maxRssiValue )
{
    Debug("\r\nemberFormAndJoinEnergyScanResultHandler RUN!!");
    if (emberFormAndJoinIsScanning()) 
    {
        Debug("\r\n[scan]: found energy %d dBm on channel 0x%2x", (s16)maxRssiValue, (u16)channel);
        channelEnergies[channel - EMBER_MIN_802_15_4_CHANNEL_NUMBER] = maxRssiValue;
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
static void saveNetwork ( EmberZigbeeNetwork *network, int8u lqi, int8s rssi )
{
    int8u i;
    //NetworkInfo *finger;
    
    Debug("\r\nsaveNetwork RUN!!");
    for (i = 0x00; i < networkCount; i ++)   // See if we already have that network.
    {
        //finger = formAndJoinGetNetworkPointer(i);
        
        if ((int8u)(networkInfo2[i].channel)==(int8u)(network->channel)
            &&(int16u)(networkInfo2[i].panid)==(int16u)(network->panId)) 
            return;
    }
  
    //if (formAndJoinSetBufferLength(networkCount + 0x01) != EMBER_SUCCESS) 
    //    return;
   
    for (i = 0x00; i < 16; i++)
    {
        if(network->channel == panid_info[i].channel&&network->panId == panid_info[i].panid)
          break;
    }
    
    //finger = formAndJoinGetNetworkPointer(networkCount);
    networkInfo2[networkCount].lqi   = lqi;
    networkInfo2[networkCount].rssi  = rssi;
    networkInfo2[networkCount].index = i; 
    networkInfo2[networkCount].channel = network->channel; 
    networkInfo2[networkCount].panid = network->panId; 
#if 1
    networkCount ++;
    //sea_memcpy(finger, network, sizeof(EmberZigbeeNetwork));
    //finger->lqi  = lqi;
    //finger->rssi = rssi;
#else    
    sea_memcpy(&networkInfo2[networkCount].net, network, sizeof(EmberZigbeeNetwork));
    if (networkCount ++ >= MAXNETBUF - 0x01)
        networkCount = 0x00;
#endif    
}

//////////////////////////////////////////////////////////////////////////////////////////
boolean emberFormAndJoinIsScanning ( void )
{
    return (formAndJoinScanType > FORM_AND_JOIN_NEXT_NETWORK);
}

//////////////////////////////////////////////////////////////////////////////////////////
static boolean setup ( int8u scanType )
{
    if (emberFormAndJoinIsScanning()) 
    {
        emberScanErrorHandler(EMBER_MAC_SCANNING);
        return FALSE;
    }
    emberFormAndJoinCleanup(EMBER_SUCCESS);  // In case we were in NEXT_NETWORK mode.
    networkCount        = 0x00;    
    formAndJoinScanType = scanType;
    dataContents        = formAndJoinAllocateBuffer();
    if (dataContents == NULL) 
    {
        emberFormAndJoinCleanup(EMBER_NO_BUFFERS);
        return FALSE;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
void emberFormAndJoinCleanup ( EmberStatus status )
{
    //formAndJoinScanType = FORM_AND_JOIN_NOT_SCANNING;
    //  formAndJoinReleaseBuffer();                                                                         
    if (status != EMBER_SUCCESS) 
        emberScanErrorHandler(status);
}

//////////////////////////////////////////////////////////////////////////////////////////
static void startScan ( EmberNetworkScanType type, int32u mask, int8u duration )
{
    EmberStatus status = emberStartScan(type, mask, duration);
    if (status != EMBER_SUCCESS) 
      return;
        //emberFormAndJoinCleanup(status);
} 

//////////////////////////////////////////////////////////////////////////////////////////
