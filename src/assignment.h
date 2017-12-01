#ifndef __assignment_h__
#define __assignment_h__

#include "type.h"
#include "ioclass.h"
#include "variable.h"
#include "operator.h"
#include "latency.h"

#include <list>
#include <iostream>
#include <sstream>

class Assignment
{
public:

  // constructor
  Assignment( int count, Operator& op, Variable& result, Variable& input1, Variable& input2 = dummyvar(),
              Variable& input3 = dummyvar(), Variable& other1 = dummyvar(), Variable& other2 = dummyvar() )
      : ops( Singleton<Operators>::instance()), lats( Singleton<Latencies>::instance() )
      , mCount( count ), mWidth( 0 ), mLatency( 0.0 ), mSigned( false ), mOperator( op ), mResult( result )
      , mOperand1( input1 ), mOperand2( input2 ), mOperand3( input3 ), mOther1( other1 ), mOther2( other2 )
  {
    // determine operator width
    mWidth = result.width();
    if( (op.id() == Operator::GT) || (op.id() == Operator::LT) || (op.id() == Operator::EQ) )
    {
      mWidth = (input1.width() > input2.width()) ? input1.width() : input2.width();
    }

    // calculate latency through the component
    mLatency = lats.getLatency( op,mWidth );

    // determine whether this component should be signed
    mSigned = input1.isSigned() || input2.isSigned() || input3.isSigned();
  }

  // get the assignment operator
  Operator& getOperator()
  {
    return mOperator;
  }

  // get the result variable
  Variable& getResult()
  {
    return mResult;
  }

  // get the number of input arguments
  const int getNumArgs()
  {
    return mOperator.nargs();
  }

  // get the first input argument
  Variable& getInput1()
  {
    return mOperand1;
  }

  // get the second input argument
  Variable& getInput2()
  {
    return mOperand2;
  }

  // get the third input argument
  Variable& getInput3()
  {
    return mOperand3;
  }

  // get the nth argument
  Variable& getInput(int n)
  {
    switch (n)
    {
      case 0:
        return mOperand1;
      case 1:
        return mOperand2;
      case 2:
        return mOperand3;
      default:
        throw;
    }
  }

  // get the instantiation count
  int getCount()
  {
    return mCount;
  }

  // get the component width
  int getWidth()
  {
    return mWidth;
  }

  // get the component latency in nanoseconds
  double getLatency()
  {
    return mLatency;
  }

  // is the component signed?
  bool isSigned()
  {
    return mSigned;
  }

  std::string C_format(bool nonblocking = false)
  {
    std::string out = mResult.name() + " ";
    if (nonblocking)
      out += "<";
    out += ops.getOperatorByID( Operator::REG ).name() + " " + mOperand1.name();
    if( (mOperator.id() == Operator::INC) || (mOperator.id() == Operator::DEC) )
    {
      out += mOperator.name();
    }
    else if( getNumArgs() > 1 )
    {
      out += " " + getOperator().name() + " " + mOperand2.name();
      if( getNumArgs() > 2 )
      {
        out += " " + ops.getOperatorByID( Operator::SELECT ).name() + " " + mOperand3.name();
      }
    }
    return out;
  }

  // stream override to output the assignment in Verilog
  friend std::ostream& operator<<( std::ostream& out, Assignment& a )
  {
    out << a.mOperator.component( a.mSigned ) << " #(.DATAWIDTH(" << a.mWidth << "))";
    out << " inst" << a.mCount << "_" << a.mOperator.component( a.mSigned );
    out << " (" << extend( a.mOperand1,a.mWidth );
    switch( a.mOperator.id() )
    {
      case Operator::REG:
        out << "," << a.mOther1.name();
        out << "," << a.mOther2.name();
        out << "," << a.mResult.name() << ");";
        break;
      case Operator::GT:
        out << "," << extend( a.mOperand2,a.mWidth );
        out << "," << a.mResult.name();
        out << "," << a.mOther1.name();
        out << "," << a.mOther2.name() << ");";
        break;
      case Operator::LT:
        out << "," << extend( a.mOperand2,a.mWidth );
        out << "," << a.mOther1.name();
        out << "," << a.mResult.name();
        out << "," << a.mOther2.name() << ");";
        break;
      case Operator::EQ:
        out << "," << extend( a.mOperand2,a.mWidth );
        out << "," << a.mOther1.name();
        out << "," << a.mOther2.name();
        out << "," << a.mResult.name() << ");";
        break;
      default:
        {
          if( a.mOperator.nargs() > 1 )
            out << "," << extend( a.mOperand2,a.mWidth );
          if( a.mOperator.nargs() > 2 )
            out << "," << extend( a.mOperand3,a.mWidth );
          out << "," << a.mResult.name() << ");";
        }
    }
    /*DEBUG*/out << " // " << a.C_format();
    return out;
  }

  // return reference to a dummy variable to fill in unused operands
  static Variable& dummyvar()
  {
    static Type dummytype( "dummy",1,false );
    static IOClass dummyio( "dummy",IOClass::DUMMY,"dummy" );
    static Variable dummyvar( "dummy",dummytype,dummyio );
    return dummyvar;
  }

private:
  // create, if necessary, a width-extended input
  static std::string extend( Variable& var, int width )
  {
    // first, assume no modification
    std::string result( var.name() );
    // now evaluate variable width versus component width
    if( var.width() < width )
    {
      std::ostringstream tmp;
      if( var.isSigned() )
      {
        // sign extend
        tmp << "{{" << (width - var.width()) << "{" << var.name() << "[" << var.width() - 1 << "]" << "}}," << result
            << "}";
      }
      else
      {
        // pad with zeros
        tmp << "{" << (width - var.width()) << "'b0," << result << "}";
      }
      result = tmp.str();
    }
    return result;
  }

private:
  Operators& ops;
  Latencies& lats;
  int mCount;
  int mWidth;
  double mLatency;
  bool mSigned;
  Operator& mOperator;
  Variable& mResult;
  Variable& mOperand1;
  Variable& mOperand2;
  Variable& mOperand3;
  Variable& mOther1;
  Variable& mOther2;
};

#endif//__assignment_h__
