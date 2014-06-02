/**
 * @file cbke-crypto-engine.h
 * @brief EmberZNet Smart Energy security API.
 * See @ref smart_energy_security for documention.
 * 
 * <!--Copyright 2008 by Ember Corporation. All rights reserved.    *80*-->
 */

/**
 * @addtogroup smart_energy_security
 *
 * This file describes functionality for Certificate Based Key Exchange (CBKE).
 * This is used by Smart Energy devices to generate and store ephemeral ECC
 * keys, derive the SMACs for the Key establishment protocol, and sign messages
 * using their private key for the Demmand Response Load Control client cluster.
 *
 * See cbke-crypto-engine.h for source code.
 * @{
 */

/** @brief Retrieves the implicit certificate stored in the MFG tokens of the
 *  device.
 *
 * @param result A pointer to an ::EmberCertificateData structure where the 
 * retrieved certificate will be stored.
 *
 * @return ::EMBER_SUCCESS if the certificate was successfully retrieved.
 *   ::EMBER_ERR_FATAL if the token contains uninitialized data.
 */
EmberStatus emberGetCertificate(EmberCertificateData* result);

/** @brief This function begins the process of generating an ephemeral
 *  public/private ECC key pair.  
 *
 *  If no other ECC operation is going on,
 *  it will immediately return with ::EMBER_OPERATION_IN_PROGRESS.  It will
 *  delay a period of time to let APS retries take place, but then it will
 *  shutdown the radio and consume the CPU processing until the key generation
 *  is complete.  This may take up to 1 second.
 *
 *  The generated results of the key generation is returned 
 *  via ::emberGenerateCbkeKeysHandler().
 *
 * @return ::EMBER_OPERATION_IN_PROGRESS if the stack has queued up the
 *   operation for execution. 
 */
EmberStatus emberGenerateCbkeKeys(void);

/** @brief This function will begin the process of generating the shared
 *  secret, the new link key, and the Secured Message Authentication Code (SMAC).
 *
 *  If no other ECC operation is going on, it will immediately return with
 *  ::EMBER_OPERATION_IN_PROGRESS.  It will delay a period of time to let APS
 *  retries take place, but then it will shutdown the radio and consume the
 *  CPU processing until SMACs calculations are complete.  This may take up to
 *  3.5 seconds.
 *
 *  The calculated SMACS are returned via ::emberCalculateSmacsHandler().
 *
 * @param amInitiator This boolean indicates whether or not the device is the
 *   one that initiated the CBKE with the remote device, or whether it was the
 *   responder to the exchange.
 * @param partnerCert A pointer to an ::EmberCertificateData structure that
 *   contains the CBKE partner's implicit certificate.
 * @param partnerEphemeralPublicKey A pointer to an ::EmberPublicKeyData 
 *   structure that contains the CBKE partner's ephemeral public key.
 *
 * @return ::EMBER_OPERATION_IN_PROGRESS if the stack has queued up the
 *   operation for execution. 
 */
EmberStatus emberCalculateSmacs(boolean amInitiator,
                                EmberCertificateData* partnerCert, 
                                EmberPublicKeyData* partnerEphemeralPublicKey);

/** @brief This function should be called when all CBKE operations are done.
 *  Any temporary data created during calls to emberGenerateCbkeKeys() or
 *  emberCalculateSmacs() is wiped out.  If the local device has validated
 *  that the partner device has generated the same SMACS as itself, it should
 *  set 'storeLinkKey' to TRUE.  Otherwise it should pass in FALSE.
 *
 * @param storeLinkKey This tells the stack whether to store the newly
 *   generated link key, or discard it.
 *
 * @return If storeLinkkey is FALSE, then this function returns ::EMBER_ERR_FATAL
 *   always.  If storeLinkKey is TRUE, then this function returns the results
 *   of whether or not the link key was stored.  ::EMBER_SUCCESS is
 *   returned when key was stored successfully.
 */
EmberStatus emberClearTemporaryDataMaybeStoreLinkKey(boolean storeLinkKey);

/* @brief LEGACY FUNCTION:  This functionality has been replaced by a single
 * bit in the ::EmberApsFrame, ::EMBER_APS_OPTION_DSA_SIGN.  Devices wishing
 * to send signed messages should use that as it requires fewer function calls
 * and message buffering.  emberDsaSignHandler() is still called when using
 * ::EMBER_APS_OPTION_DSA_SIGN is used.  However this function is still 
 * supported.
 *
 * This function begins the process of signing the passed message
 * contained within the buffer.  If no other ECC operation is going on,
 * it will immediately return with ::EMBER_OPERATION_IN_PROGRESS.  
 * It will delay a period of time to let APS retries take place, but then it 
 * will shutdown the radio and consume the CPU processing until the signing 
 * is complete.  This may take up to 1 second.
 *
 * The signed message will be returned in ::emberDsaSignHandler().
 *
 * @param messageToSign The message buffer where the complete message contained.
 * 
 * @return ::EMBER_OPERATION_IN_PROGRESS if the stack has queued up the
 * operation for execution.
 */
