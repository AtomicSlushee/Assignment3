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

  std::string C_format();

  friend std::ostream& operator<<( std::ostream& out, ForLoop& a );

private:
  int mWidth;
  Statements* mInitial;
  Statements* mCondition;
  Statements* mUpdate;
  Statements* mBody;
};

#endif//__for_h__
