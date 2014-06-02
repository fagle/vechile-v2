/**
 * @file trust-center.h
 * @brief EmberZNet security API
 * See @ref security for documentation
 *
 * <!--Copyright 2007 by Ember Corporation. All rights reserved.    *80*-->
 */

/**
 * @addtogroup trust_center
 *
 * This file describes the routines used by the Trust Center to manage
 * devices in the network. The Trust center decides whether to use 
 * preconfigured keys or not and manages passing out keys to joining
 * and rejoining devices.  The Trust Center also sends out new keys
 * and decides when to start using them.
 *
 * See trust-center.h for source code
 * @{
 */
#ifndef __TRUST_CENTER_H__
#define __TRUST_CENTER_H__

///////////////////////////////////////////////////////////////////////////////////
/** @brief A Trust Center device configuration bitmask example.
 *    The Trust Center is expected to be setup with a Network Key
 *    Preconfigured Link Key that is global throughout all devices
 *    on the Network.  The decision whether or not to send the key
 *    in-the-clear is NOT controlled through this bitmask.  That
 *    is controlled via the emberTrustCenterJoinHandler(...) function.
 */
#define EMBER_FORM_TRUST_CENTER_NETWORK_BITMASK  \
  ( EMBER_STANDARD_SECURITY_MODE                 \
    | EMBER_GLOBAL_LINK_KEY                      \
    | EMBER_HAVE_NETWORK_KEY                     \
    | EMBER_HAVE_PRECONFIGURED_KEY )

/** @brief A coordinator device configuration bitmask example.
 *    The coordinator is expected to be setup with a Network Key
 *    and a Preconfigured Link Key that is global throughout all devices
 *    on the Network.  The decision whether or not to send the key
 *    in-the-clear is decentralized, and each individual router
 *    can make this decision via the emberTrustCenterJoinHandler(...) function.
 */
#define EMBER_FORM_DISTRIBUTED_TRUST_CENTER_NETWORk_BITMASK   \
  ( EMBER_STANDARD_SECURITY_MODE                              \
    | EMBER_GLOBAL_LINK_KEY                                   \
    | EMBER_DISTRIBUTED_TRUST_CENTER_MODE                     \
    | EMBER_HAVE_NETWORK_KEY                                  \
    | EMBER_HAVE_PRECONFIGURED_KEY )

/** @brief This function broadcasts a new encryption key, but
 * does not tell the nodes in the network to start using it.
 *
 * To tell nodes to switch to the new key, use ::emberBroadcastNetworkKeySwitch().
 * This is only valid for the Trust Center/Coordinator.
 * It is not valid when operating in Distributed Trust Center mode.
 *
 * It is up to the application to determine how quickly to send
 * the Switch Key after sending the alternate encryption key.
 * The factors to consider are the polling rate of sleepy end devices,
 * and the buffer size of their parent nodes.  Sending too quickly
 * may cause a sleepy end device to miss the Alternate Encryption Key
 * and only get the Switch Key message, which means it will be unable
 * to change to the new network key.
 *  
 * @param key  A pointer to a 16-byte encryption key 
 * (EMBER_ENCRYPTION_KEY_SIZE).  A NULL (or all zero key) may be passed in,
 * which will cause the stack to randomly generate a new key.
 *
 * @return An ::EmberStatus value that indicates the success
 * or failure of the command.
 */
EmberStatus emberBroadcastNextNetworkKey(EmberKeyData* key);

/** @brief This function broadcasts a switch key message to
 * tell all nodes to change to the sequence number of the previously
 * sent Alternate Encryption Key.
 *
 * This function is only valid for the Trust Center/Coordinator,
 * and will also cause the Trust Center/Coordinator to change its Network
 * Key.  It is not valid when operating in Distributed Trust Center mode.
 *
 * @return An ::EmberStatus value that indicates the success or failure
 * of the command.
*/
#if defined DOXYGEN_SHOULD_SKIP_THIS
EmberStatus emberBroadcastNetworkKeySwitch(void);
#else
EmberStatus emSendNetworkKeySwitch(EmberNodeId address);

#define emberBroadcastNetworkKeySwitch() \
  emSendNetworkKeySwitch(EMBER_SLEEPY_BROADCAST_ADDRESS)
#endif

