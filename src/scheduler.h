#ifndef __scheduler_h__
#define __scheduler_h__

#include "singleton.h"
#include "statement.h"
#include "graphType.hpp"
#include "hlsm.h"

class Scheduler
{
  friend class Singleton< Scheduler > ;

private:
  Scheduler();

public:
  bool process(Statements& input, Statements& output);
  void ASAP(graphType& g);
  void ALAP(graphType& g);
  void FDS(graphType& g);

  static const int NOT_SCHEDULED = -1;

private:
  HLSM& hlsmTools;
};

#endif//__scheduler_h__
