#ifndef __STATEMENT_H__
#define __STATEMENT_H__

#include "assignment.h"
#include "if.h"
#include "for.h"
#include "singleton.h"

class Statement
{
public:
  Statement( Assignment& a )
      : stmt
        { .pAssignment = &a }, mID( ASSIGNMENT )
  {
  }

  Statement( IfStatement& i )
      : stmt
        { .pIfStatement = &i }, mID( IF_STATEMENT )
  {
  }

  Statement( ForLoop& f )
      : stmt
        { .pForLoop = &f }, mID( FOR_LOOP )
  {
  }

  Statement( Condition& c )
      : stmt
        { .pCondition = &c }, mID( CONDITION )
  {
  }

  Statement():stmt{ .p = nullptr }, mID( NOP ){}

  enum ID
  {
    NOP,
    ASSIGNMENT,
    IF_STATEMENT,
    FOR_LOOP,
    CONDITION
  };
    
  enum RESOURCE_TYPE
  {
    NONE,
    ADDER_SUB,
    MULTIPLIER,
    LOGICAL,
    DIV_MOD
  };

  ID id(){return mID;}
  
  RESOURCE_TYPE getResource()
  {
    RESOURCE_TYPE resource = NONE;
    
    if( isNOP() )
    {
      resource = NONE; // just being verbose
    }
    else if( isAssignment() )
    {
      switch( assignment().getOperator().id() )
      {
        case Operator::MUL:
          resource = MULTIPLIER;
          break;
        case Operator::DIV:
        case Operator::MOD:
          resource = DIV_MOD;
          break;
        case Operator::DEC:
        case Operator::INC:
        case Operator::SUB:
        case Operator::ADD:
          resource = ADDER_SUB;
          break;
        default:
          resource = LOGICAL;
          break;
      }
    }
    else if( isIfStatement() )
    {
      resource = LOGICAL;
    }
    
    return resource;
  }


  bool isAssignment()
  {
    return ASSIGNMENT == mID;
  }

  Assignment& assignment()
  {
    if( ASSIGNMENT != mID )
      throw;
    return *stmt.pAssignment;
  }

  bool isIfStatement()
  {
    return IF_STATEMENT == mID;
  }

  IfStatement& if_statement()
  {
    if( IF_STATEMENT != mID )
      throw;
    return *stmt.pIfStatement;
  }

  bool isForLoop()
  {
    return FOR_LOOP == mID;
  }

  ForLoop& for_loop()
  {
    if( FOR_LOOP != mID )
      throw;
    return *stmt.pForLoop;
  }

  bool isCondition()
  {
    return CONDITION == mID;
  }

  Condition& condition()
  {
    if( CONDITION != mID )
      throw;
    return *stmt.pCondition;
  }

  bool isNOP()
  {
    return NOP == mID;
  }

  std::string C_format()
  {
    std::string result;

    switch( mID )
    {
      case NOP:
        result = "NOP";
        break;
      case ASSIGNMENT:
        result = stmt.pAssignment->C_format();
        break;
      case IF_STATEMENT:
        result = stmt.pIfStatement->C_format();
        break;
      case FOR_LOOP:
        result = stmt.pForLoop->C_format();
        break;
      case CONDITION:
        result = stmt.pCondition->C_format();
        break;
      default:
        throw;
    }
    return result;
  }

  // stream override to output the assignment in Verilog
  friend std::ostream& operator<<( std::ostream& out, Statement& s )
  {
    switch( s.mID )
    {
      case NOP:
        out << "NOP";
        break;
      case ASSIGNMENT:
        out << *s.stmt.pAssignment;
        break;
      case IF_STATEMENT:
        out << *s.stmt.pIfStatement;
        break;
      case FOR_LOOP:
        out << *s.stmt.pForLoop;
        break;
      case CONDITION:
        out << *s.stmt.pCondition;
        break;
      default:
        throw;
    }
    return out;
  }

  Statement* getStatement()
  {
    return this;
  }

  bool operator==( const Statement& a)
  {
    return (mID == a.mID) && (stmt.p == a.stmt.p);
  }

  int scheduleLatency()
  {
    int latency = 0;

    if( isNOP() )
    {
      latency = 0;
    }
    else if( isAssignment() )
    {
      switch( assignment().getOperator().id() )
      {
        case Operator::MUL:
          latency = 2;
          break;
        case Operator::DIV:
        case Operator::MOD:
          latency = 3;
          break;
        case Operator::DEC:
        case Operator::INC:
        case Operator::SUB:
        case Operator::ADD:
          latency = 1;
          break;
        default:
          latency = 1;
          break;
      }
    }
    else if( isIfStatement() )
    {
      latency = 1;
    }

    return latency;
  }

private:
  union pStatement
  {
    Assignment* pAssignment;
    IfStatement* pIfStatement;
    ForLoop* pForLoop;
    Condition* pCondition;
    void* p;
  };

  pStatement stmt;
  ID mID;
};

class Statements : public std::list< Statement >
{
public:
  Statement& operator[](iterator i)
  {
    return *i;
  }

  Statement& addNOP()
  {
    return addStatement();
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

  IfStatement& addIfStatement( Variable& condition )
  {
    Statements* pT = new Statements;
    Statements* pF = new Statements;
    IfStatement* pI = new IfStatement( condition,pT,pF );
    addStatement( *pI );
    return *pI;
  }

  ForLoop& addForLoop()
  {
    Statements* pI = new Statements;
    Statements* pC = new Statements;
    Statements* pU = new Statements;
    Statements* pB = new Statements;
    ForLoop* pF = new ForLoop(pI, pC, pU, pB);
    addStatement( *pF );
    return *pF;
  }

  Condition& addCondition( Variable& left, Operator& logic, Variable& right)
  {
    Condition* pC = new Condition( left, logic, right);
    addStatement( *pC );
    return *pC;
  }

  Statements() : mCount( 0 )
  {
  }

private:
  Statement& addStatement()
  {
    Statement* pS = new Statement();
    push_back( *pS );
    return *pS;
  }

  Statement& addStatement( Assignment& a )
  {
    Statement* pS = new Statement( a );
    push_back( *pS );
    return *pS;
  }

  Statement& addStatement( IfStatement& i )
  {
    Statement* pS = new Statement( i );
    push_back( *pS );
    return *pS;
  }

  Statement& addStatement( ForLoop& f )
  {
    Statement* pS = new Statement( f );
    push_back( *pS );
    return *pS;
  }

  Statement& addStatement( Condition& c )
  {
    Statement* pS = new Statement( c );
    push_back( *pS );
    return *pS;
  }

  int mCount;
};

#endif//__STATEMENT_H__
