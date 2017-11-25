#ifndef graphType_hpp
#define graphType_hpp

#include "statement.h"
#include "vertex.hpp"

class graphType
{
public:
  enum HelperID {BLACK,GRAY,WHITE};
  enum WeightID {UNITY=0,LATENCY,numWeights};
private:
  struct Helper
  {
    HelperID color;
    double dist;
    double weight[numWeights];
    Helper():color(BLACK),dist(0.0),weight{1.0,0.0}{}
  };

public:
  typedef Vertex<Statement,Variable,Helper> vertex_t;
  typedef vertex_t::vertref_t vertref_t;
  typedef std::list<vertref_t> vertices_t;

public:
  graphType(){}
  ~graphType(){}

  // add a vertex in our graph for every assignment found
  void createVertices(Statements& stmt)
  {
    int node=1;
    createVertices(stmt,node,nullptr);
  }
  void createVertices(Statements& stmt, int& node, Statement* sx)
  {
    for( Statements::iterator i = stmt.begin(); i != stmt.end(); i++)
    {
      if( i->isAssignment() )
      {
        Assignment& a = i->assignment();
        vertex_t& v = *(new vertex_t(*i,node++));
        v.helper.weight[LATENCY] = a.getLatency();
        graph.push_back(v);
        v.addOutput(a.getResult());
        for( int n = 0; n < a.getNumArgs(); n++ )
        {
          v.addInput(a.getInput(n));
        }
        if( nullptr != sx)
        {
          if( sx->isIfStatement() )
          {
            Variable& condition = sx->if_statement().getCondition();
            v.addInput(condition);
          }
        }
      }
      else if( i->isIfStatement() )
      {
        //TODO: new to handle if-statements
        IfStatement& fi = i->if_statement();
        vertex_t& v = *(new vertex_t(*i,node++));
        // TODO: weight of conditional
        graph.push_back(v);
        Variable& condition = fi.getCondition();
        v.addOutput(condition);
        v.addInput(condition);
        createVertices(fi.getIfTrue(), node, i->getStatement());
        createVertices(fi.getIfFalse(), node, i->getStatement());
      }
      else if( i->isForLoop() )
      {
        //TODO: need to handle for-loops
        //TODO: not yet sure about when to unroll for-loop four times for FDS
      }
      else
      {
        throw; // shouldn't get here
      }
    }
  }

  // create the edges in the graph, which are directional dependencies: output->input
  void createEdges()
  {
    for( auto v1 = graph.begin(); v1 != graph.end(); v1++ )
    {
      for( auto v2 = std::next(v1); v2 != graph.end(); v2++ )
      {
        vertex_t::iolist_t& outs = v1->get().getOutputs();
        for( auto o = outs.begin(); o != outs.end() ; o++)
        {
          auto x = o->get();
          if( v2->get().findInput( o->get() ))
            v1->get().addLink(*v2);
        }
      }
    }
  }

  void createWeightedGraph(Statements& stmt)
  {
    // first, create the vertices
    createVertices(stmt);
    // next, create the edges
    createEdges();
  }

  void tsVisit(vertices_t& l, vertex_t& v)
  {
    v.helper.color = GRAY;
    vertex_t::nodelist_t& nlist = v.getLinks();
    for( auto n = nlist.begin(); n != nlist.end(); n++)
    {
      if( n->get().helper.color == WHITE )
      {
        tsVisit( l, n->get() );
      }
    }
    v.helper.color = BLACK;
    l.push_front( v );
  }

  void topologicalSort(vertices_t& l)
  {
    for( auto v = graph.begin(); v != graph.end(); v++ )
    {
      v->get().helper.color = WHITE;
    }
    for( auto v = graph.begin(); v != graph.end(); v++ )
    {
      if( v->get().helper.color == WHITE )
      {
        tsVisit( l, *v );
      }
    }
  }

  double longestPath(vertices_t& l, WeightID w)
  {
    // input is topological sort
    for( auto u = l.begin(); u != l.end(); u++)
    {
      u->get().helper.dist = 0.0;
    }
    for( auto u = l.begin(); u != l.end(); u++)
    {
      vertex_t::nodelist_t& nlist = u->get().getLinks();
      for( auto v = nlist.begin(); v != nlist.end(); v++)
      {
        double weight = u->get().helper.dist + v->get().helper.weight[w];
        if( weight > v->get().helper.dist)
        {
          v->get().helper.dist = weight;
        }
      }
    }
    double maxW = 0.0;
    for( auto u = l.begin(); u != l.end(); u++)
    {
      if( u->get().helper.dist > maxW)
      {
        maxW = u->get().helper.dist;
      }
    }
    return maxW;
  }

private:
  vertices_t graph;
};

#endif /* graphType_hpp */
