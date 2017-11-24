#ifndef __hlsm_h__
#define __hlsm_h__

#include "singleton.h"
#include "statement.h"

class HLSM
{
  friend class Singleton<HLSM>;
public:
  int CtoHLSM( Statements& c, Statements& hlsm );
private:
  HLSM();
};

#endif//__hlsm_h__
