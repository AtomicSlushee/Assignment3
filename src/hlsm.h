#ifndef __hlsm_h__
#define __hlsm_h__

#include "singleton.h"
#include "statement.h"
#include "graphType.hpp"

class HLSM
{
  friend class Singleton<HLSM>;
public:
  int CtoHLSM( graphType& g, Statements& hlsm );
private:
  HLSM();
};

#endif//__hlsm_h__
