#ifndef vertex_hpp
#define vertex_hpp

#include <functional>
#include <vector>
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
  std::vector<float> TotalForce;
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
  float ComputeSelfForceForTimeSlot(int timeslot, float typeDist, bool occursThisSlot)
  {
    float selfForce = 0.0;
    float opProbabilty = 0.0;
    opProbabilty = opProb[timeslot-1];
    selfForce = typeDist*(occursThisSlot - opProbabilty);
    std::cout << "\t\tCycle " << timeslot << " : " <<  typeDist << "*("<<occursThisSlot<<" - " << opProbabilty << ")" << std::endl;
    return selfForce;
    
  }
  float ComputeSuccessorForceForTimeSlot(int timestep)
  {
    float SuccessorForce = 0.0;
    for( auto p = this->getLinksTo().begin(); p != this->getLinksTo().end(); p++)
    {
      
      // this scheduled timeslot interfere's with a successor's timeslot
      // so calculate the successor force
      if (timestep >= p->get().leftEdge && timestep <= p->get().rightEdge)
      {
        
        for (int k = p->get().leftEdge; k <= p->get().rightEdge; k++)
        { // this is for each of the time frames which is what we want.
          
          std::cout << "  Successor Force at time " << k << " from node " << p->get().getNodeNumber() << " is \n\r" << p->get().ComputeSelfForceForTimeSlot(timestep, p->get().ResourceTypeDistribution[k-1], timestep!=k) << std::endl;
          // todo this will probably not work if the timewidths are greater than 1 for the suc nodes
          SuccessorForce = p->get().ComputeSelfForceForTimeSlot(timestep, p->get().ResourceTypeDistribution[k-1], timestep!=k);
        }
        // Now we've done it for the first layer, we must go deeper
        if (!p->get().getLinksTo().empty())
        {
          SuccessorForce += p->get().ComputeSuccessorForceForTimeSlot(timestep);
        }
        
      }
      else
      {
        // std::cout << "  Successor Force at time " << timestep << " from node " << p->get().getNodeNumber() << " is " << 0 << std::endl;
      }
      
    }
    std::cout << "Successor Force from node " << getNodeNumber() << " is " << SuccessorForce <<std::endl;
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
