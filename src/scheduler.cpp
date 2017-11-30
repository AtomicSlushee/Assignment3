#include "scheduler.h"
#include "graphType.hpp"
#include "operator.h"
#include "vertex.hpp"

// dElete me
#include <iostream>
#include "statement.h"
#include <vector>

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
  FDS(g, latencyConstraint);
  
  //dumpScheduledGraph( g, graphType::ASAP);
  //dumpScheduledGraph( g, graphType::ALAP);
  dumpScheduledGraph(g, graphType::FDS);
  
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

void Scheduler::FDS(graphType& g, int latencyConstraint)
{
  // Minimize resources under a latency constraint
  
  // make sure that nothing is scheduled for FDS
  // this helper is awesome! a schedule time for each algorithm? brilliant.
  
  int timewidth = 0;
  int timestep; // for iterating through times
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    v->get().helper.schedTime[graphType::FDS] = NOT_SCHEDULED;
  }
  ALAP(g, latencyConstraint);
  ASAP(g);
  for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
  {
    // first, compute time frames for every node.
    // todo: really? every node every time? meh guess so.
    // Fortunately, this is really easy now.
    // The time frame is just the earliest a node can be scheduled (ASAP)
    // followed by the latest (ALAP)
    
    
    // The width of the timeframe for every node is the ALAP - ASAP + 1
    int leftEdge = v->get().helper.schedTime[graphType::ASAP];
    int rightEdge = v->get().helper.schedTime[graphType::ALAP];
    v->get().timeFrame[0] = leftEdge;
    v->get().timeFrame[1] = rightEdge;
  }
  // keep cycling until all nodes scheduled
  bool done = false;
  while(!done)
  {
    // repeat until all ops are scheduled
    done = true; // if we get through the for loop without scheduling, we're done
    
    
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
    {
      // first, compute time frames for every node.
      // todo: really? every node every time? meh guess so.
      // Fortunately, this is really easy now.
      // The time frame is just the earliest a node can be scheduled (ASAP)
      // followed by the latest (ALAP)
      
      
      // The width of the timeframe for every node is the ALAP - ASAP + 1
      int leftEdge = v->get().helper.schedTime[graphType::ASAP];
      int rightEdge = v->get().helper.schedTime[graphType::ALAP];
      
      timewidth = rightEdge - leftEdge + 1;
      
      std::cout << "Node " << v->get().getNodeNumber() << " leftEdge " << leftEdge << " rightEdge " << rightEdge << " Timewidth " << timewidth << std::endl;
      
      // compute the operations and type probabilities
      // Operational Probability is easy. just 1/timewidth for times in the range. 0 else.
      
      for (timestep = 1; timestep <= latencyConstraint; timestep++)
      {
        if (timestep >= leftEdge && timestep <= rightEdge)
        {
          float prob = 1.0/timewidth;
          std::cout << "op prob " << prob <<  " recorded in cycle " << timestep << std::endl;
          v->get().opProb.push_back(prob);
        }
        else
        {
          v->get().opProb.push_back(0.0);
        }
        v->get().selfForce.push_back(0.0); // need a list that is the same size and starts with all 0.0s
        
      }
      std::cout << "Operational Probabilities [cycle][probability]" << std::endl;
      
      int time = 1;
      for (auto t = v->get().opProb.begin(); t != v->get().opProb.end(); t++)
      {
        std::cout << "[" << time++ << "][" << *t << "]" << std::endl;
      }
    }
    // now compute the type distribution
    // Type Distribution is the sum of probabilities of the operations implemented by a specific
    // resource at any time step of interest
    
    // the later it gets, the quicker and dirtier this solution is
    std::vector<float> ad;
    for ( timestep = 0 ; timestep < latencyConstraint; timestep++)
    {
      ad.push_back(0.0);
      
    }
    float md = 0.0;
    float dd = 0.0;
    float ld = 0.0;
    // TODO: make the obvious insertions. better yet, find a better way
    
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
    {
      
      
      // for each node sum the probabilities that the resource is being used in that time step
      for (int timestep = 0; timestep < latencyConstraint; timestep++)
      {
        /*
         ADDER_SUB,
         MULTIPLIER,
         LOGICAL,
         DIV_MOD
         */
        if (v->get().getNode().get().getResource() == Statement::ADDER_SUB)
        {
          std::vector<float>::iterator it = v->get().opProb.begin();
          std::advance(it,timestep);
          ad[timestep] += *it;
        }
        
      }
    }
    std::cout << " ADD Probabilities for each timestep" << std::endl;
    std::cout << " [cycle][Probability of use]" << std::endl;
    for ( timestep = 1; timestep <= latencyConstraint; timestep++)
    {
      std::cout << " [" << timestep << "]" << "[" << ad[timestep -1] << "]" << std::endl;
    }
    
    // Compute self forces, pred/successor forces, and total forces
    
    // for each node, compute the self force
    for( auto v = g.getGraph().begin(); v != g.getGraph().end(); v++)
    {
      // from the look of it, compute a self force for each time step

        float selfForce = 0.0;
        // damn, have to do it this way again? if only I had started earlier I
        // could think of a better way to do this.
        if (v->get().getNode().get().getResource() == Statement::ADDER_SUB)
        {
          selfForce = 0.0;
          for (int j = 1; j <= latencyConstraint; j++)
          {
            float temp = 0.0;
            for (int k = 1; k <= latencyConstraint; k++)
            {
              float opProbabilty = 0.0;
              float typeDist = 0.0;
              bool isUnity = (j==k);
              opProbabilty =v->get().opProb[j];
              typeDist = ad[j-1];// todo shouldn't need to do j-1 for resource stuff
              
              temp = opProbabilty*(isUnity - typeDist);
              //temp = v->get().opProb[j]*((j==k) - ad[j - 1]);
              selfForce += temp;
            }
            // todo better name than j
            
            std::cout << "Self force for node " << v->get().getNodeNumber() << " at timestep " << j << " is " << temp << std::endl;
            //std::cout << "opProb[" <<j<<"] = " <<v->get().opProb[j] << std::endl;
            //std::cout << ad[j - 1];
            
          }

          
          std::cout << "Total Self force is " << selfForce<<std::endl;
        v->get().selfForce.push_back(selfForce);
      }
      
    }

  }
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
