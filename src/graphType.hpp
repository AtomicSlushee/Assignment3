#ifndef graphType_hpp
#define graphType_hpp

#include "statement.h"
#include "vertex.hpp"

class graphType
{
private:
  enum HelperID {BLACK,GRAY,WHITE};
  struct Helper
  {
    HelperID color;
    Helper():color(BLACK){}
  };

public:
  typedef Vertex<Assignment,Variable,Helper> vertex_t;
  typedef std::list<vertex_t> vertices_t;

public:
  graphType(){}
  ~graphType(){}

  // add a vertex in our graph for every assignment found
  void createVertices(Statements& stmt)
  {
    int node = 1;
    for( Statements::iterator i = stmt.begin(); i != stmt.end(); i++)
    {
      if( i->isAssignment() )
      {
        Assignment& a = i->assignment();
        vertex_t& v = *(new vertex_t(a,node++));
        graph.push_back(v);
        v.addOutput(a.getResult());
        for( int n = 0; n < a.getNumArgs(); n++ )
          v.addInput(a.getInput(n));
      }
      else
      {
        //TODO: need to handle for-loops and if-statements
        //TODO: not yet sure about when to unroll for-loop four times for FDS
      }
    }
  }

  // create the edges in the graph, which are directional dependencies: output->input
  void createEdges()
  {
    for( vertices_t::iterator v1 = graph.begin(); v1 != graph.end(); v1++ )
    {
      for( vertices_t::iterator v2 = std::next(v1); v2 != graph.end(); v2++ )
      {
        vertex_t::iolist_t& outs = v1->getOutputs();
        for( vertex_t::iolist_t::iterator o = outs.begin(); o != outs.end() ; o++)
        {
          if( v2->findInput( *o ))
            v1->addLink(*v2);
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
    l.push_back( v );
  }

  void topologicalSort(vertices_t& l)
  {
    for( auto v = graph.begin(); v != graph.end(); v++ )
    {
      v->helper.color = WHITE;
    }
    for( auto v = graph.begin(); v != graph.end(); v++ )
    {
      if( v->helper.color == WHITE )
      {
        tsVisit( l, *v );
      }
    }
  }

private:
  vertices_t graph;
};

#endif /* graphType_hpp */
