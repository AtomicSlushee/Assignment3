#ifndef __latency_h__
#define __latency_h__

#include "operator.h"

#include <map>

class LatencyKey
{
public:
  LatencyKey( Operator& op, int width )
      : mOperator( op ), mWidth( width )
  {
  }
  bool operator<( const LatencyKey& k ) const
  {
    if( this->mOperator.name() == k.mOperator.name() )
      return this->mWidth < k.mWidth;
    return this->mOperator.name() < k.mOperator.name();
  }
private:
  Operator& mOperator;
  int mWidth;
};

class Latencies : public std::map< LatencyKey, double >
{
  friend class Singleton<Latencies>;

public:
  double getLatency( Operator& op, int width )
  {
    iterator i = find( key_t( op,width ) );
    if( i == end() )
      throw;
    return i->second;
  }

private:
  typedef LatencyKey key_t;
  typedef std::pair< key_t, double > pair_t;

  Latencies() : ops( Singleton<Operators>::instance() )
  {
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),1 ),2.616 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),2 ),2.644 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),8 ),2.879 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),16 ),3.061 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),32 ),3.602 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::REG ),64 ),3.966 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),1 ),2.704 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),2 ),3.713 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),8 ),4.924 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),16 ),5.638 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),32 ),7.270 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::ADD ),64 ),9.566 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),1 ),3.024 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),2 ),3.412 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),8 ),4.890 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),16 ),5.569 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),32 ),7.253 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SUB ),64 ),9.566 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),1 ),2.438 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),2 ),3.412 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),8 ),7.453 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),16 ),7.811 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),32 ),12.395 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUL ),64 ),15.354 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),1 ),3.031 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),2 ),3.934 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),8 ),5.949 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),16 ),6.256 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),32 ),7.264 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::GT ),64 ),8.416 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),1 ),3.031 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),2 ),3.934 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),8 ),5.949 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),16 ),6.256 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),32 ),7.264 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::LT ),64 ),8.416 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),1 ),3.031 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),2 ),3.934 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),8 ),5.949 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),16 ),6.256 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),32 ),7.264 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::EQ ),64 ),8.416 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),1 ),4.083 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),2 ),4.115 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),8 ),4.815 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),16 ),5.623 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),32 ),8.079 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MUX2x1 ),64 ),8.766 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),1 ),3.644 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),2 ),4.007 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),8 ),5.178 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),16 ),6.460 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),32 ),8.819 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHR ),64 ),11.095 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),1 ),3.614 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),2 ),3.980 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),8 ),5.152 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),16 ),6.549 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),32 ),8.565 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::SHL ),64 ),11.220 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),1 ),0.619 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),2 ),2.144 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),8 ),15.439 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),16 ),33.093 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),32 ),86.312 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DIV ),64 ),243.233 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),1 ),0.758 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),2 ),2.149 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),8 ),16.078 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),16 ),35.563 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),32 ),88.142 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::MOD ),64 ),250.583 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),1 ),1.792 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),2 ),2.218 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),8 ),3.111 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),16 ),3.471 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),32 ),4.347 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::INC ),64 ),6.200 ) );

    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),1 ),1.792 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),2 ),2.218 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),8 ),3.108 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),16 ),3.701 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),32 ),4.685 ) );
    insert( pair_t( key_t( ops.getOperatorByID( Operator::DEC ),64 ),6.503 ) );
  }

  Operators& ops;
};

#endif//__latency_h__
