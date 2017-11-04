#ifndef __condition_h__
#define __condition_h__

#include "variable.h"
#include "operator.h"
#include <string>

class Condition
{
public:
  // constructor
  Condition( Variable& left, Operator& logic, Variable& right )
  : mLeft( left )
  , mLogic( logic )
  , mRight( right )
  {
    // determine operator width
    mWidth = 0; //TODO
  }

  // get the left condition argument
  Variable& getLeft()
  {
    return mLeft;
  }

  // get the right condition argument
  Variable& getRight()
  {
    return mRight;
  }

  // get the component width
  int getWidth()
  {
    return mWidth;
  }

  // get the logic operator
  Operator& getLogic()
  {
    return mLogic;
  }

  std::string C_format()
  {
    std::string out;
    out = "condition expression is todo"; //TODO
    return out;
  }

  // stream override to output the statement in Verilog
  friend std::ostream& operator<<( std::ostream& out, Condition& a )
  {
    out << "CONDITION EXPRESSION IS TODO"; //TODO
    /*DEBUG*/out << " // " << a.C_format();
    return out;
  }

private:
  int mWidth;
  Variable& mLeft;
  Operator& mLogic;
  Variable& mRight;
};

#endif//__condition_h__
