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
    int group;
    Helper():color(BLACK),dist(0.0),weight{1.0,0.0},group(0){}
  };

public:
  typedef Vertex<Statement,Variable,Helper> vertex_t;
  typedef vertex_t::vertices_t vertices_t;

public:
  graphType(){}
  ~graphType(){}
  graphType(vertices_t& copyMe)
  {
    graph = copyMe;
  }
  graphType(graphType& copyMe)
  {
    graph = copyMe.getGraph();
  }

  // add a vertex in our graph for every assignment found
  int createVertices(Statements& stmt)
  {
    int node=1;
    int group=1;
    createVertices(stmt,node,group,nullptr);
    return ++group;
  }

  // this is the recursive element that covers all statements, if branches, etc.
  void createVertices(Statements& stmt, int& node, int& group, Statement* sx)
  {
    // loop through all the given statements
    for( Statements::iterator i = stmt.begin(); i != stmt.end(); i++)
    {
      // if this statement is an assignment
      if( i->isAssignment() )
      {
        Assignment& a = i->assignment();              // grab the assignment
        vertex_t& v = *(new vertex_t(*i,node++));     // create a new vertex for it
        v.helper.weight[LATENCY] = a.getLatency();    // set the weight based on latency
        v.helper.group = group;                       // give it the current group number
        graph.push_back(v);                           // add vertex to graph
        v.addOutput(a.getResult());                   // add assignment output to vertex output list
        for( int n = 0; n < a.getNumArgs(); n++ )     // add assignment inputs to vertex input list
        {
          v.addInput(a.getInput(n));
        }
        if( nullptr != sx)
        {
          // if we have been called from within an if-statement, we need to add
          // the condition variable to this vertex's input list
          if( sx->isIfStatement() )
          {
            Variable& condition = sx->if_statement().getCondition();
            v.addInput(condition);
          }
        }
      }
      // if this statement is an if-statement
      else if( i->isIfStatement() )
      {
        IfStatement& fi = i->if_statement();          // grab the if-statement
        vertex_t& v = *(new vertex_t(*i,node++));     // create a new vertex for it
                                                      // TODO: weight of conditional
        v.helper.group = ++group;                     // give it the next group number (by itself)
        graph.push_back(v);                           // add vertex to graph
        Variable& condition = fi.getCondition();      // add the condition variable...
        v.addOutput(condition);                       // ...as an output
        v.addInput(condition);                        // ...and an input
        // now, create vertices for the true branch in the next group
        createVertices(fi.getIfTrue(), node, ++group, i->getStatement());
        // and, create vertices for the false branch in the next group
        createVertices(fi.getIfFalse(), node, ++group, i->getStatement());
        // if we have another statement, it will be in the next group
        if( std::next(i) != stmt.end())
          ++group;
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
    // loop through all the vertices of the graph
    for( auto v1 = graph.begin(); v1 != graph.end(); v1++ )
    {
      // compare against every other vertex in the graph
      for( auto v2 = std::next(v1); v2 != graph.end(); v2++ )
      {
        // grab this vertex's output list
        vertex_t::iolist_t& outs = v1->get().getOutputs();
        // check all v1 vertex outputs (usually only one) to the v2 inputs
        for( auto o = outs.begin(); o != outs.end() ; o++)
        {
          auto x = o->get();
          // if v2 output is in v1 input list
          if( v2->get().findInput( o->get() ))
          {
            // add a link: v1 to v2
            v1->get().addLinkTo(*v2);
            // add a link v2 from v1
            v2->get().addLinkFrom(*v1);
          }
        }
      }
    }
  }

  // create a weighted graph
  int createWeightedGraph(Statements& stmt)
  {
    // first, create the vertices
    int groups = createVertices(stmt);
    // next, create the edges
    createEdges();

    return groups;
  }

  // topological sort visit activity (recursive)
  void tsVisit(vertices_t& l, vertex_t& v)
  {
    v.helper.color = GRAY;
    vertices_t& nlist = v.getLinks();
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

  // create a topologically sorted list per the algorithm
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

  // compute the longest path through the given list of vertices
  // using the given weight ID (either unity or latency for now)
  double longestPath(vertices_t& l, WeightID w)
  {
    // input is assumed to be topologically sorted
    for( auto u = l.begin(); u != l.end(); u++)
    {
      u->get().helper.dist = 0.0;
    }
    for( auto u = l.begin(); u != l.end(); u++)
    {
      vertices_t& nlist = u->get().getLinks();
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

  // wipe out the graph to start over
  void graphClear()
  {
    graph.clear();
  }

  vertices_t& getGraph()
  {
    return graph;
  }

private:
  vertices_t graph;
};

#endif /* graphType_hpp */
