#include "hlsyn.h"
#include "scheduler.h"
#include "graphType.hpp"
#include "operator.h"
#include "vertex.hpp"
#include <algorithm>


Scheduler::Scheduler()
: hlsmTools( Singleton< HLSM >::instance() )
{
}

bool Scheduler::process( Statements& input, graphType& output, int latencyConstraint, Variables& modelVars,
                        graphType::ScheduleID id )
{
  graphType g;
  graphType::vertices_t topo;
  
  g.createWeightedGraph( input );
  g.topologicalSort( topo );
  
  //  double lp = g.longestPath(topo, graphType::SCHEDULING);
  
  // always do ASAP
  if( true )
  {
    ASAP( g );
    if( DEBUG_ENABLED )
      dumpScheduledGraph( g,graphType::ASAP );
  }
  
  if( id > graphType::ASAP )
  {
    // check states in ASAP against given latency
    partitionMap_t m;
    // build a map from which we'll get the number of ASAP states
    int maxTimeSlot = buildPartTimeMap( m, g, graphType::ASAP ) - 1;
    if( maxTimeSlot > latencyConstraint )
    {
      std::cout << "WARNING: the given latency constraint of " << latencyConstraint << " is less than the ASAP schedule; compensating." << std::endl;
      latencyConstraint = maxTimeSlot-1;
    }
  }
  
  if( id != graphType::ASAP )
  {
    ALAP( g,latencyConstraint );
    if( DEBUG_ENABLED )
      dumpScheduledGraph( g,graphType::ALAP );
  }
  
  if( id == graphType::FDS )
  {
    
    FDS( g,latencyConstraint );
    if( DEBUG_ENABLED )
      dumpScheduledGraph( g,graphType::FDS );
  }
  
  hlsmTools.CtoHLSM( g,output,modelVars,id );
  
  return true;
}

void Scheduler::ASAP( graphType& g )
{
  // reset all asap times to zero
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++ )
  {
    v->get().helper.schedTime[graphType::ASAP] = NOT_SCHEDULED;
  }
  
  // keep cycling until all nodes scheduled
  while( true )
  {
    bool done = true; // if we get through the for loop without scheduling, we're done
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++ )
    {
      // if this node has already been scheduled, skip it
      if( v->get().helper.schedTime[graphType::ASAP] > NOT_SCHEDULED )
        continue;
      
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
        for( auto p = v->get().getLinksFrom().begin(); p != v->get().getLinksFrom().end(); p++ )
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
    if( done )
      break; // we're done
  }
}

