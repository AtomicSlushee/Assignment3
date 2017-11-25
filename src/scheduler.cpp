#include "scheduler.h"

Scheduler::Scheduler():hlsmTools(Singleton< HLSM >::instance())
{
}

bool Scheduler::process(Statements& input, Statements& output)
{
  graphType g;
  graphType::vertices_t topo;

  g.createWeightedGraph( input );
  g.topologicalSort(topo);
  double lp = g.longestPath(topo, graphType::UNITY);

  hlsmTools.CtoHLSM( g, output );

  return true;
}

void Scheduler::ASAP(graphType& g)
{
}

void Scheduler::ALAP(graphType& g)
{
}

void Scheduler::FDS(graphType& g)
{
}
