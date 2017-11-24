#ifndef vertex_hpp
#define vertex_hpp

#include <functional>

template< class NODETYPE, class IOTYPE, class HELPER >
class Vertex
{
public:
  typedef std::reference_wrapper<Vertex<NODETYPE,IOTYPE,HELPER>> vertref_t;
  typedef std::reference_wrapper<NODETYPE> noderef_t;
  typedef std::list<vertref_t> nodelist_t;
  typedef std::reference_wrapper<IOTYPE> ioref_t;
  typedef std::list<ioref_t> iolist_t;

  HELPER helper;

  Vertex( NODETYPE& r, int n ) : helper(), rNode( r ), nodeNum( n ), inList(), outList(), nodeList(){}
  ~Vertex()
  {
  }

  void addInput(IOTYPE& i){inList.push_back(i);}
  void addOutput(IOTYPE& o){outList.push_back(o);}
  iolist_t& getOutputs(){return outList;}
  bool findInput(IOTYPE& in)
  {
    typename iolist_t::iterator i;
    for( i = inList.begin(); i != inList.end(); i++)
      if( i->get() == in)
        return true;
    return false;
  }
  void addLink(vertref_t v){nodeList.push_back(v);}
  nodelist_t& getLinks(){return nodeList;}
  int getNodeNumber(){return nodeNum;}
  NODETYPE& getNode(){return rNode;}

private:
  noderef_t rNode;
  int nodeNum;
  iolist_t inList;
  iolist_t outList;
  nodelist_t nodeList;

};

#endif//vertex_hpp