void Scheduler::ALAP( graphType& g, int latencyConstraint )
{
  // reset all alap times to latency constraint
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++ )
  {
    v->get().helper.schedTime[graphType::ALAP] = NOT_SCHEDULED;
    v->get().helper.color = graphType::BLACK;
  }
  g.getGraph().rbegin()->get().helper.color = graphType::WHITE; // special math needed for the sink node
  
  // keep cycling until all nodes scheduled
  bool done = false;
  while( !done )
  {
    done = true; // if we get through the for loop without scheduling, we're done
    
    for( auto v = g.getGraph().rbegin(); v != g.getGraph().rend(); v++ ) // using reverse iterators
    {
      // if this node has already been scheduled, skip it
      if( v->get().helper.schedTime[graphType::ALAP] > NOT_SCHEDULED )
      {
        continue;
      }
      
      done = false; // nope, still scheduling
      
      // does this node have any successors?
      if( v->get().getLinksTo().empty() )
      {
        v->get().helper.schedTime[graphType::ALAP] = latencyConstraint + 1; // schedule at end
      }
      else
      {
        // check to see if all successors are scheduled
        int minStart = latencyConstraint; // will tell us when to start if all clear
        bool allClear = true;             // will tell us if it is all clear
        
        // loop through all this node's successors
        for( auto s = v->get().getLinksTo().begin(); s != v->get().getLinksTo().end(); s++ )
        {
          // grab the latency of the successors statement operation
          int latency;
          if( s->get().helper.color == graphType::WHITE )
            latency = 1; // special case to keep the sink node at a later time
          else
            latency = v->get().getNode().get().getStatement()->scheduleLatency();
          // see if the successor has been scheduled
          if( s->get().helper.schedTime[graphType::ALAP] <= NOT_SCHEDULED )
          {
            // nope, can't schedule this vertex
            allClear = false;
            break; // don't need to check other successors
          }
          else
          {
            // compute this node's start time based on successor schedule and current node latency
            int nextStart = s->get().helper.schedTime[graphType::ALAP] - latency;
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

void Scheduler::FDS(graphType& g, int latencyConstraint)
{
  // to make life easier, ignore the sink nodes
  auto firstNode = std::next(g.getGraph().begin());
  auto sinkNode = std::prev(g.getGraph().end());

  // Minimize resources under a latency constraint
  
  int Node_Time_Interval = 0;
  int timestep; // for iterating through times
  
  // Start with everything unscheduled
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    v->get().helper.schedTime[graphType::FDS] = NOT_SCHEDULED;
  }
  
#if 0 // not required; already done earlier in the 'process' method
  // Get the left and right edges for each node
  ASAP(g); // for left edge
  ALAP(g, latencyConstraint); // for right edge
#endif
  
  // first, compute time frames for every node.
  // The time frame is just the earliest a node can be scheduled (ASAP)
  // followed by the latest (ALAP)
  
  for( auto v = firstNode; v != sinkNode; v++)
  {
    // The width of the timeframe for every node is the ALAP - ASAP + 1
    v->get().leftEdge = v->get().helper.schedTime[graphType::ASAP];
    v->get().rightEdge = v->get().helper.schedTime[graphType::ALAP];
  }
  
  // for each node in the graph
  for( auto v = firstNode; v != sinkNode; v++)
  {
    //                   ALAP time               ASAP time
    Node_Time_Interval = v->get().rightEdge - v->get().leftEdge + 1;

    // compute the operations and type probabilities
    // Operational Probability is easy. just 1/timewidth for times in the range. 0 else.
    
    for (timestep = 1; timestep <= latencyConstraint; timestep++)
    {
      if (timestep >= v->get().leftEdge && timestep <= v->get().rightEdge)
      {
        float prob = 1.0/Node_Time_Interval;
        v->get().opProb.push_back(prob);
      }
      else
      {
        v->get().opProb.push_back(0.0);
      }
      
      
    }
#if DEBUGPRINTS
    std::cout << "Operational Probabilities [cycle][probability]" << std::endl;
    
    int time = 1;
    for (auto t = v->get().opProb.begin(); t != v->get().opProb.end(); t++)
    {
      std::cout << "[" << time++ << "][" << *t << "]" << std::endl;
    }
#endif
  }
  // now compute the type distribution
  // Type Distribution is the sum of probabilities of the operations implemented by a specific
  // resource at any time step of interest
  
  std::vector<float> ad;
  std::vector<float> md;
  std::vector<float> dd;
  std::vector<float> ld;
  std::vector<float> NOP;
  // set them all to 0.0 first
  for ( timestep = 0 ; timestep < latencyConstraint; timestep++)
  {
    ad.push_back(0.0);
    md.push_back(0.0);
    dd.push_back(0.0);
    ld.push_back(0.0);
    NOP.push_back(0.0);
  }
  
  for( auto v = firstNode; v != sinkNode; v++)
  {
    // for each node sum the probabilities that the resource is being used in that time step
    for (int timestep = 0; timestep < latencyConstraint; timestep++)
    {
      if (v->get().getNode().get().getResource() == Statement::ADDER_SUB)
      {
        std::vector<float>::iterator it = v->get().opProb.begin();
        std::advance(it,timestep);
        ad[timestep] += *it;
      }
      if (v->get().getNode().get().getResource() == Statement::MULTIPLIER)
      {
        std::vector<float>::iterator it = v->get().opProb.begin();
        std::advance(it,timestep);
        md[timestep] += *it;
      }
      if (v->get().getNode().get().getResource() == Statement::DIV_MOD)
      {
        std::vector<float>::iterator it = v->get().opProb.begin();
        std::advance(it,timestep);
        dd[timestep] += *it;
      }
      if (v->get().getNode().get().getResource() == Statement::LOGICAL)
      {
        std::vector<float>::iterator it = v->get().opProb.begin();
        std::advance(it,timestep);
        ld[timestep] += *it;
      }
      
    }
  }
  // for each node in the graph take note of the associated resource list
  for( auto v = firstNode; v != sinkNode; v++)
  {
    if (v->get().getNode().get().getResource() == Statement::ADDER_SUB)
    {
      v->get().ResourceTypeDistribution = ad;
    }
    else if (v->get().getNode().get().getResource() == Statement::MULTIPLIER)
    {
      v->get().ResourceTypeDistribution = md;
    }
    else if (v->get().getNode().get().getResource() == Statement::DIV_MOD)
    {
      v->get().ResourceTypeDistribution = dd;
    }
    else if (v->get().getNode().get().getResource() == Statement::LOGICAL)
    {
      v->get().ResourceTypeDistribution = ld;
    }
    else
    {
      // NOP
      v->get().ResourceTypeDistribution = NOP;
    }
  }
#if DEBUGPRINTS
  std::cout << " ADDER/SUBTRACTOR USE Probabilities for each timestep" << std::endl;
  std::cout << " [cycle][Probability of use]" << std::endl;
  for ( timestep = 1; timestep <= latencyConstraint; timestep++)
  {
    std::cout << " [" << timestep << "]" << "[" << ad[timestep -1] << "]" << std::endl;
  }
  
  std::cout << " MULTIPLIER USE Probabilities for each timestep" << std::endl;
  std::cout << " [cycle][Probability of use]" << std::endl;
  for ( timestep = 1; timestep <= latencyConstraint; timestep++)
  {
    std::cout << " [" << timestep << "]" << "[" << md[timestep -1] << "]" << std::endl;
  }
  std::cout << " DIVIDER/MOD USE Probabilities for each timestep" << std::endl;
  std::cout << " [cycle][Probability of use]" << std::endl;
  for ( timestep = 1; timestep <= latencyConstraint; timestep++)
  {
    std::cout << " [" << timestep << "]" << "[" << dd[timestep -1] << "]" << std::endl;
  }
  std::cout << " LOGICAL USE Probabilities for each timestep" << std::endl;
  std::cout << " [cycle][Probability of use]" << std::endl;
  for ( timestep = 1; timestep <= latencyConstraint; timestep++)
  {
    std::cout << " [" << timestep << "]" << "[" << ld[timestep -1] << "]" << std::endl;
  }
#endif
  
  // for each node, compute the self force, pred force, and succ force for each timestep
  for( auto v = firstNode; v != sinkNode; v++)
  {
    
    std::cout<< "\n\n-----------" << "Forces for node " << v->get().getNodeNumber() << "--------"  << std::endl;
    std::cout << "Node " << v->get().getNodeNumber() << " leftEdge " << v->get().leftEdge << " rightEdge " << v->get().rightEdge << " Timewidth " << Node_Time_Interval << std::endl;
    for (timestep = v->get().leftEdge; timestep <= v->get().rightEdge; timestep++)
    {
      float selfForce         = 0.0;
      float SuccessorForce    = 0.0;
      float PredecessorForce  = 0.0;

      selfForce = v->get().ComputeSelfForce(timestep);
      
      std::cout << " Self Force " << selfForce <<std::endl;
      
      SuccessorForce = v->get().ComputeSuccessorForceForTimeSlot(timestep);
      
      std::cout << " Successor Force " << SuccessorForce <<std::endl;
      
      PredecessorForce = v->get().ComputePredecessorForceForTimeSlot(timestep);
      
      std::cout << " Predecessor Force " << PredecessorForce <<std::endl;

      //;
      v->get().TotalForce.insert(std::pair<float,int>(selfForce + SuccessorForce + PredecessorForce, timestep));
      
      std::cout << "The total force for node " << v->get().getNodeNumber() << " is " << selfForce + SuccessorForce + PredecessorForce  << " at cycle " << timestep  << "\n\r ------- \n\r"<< std::endl;
    }

    
    v->get().helper.schedTime[graphType::FDS] = v->get().TimeWithMinimumForce();
  }

  // last step: clean up to help the state machine output code
  g.getGraph().begin()->get().helper.schedTime[graphType::FDS] = 0;
  sinkNode->get().helper.schedTime[graphType::FDS] = latencyConstraint + 1;

}

int Scheduler::buildPartTimeMap( partitionMap_t& m, graphType& g, graphType::ScheduleID s )
{
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++ )
  {
    int part = v->get().helper.partition;
    int stime = v->get().helper.schedTime[s];
    m[part][stime].push_back( v->get().getVertex() );
  }
  
  // return the number of states
  return std::prev(std::prev(m.end())->second.end())->first + 1;
}

void Scheduler::dumpScheduledGraph( graphType& g, graphType::ScheduleID s )
{
  partitionMap_t m;
  buildPartTimeMap( m,g,s );
  std::cout << std::endl << "List of graph nodes for " << g.nameScheduleID(s) << " schedule:" << std::endl;
  std::cout << "[partition][time] C_code" << std::endl;
  for( auto i = m.begin(); i != m.end(); i++ )
  {
    for( auto j = i->second.begin(); j != i->second.end(); j++ )
    {
      for( auto k = j->second.begin(); k != j->second.end(); k++ )
      {
        graphType::vertex_t* p = *k;
        std::cout << "[" << i->first << "][" << j->first << "] " << p->getNode().get().C_format() << std::endl;
      }
    }
  }
  std::cout << std::endl;
}
