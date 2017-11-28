#ifndef __scheduler_h__
#define __scheduler_h__

#include "singleton.h"
#include "statement.h"
#include "graphType.hpp"
#include "hlsm.h"
#include "hlsyn.h"
#include <map>
#include <list>

class Scheduler
{
  friend class Singleton< Scheduler > ;

private:
  Scheduler();

public:
  bool process(Statements& input, Statements& output, int latencyConstraint);
  void ASAP(graphType& g);
  void ALAP(graphType& g, int latencyConstraint);
  void FDS(graphType& g);

  static const int NOT_SCHEDULED = -1;

  typedef std::list<graphType::vertex_t*> vertList_t;
  typedef std::map<int,vertList_t> timeMap_t;
  typedef std::map<int,timeMap_t> partitionMap_t;

  void buildPartTimeMap(partitionMap_t& m, graphType& g, graphType::ScheduleID s);
  void dumpScheduledGraph(graphType& g, graphType::ScheduleID s);

private:
  HLSM& hlsmTools;
};

#endif//__scheduler_h__
