
#include "config.h"
#include "ember.h"

//-- Build structure defines (these define all the data types of the tokens).
#define DEFINETYPES
  #include "token-stack.h"
#undef DEFINETYPES

//-- Build parameter links
#define DEFINETOKENS
#define TOKEN_MFG(name,creator,iscnt,isidx,type,arraysize,...)

#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  creator,
  const int16u tokenCreators[] = {
    #include "token-stack.h"
  };
#undef TOKEN_DEF

#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  iscnt,
  const boolean tokenIsCnt[] = {
    #include "token-stack.h"
  };
#undef TOKEN_DEF

#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  sizeof(type),
  const int8u tokenSize[] = {
    #include "token-stack.h"
  };
#undef TOKEN_DEF

#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  arraysize,
  const int8u tokenArraySize[] = {
    #include "token-stack.h"
  };
#undef TOKEN_DEF

//These set of DEFAULT token values are only used in the tokenDefaults array
//below.
#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  const type TOKEN_##name##_DEFAULTS = __VA_ARGS__;
  #include "token-stack.h"
#undef TOKEN_DEF

#define TOKEN_DEF(name,creator,iscnt,isidx,type,arraysize,...) \
  ((void *)&TOKEN_##name##_DEFAULTS),
  const void * tokenDefaults[] = {
    #include "token-stack.h"
  };
#undef TOKEN_DEF

#undef DEFINETOKENS

#undef TOKEN_MFG

