#ifndef graphType_hpp
#define graphType_hpp

#include "statement.h"
#include "vertex.hpp"
#include "condition.h"
#include <new>
#include <functional>

class graphType
{
public:
  enum HelperID {BLACK,GRAY,WHITE};
  enum WeightID {UNITY=0,LATENCY,SCHEDULING,numWeights};
  enum ScheduleID {ASAP=0,ALAP,FDS,numSchedules};
private:
  static const int UnrollCount = 4;
  struct Helper
  {
    HelperID color;
    double dist;
    double weight[numWeights];
    int partition;
    int schedTime[numSchedules];
    int nextPart; // used only by if-statements
    Helper():color(BLACK),dist(0.0),weight{1.0,0.0},partition(0),schedTime(),nextPart(0){}
  };

public:
  typedef Vertex<Statement,Variable,Helper> vertex_t;
  typedef std::reference_wrapper< vertex_t > vertref_t;
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
    int partition=1;
    // create a top and bottom NOP to hook everything up in between
    vertex_t* topNOP = new vertex_t(*(new Statement()), 0);
    vertex_t* endNOP = new vertex_t(*(new Statement()) ,0);
    graph.push_back( *topNOP );
    createVertices(stmt,node,partition,topNOP,endNOP);
    endNOP->helper.partition = partition+1;
    graph.push_back(*endNOP);
    return partition;
  }

  // this is the recursive element that covers all statements, if branches, etc.
  void createVertices(Statements& stmt, int& node, int& partition, vertex_t* from, vertex_t*& to)
  {
    bool firstStatement = true;

    // loop through all the given statements
    for( Statements::iterator i = stmt.begin(); i != stmt.end(); i++)
    {
      // if this statement is an assignment
      if( i->isAssignment() )
      {
        doAssignment( i, node, partition, from, to );
      }
      // if this statement is an if-statement
      else if( i->isIfStatement() )
      {
        // This part gets entertaining... need to substitute the if-statement for the NOP statement
        // inside the 'to' value, such that everything preceding remains linked, only now it will be
        // to the if-statement. Then, we modify 'to' so that it points to a new pseudo-endif
        // vertex to be used as the next end-of-links, which gets passed back to the caller.

        IfStatement& fi = i->if_statement();          // grab the if-statement
        new (to->getNode().get().getStatement())Statement(fi); // 'to' vertex now has the if-statement
        vertex_t* v = to;                             // let's move that to 'v', the new local 'from'
        if( fi.getIfFalse().empty() )
          to = new vertex_t(*(new Statement()), 0);   // create the new pseudo-endif to get back
        else
          to = new vertex_t(*(new Statement(true)), 0);//create the new pseudo-else to get back

        v->addLinkTo(*to);                            // establish dependency links
        to->addLinkFrom(*v);
                                                      // TODO: weight of conditional, if any
        if( firstStatement)
          v->helper.partition = partition;            // partition number previously incremented to get here
        else
          v->helper.partition = ++partition;          // give it the next partition number (by itself)
        graph.push_back(*v);                          // add vertex to graph
        Condition& condition = fi.getCondition();     // add the condition variable...
        v->addOutput(condition.getLeft());            // ...as an output
        v->addInput(condition.getLeft());             // ...and an input
        if( condition.getLogic().id() != Operator::NOP )
        { // if condition with more than one variable (from for-loop)
          v->addOutput(condition.getRight());         // ...as an output
          v->addInput(condition.getRight());          // ...and an input
        }

        // to keep things chained properly
        from = v;

        // now, create vertices for the true branch in the next partition
        if( !fi.getIfTrue().empty() )
        {
          createVertices(fi.getIfTrue(), node, ++partition, from, to );
          // if there is also a false branch
          if( !fi.getIfFalse().empty() )
          {
            to->helper.partition = partition;
            graph.push_back(*to); // save the 'to' node in the graph
            from = to; // need to move 'to' to 'from' and create a new 'to' the keep the chain going
            to = new vertex_t(*(new Statement()), 0);     // create the new end target for either the next partition or caller
          }
          // if not a false, are there more statements to come?
          else if( std::next(i) != stmt.end() )
          {
            to->helper.partition = partition;
            graph.push_back(*to); // save the 'to' node in the graph
            from = to; // 'to' needs to become the new 'from', and a new 'to' created
            to = new vertex_t(*(new Statement()), 0);     // create the new end target for either the next partition or caller
            ++partition;
            // let the if-statment know where to go after the true branch
            v->helper.nextPart = partition;
          }
          else
          { // special case: no else, and nothing more to come
            v->helper.nextPart = partition + 1;
          }
        }
        else
        {
          // oddball case, won't happen
        }

        // and, create vertices for the false branch in the next partition
        if( !fi.getIfFalse().empty() )
        {
          // let the if-statment know where to go after the true branch
          v->helper.nextPart = ++partition;
          createVertices(fi.getIfFalse(), node, partition, from, to );
          // remember how to get here from the end of the true branch
          from->helper.nextPart = partition+1;

          // are there more statements to come?
          if( std::next(i) != stmt.end() )
          {
            to->helper.partition = partition;
            graph.push_back(*to); // save the 'to' node in the graph
            from = to; // 'to' needs to become the new 'from', and a new 'to' created
            to = new vertex_t(*(new Statement()), 0);     // create the new end target for either the next partition or caller
            ++partition;
          }
        }
      }
      else if( i->isForLoop() )
      {
        // grab the initialize statement from the for loop...
        Statements::iterator ii = i->for_loop().getInitial().begin();
        // and add it to the graph as an assignment
        doAssignment( ii, node, partition, from, to );
        // now create a new Statements type, which will hold a single if-statement
        Statements& u = *new Statements();
        // now recurse to create the unrolled for-loop
        recurseForLoop( i, u, UnrollCount );
        // finally we need to call ourselves with the new if-statement
        partition++;
        createVertices( u, node, partition, from, to );
      }
      else
      {
        throw; // shouldn't get here
      }
      firstStatement = false;
    }
  }

  void doAssignment( Statements::iterator& i, int& node, int partition, vertex_t* from, vertex_t* to )
  {
    Assignment& a = i->assignment();              // grab the assignment
    vertex_t& v = *(new vertex_t(*i,node++));     // create a new vertex for it
    v.helper.weight[LATENCY] = a.getLatency();    // set the weight based on latency
    v.helper.weight[SCHEDULING] = i->scheduleLatency();
    v.helper.partition = partition;               // give it the current partition number
    graph.push_back(v);                           // add vertex to graph
    v.addOutput(a.getResult());                   // add assignment output to vertex output list
    for( int n = 0; n < a.getNumArgs(); n++ )     // add assignment inputs to vertex input list
    {
      v.addInput(a.getInput(n));
    }
    // MUX2x1 is a special case: we must treat the "x : y" as outputs, too
    if( a.getOperator().id() == Operator::MUX2x1 )
    {
      v.addOutput( a.getInput2() );
      v.addOutput( a.getInput3() );
    }
    v.addLinkFrom(*from);                         // top level linkage for scheduling
    from->addLinkTo(v);
    v.addLinkTo(*to);
    to->addLinkFrom(v);
  }

  void recurseForLoop( Statements::iterator& i, Statements& u, int level)
  {
    if( level-- )
    {
      // grab the conditional from the original for-loop
      Condition& c = i->for_loop().getCondition().begin()->condition();
      // add it to the given body as an if-statement
      IfStatement& fi = u.addIfStatement( c );
      // grab a reference to the true-branch of our if-statement
      Statements& ii = fi.getIfTrue();
      // grab the body from the original for-loop
      Statements& b = i->for_loop().getBody();
      // add the body statements to the true-branch of the if-statement
      for( auto x = b.begin(); x != b.end(); x++)
      {
        ii.addStatement(*x->getStatement());
      }
      // add the increment from the original for-loop to the true-branch
      Statement& inc = *(i->for_loop().getUpdate().begin()->getStatement());
      ii.addStatement( inc );
      // now recurse until done
      recurseForLoop( i, ii, level );
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
    vertices_t& nlist = v.getLinksTo();
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
      vertices_t& nlist = u->get().getLinksTo();
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

  vertices_t::iterator findStatement(Statement& stmt)
  {
    for( auto v = graph.begin(); v != graph.end(); v++)
    {
      if( *(v->get().getNode().get().getStatement()) == stmt)
        return v;
    }
    return graph.end();
  }

  std::string nameScheduleID( ScheduleID id )
  {
    static std::string idName[numSchedules] = { "ASAP", "ALAP", "FDS" };
    return idName[id];
  }

private:
  vertices_t graph;
};

#endif /* graphType_hpp */