/** @brief A callback that allows the application running on the Trust
  * Center (which is the coordinator for ZigBee networks) to control
  * which nodes are allowed to join the network. If the node is allowed to join,
  * the trust center must decide how to send it the Network Key, encrypted
  * or unencrypted.
  * 
  * A default handler is provided and its behavior is as follows.
  * A status of ::EMBER_DEVICE_SECURED_REJOIN means that the device has the 
  * Network Key, no action is required from the Trust Center.   A status of 
  * ::EMBER_DEVICE_LEFT also requires no action.  In both cases ::EMBER_NO_ACTION
  * is returned.
  *
  * When operating in a network with a Trust Center and there is a Global
  * Link Key configured, ::EMBER_USE_PRECONFIGURED_KEY will be returned which 
  * means the Trust Center is using a pre-configured Link Key.  The Network Key
  * will be sent to the joining node encrypted with the Link Key.  If a Link Key
  * has not been set on the Trust Center, ::EMBER_DENY_JOIN is returned.
  *
  * The ::EMBER_ASK_TRUST_CENTER decision has been deprecated.  This function
  * will not be called for a router or end device when operating in a Network
  * With a Trust Center.
  *
  * If the device is a router in a network that is operating in a Distributed 
  * Trust Center Security mode, then the handler will be called by the stack.
  *
  * The default handler in a Distributed Trust Center Security mode network is 
  * as follows:
  * If the router received an encrypted Network Key when it joined, then a
  * pre-configured Link key will be used to send the Network Key Encrypted to
  * the joining device (::EMBER_USE_PRECONFIGURED_KEY).  If the router received 
  * the Network Key in the clear, then it will also send the key in the clear to
  * the joining node (::EMBER_SEND_KEY_IN_THE_CLEAR).
  *
  * @param newNodeId       The node id of the device wishing to join.
  * @param newNodeEui64    The EUI64 of the device wishing to join.
  * @param status          The EmberUpdateDeviceStatus indicating whether the device
  *   is joining/rejoining or leaving.
  * @param parentOfNewNode The node id of the parent of device wishing to join.
  *
  * @return ::EMBER_USE_PRECONFIGURED_KEY to allow the node to join without sending it the key.
  * ::EMBER_SEND_KEY_IN_THE_CLEAR to allow the node to join and send it the key.
  * ::EMBER_DENY_JOIN to reject the join attempt.
  * value should not be returned if the local node is itself the trust center).
  */
EmberJoinDecision emberTrustCenterJoinHandler(EmberNodeId newNodeId,
                                              EmberEUI64 newNodeEui64,
                                              EmberDeviceUpdate status,
                                              EmberNodeId parentOfNewNode);

/** @brief This function causes a coordinator to become the Trust Center
 *    when it is operating in a network that is not using one.  It will
 *    send out an updated Network Key to all devices that will indicate a
 *    transition of the network to now use a Trust Center.  The Trust Center
 *    should also switch all devices to using this new network key with a call
 *    to emberBroadcastNetworkKeySwitch().  
 *
 * @param newNetworkKey  The key data for the Updated Network Key.
 *
 * @return An ::EmberStatus value that indicates the success or failure
 * of the command.
 */
EmberStatus emberBecomeTrustCenter(EmberKeyData* newNetworkKey);


/** @brief This variable controls the policy that the Trust Center uses
 *  for determining whether to allow or deny requests for Trust Center
 *  link keys.
 *
 *  The following is a good set of guidelines for TC Link key requests:
 *    - If preconfigured TC link keys are setup on devices, requests
 *      for the TC key should never be allowed (::EMBER_DENY_KEY_REQUESTS).
 *    - If devices request link keys during joining (i.e. join in the clear
 *      and set ::EMBER_GET_LINK_KEY_WHEN_JOINING) then it is advisable to
 *      allow requesting keys from the TC for a short
 *      period of time (e.g. the same amount of time "permit joining" is turned
 *      on).  Afterwards requests for the TC link key should be denied.
 */
extern EmberLinkKeyRequestPolicy emberTrustCenterLinkKeyRequestPolicy;

/** @brief This variable controls the policy that the Trust Center uses 
 *  for determining whether to allow or deny requests for application 
 *  link keys between device pairs.  When a request is received and the
 *  policy is ::EMBER_ALLOW_KEY_REQUESTS, the TC will generate
 *  a random key and send a copy to both devices encrypted with their
 *  individual link keys.
 *
 *  Generally application link key requests may always be allowed.
 */
extern EmberLinkKeyRequestPolicy emberAppLinkKeyRequestPolicy;

///////////////////////////////////////////////////////////////////////////////////
#endif  // __TRUST_CENTER_H__
