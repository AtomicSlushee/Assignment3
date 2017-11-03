#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include "assignment.h"
#include "singleton.h"

class Statement
{
public:
  Statement( Assignment& a )
      : stmt
        { .pAssignment = &a }, id( ASSIGNMENT )
  {
  }
  bool isAssignment()
  {
    return ASSIGNMENT == id;
  }
  Assignment& assignment()
  {
    if( ASSIGNMENT != id )
      throw;
    return *stmt.pAssignment;
  }
  // stream override to output the assignment in Verilog
  friend std::ostream& operator<<( std::ostream& out, Statement& s )
  {
    switch( s.id )
    {
      case ASSIGNMENT:
        out << *s.stmt.pAssignment;
        break;
      default:
        throw;
    }
    return out;
  }

private:
  enum ID
  {
    ASSIGNMENT
  };
  union pStatement
  {
    Assignment* pAssignment;
  };
  pStatement stmt;
  ID id;
};

class Statements : public std::list< Statement >
{
public:
  Statement& operator[](iterator i)
  {
    return *i;
  }
  Assignment& addAssignment( Operator& op, Variable& output, Variable& input1, Variable& input2 =
                             Assignment::dummyvar(),
                             Variable& input3 = Assignment::dummyvar(), Variable& other1 = Assignment::dummyvar(),
                             Variable& other2 = Assignment::dummyvar() )
  {
    Assignment* pA = new Assignment( mCount++,op,output,input1,input2,input3,other1,other2 );
    addStatement( *pA );
    return *pA;
  }

  Statements() : mCount( 0 )
  {
  }

private:
  Statement& addStatement( Assignment& a )
  {
    Statement* pS = new Statement( a );
    push_back( *pS );
    return *pS;
  }
  int mCount;
};

#endif//__STATEMENT_H__
