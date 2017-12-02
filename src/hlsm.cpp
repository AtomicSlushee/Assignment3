#include "hlsm.h"
#include "scheduler.h"
#include "variable.h"
#include "type.h"
#include "ioclass.h"
#include <string>

HLSM::HLSM()
{
}

// from Bit Twiddling Hacks at
// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned HLSM::NextHighestPowerOfTwo(unsigned v)
{
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

// from Bit Twiddling Hacks at
// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
unsigned HLSM::BitsToRepresent( unsigned v )
{
  unsigned r = 0;
  while( v >>= 1) r++;
  return r;
}

int HLSM::CtoHLSM( graphType& g, graphType& hlsm, Variables& modelVars, graphType::ScheduleID id )
{
  // gain access to the master classes
  Scheduler& scheduler = Singleton<Scheduler>::instance();
  Types& types = Singleton<Types>::instance();
  IOClasses& ios = Singleton<IOClasses>::instance();

  // declare a map
  Scheduler::partitionMap_t m;
  // build a map from which we'll generate the HLSM
  int numStates = scheduler.buildPartTimeMap( m, g, id );

  // add the state variable, with just enough bits
  int width = BitsToRepresent( NextHighestPowerOfTwo( numStates ));
  std::string stype = "UInt" + std::to_string(width);
  if( !types.isType(stype)) types.addType( false, width );
  Type& type = types.getType(stype);
  IOClass& ioc = ios.getIOClass("variable");
  /*Variable& state = */modelVars.addVariable(Variables::nameState(), type, ioc);

  // Turns out the tree is sufficient, and needs no additional processing.
  hlsm = g;

  return numStates;
}
