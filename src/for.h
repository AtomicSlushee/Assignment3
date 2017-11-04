#ifndef __for_h__
#define __for_h__

#include "assignment.h"
#include "condition.h"
#include <string>

class Statements;

class ForLoop
{
public:
  // constructor
  ForLoop( Statements* initial, Statements* condition, Statements* update, Statements* body )
  : mInitial(initial)
  , mCondition(condition)
  , mUpdate(update)
  , mBody(body)
  {
    // determine operator width
    mWidth = 0; //TODO
  }

  // get the initial expression
  Statements& getInitial(){return *mInitial;}

  // get the condition argument
  Statements& getCondition(){return *mCondition;}

  // get the update expression
  Statements& getUpdate(){return *mUpdate;}

  // get the body statements reference
  Statements& getBody(){return *mBody;}

  // get the component width
  int getWidth()
  {
    return mWidth;
  }

  std::string C_format()
  {
    std::string out;
    out = "for-loop is todo"; //TODO
    return out;
  }

  // stream override to output the statement in Verilog
  friend std::ostream& operator<<( std::ostream& out, ForLoop& a )
  {
    out << "FOR-LOOP IS TODO"; //TODO
    /*DEBUG*/out << " // " << a.C_format();
    return out;
  }

private:
  int mWidth;
  Statements* mInitial;
  Statements* mCondition;
  Statements* mUpdate;
  Statements* mBody;
};

#endif//__for_h__
