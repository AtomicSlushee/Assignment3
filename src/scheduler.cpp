#include "scheduler.h"
#include "graphType.hpp"
#include "operator.h"

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
  // reset all asap times to zero
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    v->get().helper.asapTime = 0;
  }

  // keep cycling until all nodes scheduled
  while(true)
  {
    bool done = true; // if we get through the for loop without scheduling, we're done
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
    {
      // if this node has already been scheduled, skip it
      if( v->get().helper.asapTime ) continue;

      done = false; // nope, still scheduling

      // does this node have any predecessors at all?
      if( v->get().getLinksFrom().empty() )
      {
        v->get().helper.asapTime = 1; // no, schedule in the first round
      }

      else
      {
        // check to see if all predecessors scheduled, and while you're at it,
        // look for the max latency among them
        int maxStart = 0;       // will tell us when to start if all clear
        bool allClear = true;   // will tell us if it is all clear
        // loop through all this node's predecessors
        for( auto p = v->get().getLinksFrom().begin(); p != v->get().getLinksFrom().end(); p++)
        {
          // grab the latency of the previous statement operation
          int latency = ScheduleLatency(*(p->get().getNode().get().getStatement()));
          // see if the predecessor has been scheduled
          if( p->get().helper.asapTime == 0 )
          {
            // nope, can't schedule this vertex
            allClear = false;
            break; // don't need to check other predecessors
          }
          else
          {
            // compute this node's start time based on predecessor schedule and latency
            int nextStart = p->get().helper.asapTime + latency;
            if( nextStart > maxStart )
            {
              maxStart = nextStart; // save the max
            }
          }
        }
        // see if all predecessors are scheduled
        if( allClear )
        {
          // yes, schedule this node
          v->get().helper.asapTime = maxStart;
        }
      }
    }
    if( done ) break; // we're done
  }
}

void Scheduler::ALAP(graphType& g)
{
}

void Scheduler::FDS(graphType& g)
{
}

int Scheduler::ScheduleLatency( Statement& stmt )
{
  int latency = 0;

  if( stmt.isAssignment() )
  {
    switch( stmt.assignment().getOperator().id() )
    {
      case Operator::MUL:
        latency = 2;
        break;
      case Operator::DIV:
      case Operator::MOD:
        latency = 3;
        break;
      default:
        latency = 1;
    }
  }
  else if( stmt.isIfStatement() )
  {
    latency = 1;
  }

  return latency;
}
