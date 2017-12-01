#ifndef __hlsm_h__
#define __hlsm_h__

#include "singleton.h"
#include "statement.h"
#include "graphType.hpp"

class HLSM
{
  friend class Singleton<HLSM>;
public:
  unsigned NextHighestPowerOfTwo(unsigned v);
  int CtoHLSM( graphType& g, Statements& hlsm, Variables& modelVars );
private:
  HLSM();
};

#endif//__hlsm_h__
