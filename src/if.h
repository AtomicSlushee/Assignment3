#ifndef __if_h__
#define __if_h__

#include "variable.h"
#include <string>

class Statements;

class IfStatement
{
public:
  // constructor
  IfStatement( Variable& condition, Statements* ifTrue, Statements* ifFalse )
  : mCondition( condition )
  , mIfTrue(ifTrue)
  , mIfFalse(ifFalse)
  {
    // determine operator width
    mWidth = condition.width();
  }

  // get the condition argument
  Variable& getCondition()
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
    out = "if ( " + mCondition.name() + " )";
    return out;
  }

  // stream override to output the statement in Verilog
  friend std::ostream& operator<<( std::ostream& out, IfStatement& a )
  {
    out << "IF STATEMENT IS TODO"; //TODO
    /*DEBUG*/out << " // " << a.C_format();
    return out;
  }

private:
  int mWidth;
  Variable& mCondition;
  Statements* mIfTrue;
  Statements* mIfFalse;
};

#endif//__if_h__
