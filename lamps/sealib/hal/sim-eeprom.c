#include "config.h"
#include "ember.h"
#include "error.h"
#include "hal.h"
#include "sim-eeprom.h"

#ifdef CORTEXM3
  //On the Cortex-M3 chips we define a variable that holds the actual
  //SimEE storage and then place this storage at the proper location
  //in the linker.
  __root __no_init int8u simulatedEepromStorage[SIMEE_SIZE_B] @ __SIMEE__;
  //sim-eeprom-internal.c uses a set of defines that parameterize its behavior.
  //Since the -internal file is precompiled, we must externally define the
  //parameters (as constants) so an application build can alter them.
  const int8u VIRTUAL_PAGES = ((SIMEE_SIZE_HW / FLASH_PAGE_SIZE_HW) / 2);
  const int16u LEFT_BASE = SIMEE_BASE_ADDR_HW;
  const int16u LEFT_TOP = ((SIMEE_BASE_ADDR_HW + (FLASH_PAGE_SIZE_HW *
                          ((SIMEE_SIZE_HW / FLASH_PAGE_SIZE_HW) / 2))) - 1);
  const int16u RIGHT_BASE = (SIMEE_BASE_ADDR_HW + (FLASH_PAGE_SIZE_HW *
                            ((SIMEE_SIZE_HW / FLASH_PAGE_SIZE_HW) / 2)));
  const int16u RIGHT_TOP = (SIMEE_BASE_ADDR_HW + (SIMEE_SIZE_HW - 1));
  const int16u ERASE_CRITICAL_THRESHOLD = (SIMEE_SIZE_HW / 4);
#endif //CORTEXM3

const int16u ID_COUNT = TOKEN_COUNT;

#define DEFINETOKENS
//Manufacturing tokens do not exist in the SimEEPROM -define to nothing
#define TOKEN_MFG(name,creator,iscnt,isidx,type,arraysize,...)

//If the arraysize is exactly 1, we need only a single entry in the ptrCache
//to hold this entry.  If the arraysize is not 1, we need +1 to hold
//the redirection entry in the Primary Table and then arraysize entries in
//the indexed table.  This works for all values of arraysize.
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  + arraysize + ((arraysize==1) ? 0 : 1)
  //value of all index counts added together
  const int16u PTR_CACHE_SIZE = 0
    #include "token-stack.h"
  ;
  //the ptrCache definition - used to provide efficient access, based upon
  //ID and index, to the freshest data in the Simulated EEPROM.
  int16u ptrCache[
    #include "token-stack.h"
  ];
#undef TOKEN_DEF

//The Size Cache definition.  Links the token to its size for efficient
//calculations.  Indexed by compile ID.  Size is in words.  This Cache should
//be used for all size calculations.  halInternalSimEeInit will guarantee that
//the compiled sizes defining this cache match the stored sizes.
const int8u sizeCache[] = {
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  (BYTES_TO_WORDS(sizeof(type))                                        \
     + (iscnt ? BYTES_TO_WORDS(COUNTER_TOKEN_PAD) : 0)),
    #include "token-stack.h"
 };
#undef TOKEN_DEF

#undef TOKEN_MFG
#undef DEFINETOKENS

EmberStatus halInternalSimEeStartupCore(boolean forceRebuildAll, 
                                        int8u *lookupCache);


EmberStatus halInternalSimEeStartup(boolean forceRebuildAll)
{
  // lookupCache must be declared here, outside the library so that it can
  //  be appropriately sized based on the number of tokens the application
  //  uses.
  int8u lookupCache[TOKEN_COUNT];
  return halInternalSimEeStartupCore(forceRebuildAll, lookupCache);
}
