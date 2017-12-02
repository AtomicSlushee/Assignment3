#ifndef __if_h__
#define __if_h__

#include "condition.h"
#include <string>

class Statements;

class IfStatement
{
public:
  // constructor
  IfStatement( Condition& condition, Statements* ifTrue, Statements* ifFalse )
  : mCondition( condition )
  , mIfTrue(ifTrue)
  , mIfFalse(ifFalse)
  {
    // determine operator width
    mWidth = 0; //TODO
  }

  // get the condition argument
  Condition& getCondition()
  {
    return mCondition;
  }

  // get the component width
  int getWidth()
  {
    return mWidth;
  }

  // get the IfTrue/IfFalse statements reference
  Statements& getIfTrue(){return *mIfTrue;}
  Statements& getIfFalse(){return *mIfFalse;}

  std::string C_format()
  {
    std::string out;
    out = "if ( " + mCondition.C_format() + " )";
    return out;
  }

  // stream override to output the statement in Verilog
  friend std::ostream& operator<<( std::ostream& out, IfStatement& a )
  {
    out << " // " << a.C_format();
    return out;
  }

private:
  int mWidth;
  Condition& mCondition;
  Statements* mIfTrue;
  Statements* mIfFalse;
};

#endif//__if_h__
