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
  bool process(Statements& input, graphType& output, int latencyConstraint, Variables& modelVars, graphType::ScheduleID id);
  void ASAP(graphType& g);
  void ALAP(graphType& g, int latencyConstraint);
  void FDS(graphType& g, int latencyConstraint);

  static const int NOT_SCHEDULED = -1;

  typedef std::list<graphType::vertex_t*> vertList_t;
  typedef std::map<int,vertList_t> timeMap_t;
  typedef std::map<int,timeMap_t> partitionMap_t;

  int buildPartTimeMap(partitionMap_t& m, graphType& g, graphType::ScheduleID s);
  void dumpScheduledGraph(graphType& g, graphType::ScheduleID s);
  void updateTypeDistributions(graphType& g, int latencyConstraint);

private:

  std::vector<float> ad;
  std::vector<float> md;
  std::vector<float> dd;
  std::vector<float> ld;
  std::vector<float> NOP;

  HLSM& hlsmTools;

};

#endif//__scheduler_h__
