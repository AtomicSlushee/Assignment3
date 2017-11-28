#include "scheduler.h"
#include "graphType.hpp"
#include "operator.h"
#include "vertex.hpp"

Scheduler::Scheduler():hlsmTools(Singleton< HLSM >::instance())
{
}

bool Scheduler::process(Statements& input, Statements& output, int latencyConstraint)
{
  graphType g;
  graphType::vertices_t topo;

  g.createWeightedGraph( input );
  g.topologicalSort(topo);
  double lp = g.longestPath(topo, graphType::SCHEDULING);
  ASAP(g);
  ALAP(g, latencyConstraint);
  dumpScheduledGraph( g, graphType::ASAP);
  dumpScheduledGraph( g, graphType::ALAP);

  hlsmTools.CtoHLSM( g, output );

  return true;
}

void Scheduler::ASAP(graphType& g)
{
  // reset all asap times to zero
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    v->get().helper.schedTime[graphType::ASAP] = NOT_SCHEDULED;
  }

  // keep cycling until all nodes scheduled
  while(true)
  {
    bool done = true; // if we get through the for loop without scheduling, we're done
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
    {
      // if this node has already been scheduled, skip it
      if( v->get().helper.schedTime[graphType::ASAP] > NOT_SCHEDULED ) continue;

      done = false; // nope, still scheduling

      // does this node have any predecessors at all?
      if( v->get().getLinksFrom().empty() )
      {
        v->get().helper.schedTime[graphType::ASAP] = 1; // no, schedule in the first round
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
          int latency = p->get().getNode().get().getStatement()->scheduleLatency();
          // see if the predecessor has been scheduled
          if( p->get().helper.schedTime[graphType::ASAP] <= NOT_SCHEDULED )
          {
            // nope, can't schedule this vertex
            allClear = false;
            break; // don't need to check other predecessors
          }
          else
          {
            // compute this node's start time based on predecessor schedule and latency
            int nextStart = p->get().helper.schedTime[graphType::ASAP] + latency;
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
          v->get().helper.schedTime[graphType::ASAP] = maxStart;
        }
      }
    }
    if( done ) break; // we're done
  }
}

void Scheduler::ALAP(graphType& g, int latencyConstraint)
{
  // reset all alap times to latency constraint
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
	  v->get().helper.schedTime[graphType::ALAP] = NOT_SCHEDULED;
  }

  // keep cycling until all nodes scheduled
  bool done = false;
  while(!done)
  {
	  done = true; // if we get through the for loop without scheduling, we're done

      for( auto v = g.getGraph().rbegin(); v != g.getGraph().rend(); v++) // using reverse iterators
      {
        // if this node has already been scheduled, skip it
        if( v->get().helper.schedTime[graphType::ALAP] > NOT_SCHEDULED ) continue;

        done = false; // nope, still scheduling

        // does this node have any successors?
        if( v->get().getLinksTo().empty() )
        {
        	v->get().helper.schedTime[graphType::ALAP] = latencyConstraint; // schedule at end
        }
        else
        {
            // check to see if all successors are scheduled
            int minStart = latencyConstraint; // will tell us when to start if all clear
            bool allClear = true;             // will tell us if it is all clear

            // loop through all this node's successors
            for( auto p = v->get().getLinksTo().begin(); p != v->get().getLinksTo().end(); p++)
            {
              // grab the latency of the successors statement operation
              int latency = p->get().getNode().get().getStatement()->scheduleLatency();
              // see if the successor has been scheduled
              if( p->get().helper.schedTime[graphType::ALAP] <= NOT_SCHEDULED )
              {
                // nope, can't schedule this vertex
                allClear = false;
                break; // don't need to check other successors
              }
              else
              {
                // compute this node's start time based on successor schedule and latency
                int nextStart = p->get().helper.schedTime[graphType::ALAP] - latency;
                if( nextStart < minStart )
                {
                  minStart = nextStart; // save the min
                }
              }
            }

            // see if all successors are scheduled
            if( allClear )
            {
              // yes, schedule this node
              v->get().helper.schedTime[graphType::ALAP] = minStart;
            }
        }
      }
  }
}

void Scheduler::FDS(graphType& g)
{
}

void Scheduler::buildPartTimeMap(partitionMap_t& m, graphType& g, graphType::ScheduleID s)
{
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    int part = v->get().helper.partition;
    int stime = v->get().helper.schedTime[s];
    m[part][stime].push_back( v->get().getVertex() );
  }
}

void Scheduler::dumpScheduledGraph(graphType& g, graphType::ScheduleID s)
{
  std::cout << std::endl << "[partition][time] C_code" << std::endl;
  partitionMap_t m;
  buildPartTimeMap(m,g,s);
  for( auto i = m.begin(); i != m.end(); i++)
  {
    for( auto j = i->second.begin(); j != i->second.end(); j++)
    {
      for( vertList_t::iterator k = j->second.begin(); k != j->second.end(); k++ )
      {
        graphType::vertex_t* p = *k;
        std::cout << "[" << i->first << "][" << j->first << "] " << p->getNode().get().C_format() << std::endl;
      }
    }
  }
}