EmberStatus emberDsaSign(EmberMessageBuffer messageToSign);


/* @brief This function is an application callback that must be defined
 * when using CBKE.  It is called when the ephemeral key generation operation
 * is complete.  The newly generated public key is passed back to the 
 * application to be sent to the CBKE partner over-the-air.  Internally
 * the stack saves the public and private key pair until it the function
 * ::emberClearTemporaryDataMaybeStoreLinkKey() is called by the application.
 *
 * @param status This is the ::EmberStatus value indicating the success or 
 *   failure of the operation.
 * @param ephemeralPublicKey A pointer to an ::EmberPublicKeyData structure
 *   containing the newly generated public key.
 */
void emberGenerateCbkeKeysHandler(EmberStatus status,
                                  EmberPublicKeyData* ephemeralPublicKey);

/* @brief This function is an application callback that must be defined
 * when using CBKE.  It is called when the shared secret generation is
 * complete and the link key and SMACs have been derived.  The link key is
 * stored in a temporary location until the application decides to
 * store or discard the key by calling
 * ::emberClearTemporaryDataMaybeStoreLinkKey().  
 *
 * @param status This is the ::EmberStatus value indicating the success or 
 *   failure of the operation.
 * @param initiatorSmac This is a pointer to the ::EmberSmacData structure
 *   to the initiator's version of the SMAC.
 * @param responderSmac This is a pointer to the ::EmberSmacData structure
 *   to the responder's version of the SMAC.
 */
void emberCalculateSmacsHandler(EmberStatus status,
                                EmberSmacData* initiatorSmac,
                                EmberSmacData* responderSmac);

/* @brief This function is an application callback that must be defined
 * when using CBKE.  This callback is provided to the application to let
 * it know that the ECC operations have completed and the radio has been turned
 * back on.  When using the sign-and-send option of the ::EmberApsFrame, 
 * ::EMBER_APS_OPTION_DSA_SIGN, the handler will NOT return the complete
 * signed message.  This callback is merely informative.  If ::emberDsaSign() 
 * has been called, the message plus signature will be returned to the caller
 * and it must be sent separately by one of the message send primitives
 * (such as ::emberSendUnicast()).
 * 
 * @param status This is the ::EmberStatus value indicating the success or 
 *   failure of the operation.
 * @param signedMessage This is the ::EmberMessageBuffer indicating the newly
 *   signed message, if ::emberDsaSign() was called.  If message was signed
 *   using ::EMBER_APS_OPTION_DSA_SIGN then this will be
 *    ::EMBER_NULL_MESSAGE_BUFFER.
 */
void emberDsaSignHandler(EmberStatus status, 
                         EmberMessageBuffer signedMessage);


/* @brief This function is used to update the Smart Energy certificate,
 * CA public key, and local private key that the device uses for CBKE.
 * The preferred method for adding certificates is to pre-install them
 * in MFG tokens when the chip is manufactured.  However this function
 * allows the certificate to be updated at runtime after the device has
 * been deployed.  To use this functionality the application must also set
 * the stack configuration value ::EMBER_CERTIFICATE_TABLE_SIZE to 1.
 * Attempts to call this function with ::EMBER_CERTIFICATE_TABLE_SIZE of 0
 * will return ::EMBER_SECURITY_CONFIGURATION_INVALID.
 * The passed security data will be persistently stored in stack tokens.
 *
 * The certificate contains the EUI64 it is associated with.  If that
 * EUI64 matches the EUI64 currently in use by the device, this
 * function may be called at any time, even while running in a network.
 * If the EUI64 does not match, this function may only be called when the
 * network is in a state of ::EMBER_NO_NETWORK.  Attempts to do otherwise
 * will result in a return value of ::EMBER_INVALID_CALL.
 * 
 * @note The device will immediately and persistently <b>change its EUI64
 * to match the value in the certificate</b>. 
 *
 * @param caPublic A pointer to the CA public key data that will be stored
 *   in stack tokens.
 * @param myCert A pointer to the certificate data that will be stored in
 *   stack tokens.
 * @param mykey A pointer to the private key data that will be stored in
 *   stack tokens.
 * @return The ::EmberStatus value indicating success or failure of the 
 *   operation.
 */
EmberStatus emberSetPreinstalledCbkeData(EmberPublicKeyData* caPublic,
                                         EmberCertificateData* myCert,
                                         EmberPrivateKeyData* myKey);

/* @brief This function retrieves the EUI64 from the stack token 
 * Smart Energy Certificate (it does not examine the MFG token certificate)
 * and returns the value in the "Subject" field (the EUI64) to the caller.
 * If no stack token is set, the ::EMBER_CERTIFICATE_TABLE_SIZE is zero
 * or if the CBKE library is not present, this function returns FALSE
 * and the EUI64 for the return value is not set.
 *
 * @param certEui64 The location of the return value for the EUI64.
 * @return TRUE if the stack token certificate is set and the EUI64
 *   return value is valid.  FALSE otherwise.
 */
boolean emberGetStackCertificateEui64(EmberEUI64 certEui64);

/** @} // END addtogroup
 */


