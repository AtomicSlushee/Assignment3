#ifndef __scheduler_h__
#define __scheduler_h__

#include "singleton.h"
#include "statement.h"

class Scheduler
{
  friend class Singleton< Scheduler > ;

private:
  Scheduler();

public:
  bool process(Statements& input, Statements& output);

private:
};

#endif//__scheduler_h__
