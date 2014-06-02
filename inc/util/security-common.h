// *******************************************************************
//  security-common.h
//
//  Common functions for manipulating security for Trust Center and
//  non Trust Center devices.
//
//  Copyright 2008 by Ember Corporation. All rights reserved.              *80*
// *******************************************************************
#ifndef __SECURITY_COMMON_H__
#define __SECURITY_COMMON_H__

#if defined EZSP_HOST
  // These are normally provided natively by the 250.
  boolean emberHaveLinkKey(EmberEUI64 remoteDevice);
  #define emberKeyContents(key) ((key)->contents)
#endif

boolean nodeSecurityInit ( EmberKeyData* preconfiguredKey );

#endif    // __SECURITY_COMMON_H__

