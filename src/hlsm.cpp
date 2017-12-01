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

int HLSM::CtoHLSM( graphType& g, graphType& hlsm, Variables& modelVars )
{
  int states = 0;

  // IN-WORK ------------------------------------------------------------------

  // gain access to the master classes
  Scheduler& scheduler = Singleton<Scheduler>::instance();
  Types& types = Singleton<Types>::instance();
  IOClasses& ios = Singleton<IOClasses>::instance();

  // declare a map
  Scheduler::partitionMap_t m;
  // build a map from which we'll generate the HLSM
  scheduler.buildPartTimeMap( m, g, graphType::ASAP );

  // TODO: gotta figureout how to get number of states and such;
  //       for now, just use max time slot as a placeholder
  int numStates = std::prev(std::prev(m.end())->second.end())->first + 2;

  // add the state variable, with just enough bits
  int width = NextHighestPowerOfTwo( numStates );
  std::string stype = "UInt" + std::to_string(width);
  if( !types.isType(stype)) types.addType( false, width );
  Type& type = types.getType(stype);
  IOClass& ioc = ios.getIOClass("variable");
  Variable& state = modelVars.addVariable(Variables::nameState(), type, ioc);

  // TODO: walk through the tree and generate the program output
  // Rough cut: just copy everything and see what happens
  hlsm = g;

  // IN-WORK ------------------------------------------------------------------

  /*HACK*/ return 1; /*HACK*/
  return states;
}
