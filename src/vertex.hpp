#ifndef vertex_hpp
#define vertex_hpp

#include <functional>
#include <vector>
#include <map>
template< class NODETYPE, class IOTYPE, class HELPER >
class Vertex
{
public:
  typedef std::reference_wrapper< Vertex< NODETYPE, IOTYPE, HELPER > > vertref_t;
  typedef std::reference_wrapper< NODETYPE > noderef_t;
  typedef std::reference_wrapper< IOTYPE > ioref_t;
  typedef std::list< vertref_t > vertices_t;
  typedef std::list< ioref_t > iolist_t;
  
  // just something quick here:
  // need a list of probabilities
  
  std::vector<float> opProb;
  std::map<float,int> TotalForce;
  std::vector<float> selfForce;
  
  float leftEdge  = 0.0;
  float rightEdge = 0.0;
  std::vector<float> ResourceTypeDistribution;
  
  HELPER helper;
  
  Vertex( NODETYPE& r, int n )
  : helper(), rNode( r ), nodeNum( n ), inList(), outList(), linkTo()
  {
  }
  
  ~Vertex()
  {
  }
  
  int TimeWithMinimumForce()
  {
    float Current_Minimum_Cycle = 10000000;
    float Current_Minimum       = 10000000;
    float Current_Cycle         = 10000000;
    float Current_Force         = 10000000;
    std::cout << "Searching for mimimum force" << std::endl;
    std::cout << "[Cycle][Force]" << std::endl;
    for(auto it = TotalForce.cbegin(); it != TotalForce.cend(); ++it )
    {
      Current_Cycle = it->second;
      Current_Force = it->first;
      std::cout << "[" << Current_Cycle << "]" << "[" << Current_Force<< "]" << std::endl;
      if (Current_Force < Current_Minimum) {
        Current_Minimum       = Current_Force;
        Current_Minimum_Cycle = Current_Cycle;
      }
    }
    std::cout << Current_Minimum << " is the minimum force at cycle " << Current_Minimum_Cycle << std::endl;

#ifdef EHL
    for (auto op = opProb.begin(); op != opProb.end(); op++)
    {
      *op = 0.0;
    }
    opProb[(int)Current_Minimum_Cycle] = 1.0;
#endif

    return Current_Minimum_Cycle;
  }
  float ComputeSelfForceForTimeSlot(int timeslot, float typeDist, bool occursThisSlot)
  {
    float selfForce = 0.0;
    float opProbabilty = 0.0;
    opProbabilty = opProb[timeslot];
    selfForce = typeDist*(occursThisSlot - opProbabilty);
    std::cout << "\t\tCycle " << timeslot << " : " <<  typeDist << "*("<<occursThisSlot<<" - " << opProbabilty << ")   selfForce = " << selfForce << std::endl;
    return selfForce;
    
  }
  float ComputeSelfForce(int timestep)
  {
    float sf = 0.0;
    for (int k = leftEdge; k <= rightEdge; k++)
    {
      sf +=  ComputeSelfForceForTimeSlot(timestep, ResourceTypeDistribution[k], timestep==k);
    }
    return sf;
  }
  
  float ComputePredecessorForceForTimeSlot(int timestep)
  {
    float PredecessorForce = 0.0;
    float dbTemp = 0.0;
    for( auto p = this->getLinksFrom().begin(); p != this->getLinksFrom().end(); p++)
    {
      if (p->get().getNodeNumber() == 0)
      {
        continue;
      }
      // this scheduled timeslot interfere's with a predecessor's timeslot
      // so calculate the successor force
      if (timestep >= p->get().leftEdge && timestep <= p->get().rightEdge)
      {
        
        for (int k = p->get().leftEdge; k <= p->get().rightEdge; k++)
        { // this is for each of the time frames which is what we want.
          dbTemp = p->get().ComputeSelfForceForTimeSlot(timestep, p->get().ResourceTypeDistribution[k], timestep!=k);
          PredecessorForce += dbTemp;
          //std::cout << "  Successor Force at time " << k << " from node " << p->get().getNodeNumber() << " is " << dbTemp << std::endl;
          // Now we've done it for the first layer, we must go deeper
          if (!p->get().getLinksFrom().empty())
          {
            PredecessorForce += p->get().ComputePredecessorForceForTimeSlot(k);
          }
        }
        
        
      }

      
    }
    return PredecessorForce;
  }
  float ComputeSuccessorForceForTimeSlot(int timestep)
  {
   
    float SuccessorForce = 0.0;
    float dbTemp = 0.0;
    for( auto p = this->getLinksTo().begin(); p != this->getLinksTo().end(); p++)
    {
      

      // this scheduled timeslot interfere's with a successor's timeslot
      // so calculate the successor force
      if (timestep >= p->get().leftEdge && timestep <= p->get().rightEdge)
      {
        
        for (int k = p->get().leftEdge; k <= p->get().rightEdge; k++)
        {
          // this is for each of the time frames which is what we want.
          dbTemp = p->get().ComputeSelfForceForTimeSlot(timestep, p->get().ResourceTypeDistribution[k], timestep!=k);
          SuccessorForce += dbTemp;
          //std::cout << "  Successor Force at cycle " << k << " from node " << p->get().getNodeNumber() << " is " << dbTemp << std::endl;
          // Now we've done it for the first layer, we must go deeper
          if (!p->get().getLinksTo().empty())
          {
            SuccessorForce += p->get().ComputeSuccessorForceForTimeSlot(k);
          }
        }
        
        
      }

      
    }
    return SuccessorForce;
  }
  
  void addInput( IOTYPE& i )
  {
    inList.push_back( i );
  }
  
  void addOutput( IOTYPE& o )
  {
    outList.push_back( o );
  }
  
  iolist_t& getOutputs()
  {
    return outList;
  }
  
  bool findInput( IOTYPE& in )
  {
    typename iolist_t::iterator i;
    for( i = inList.begin(); i != inList.end(); i++ )
      if( i->get() == in )
        return true;
    return false;
  }
  
  void addLinkTo( vertref_t v )
  {
    linkTo.push_back( v );
  }
  
  void addLinkFrom( vertref_t v )
  {
    linkFrom.push_back( v );
  }
  
  vertices_t& getLinksTo()
  {
    return linkTo;
  }
  
  vertices_t& getLinksFrom()
  {
    return linkFrom;
  }
  
  int getNodeNumber()
  {
    return nodeNum;
  }
  
  noderef_t& getNode()
  {
    return rNode;
  }
  
  Vertex* getVertex()
  {
    return this;
  }
private:
  noderef_t rNode;
  int nodeNum;
  iolist_t inList;
  iolist_t outList;
  vertices_t linkTo;
  vertices_t linkFrom;
  
};

#endif//vertex_hpp
