#ifndef __operator_h__
#define __operator_h__

#include <string>
#include <map>

class Operator
{
public:
  // enumerations of the component types
  enum ID
  {
    REG, ADD, SUB, MUL, GT, LT, EQ, MUX2x1, SHR, SHL, DIV, MOD, INC, DEC, UNITY, SELECT, NOP
  };

  // constructor
  Operator( std::string name, ID id, const std::string comp, int nargs )
      : mName( name ), mID( id ), mComponent( comp ), mNumArgs( nargs )
  {
  }

  // return the component netlist symbolic name
  std::string& name()
  {
    return mName;
  }

  // return the operator ID enumeration
  ID id()
  {
    return mID;
  }

  // return the component name for the operator;
  // uses the optional 'isSigned' flag to prepend 'S' if necessary
  const std::string component( bool isSigned = false )
  {
    std::string name = mComponent;
    if( isSigned && (mID != REG) && (mID != MUX2x1) )
      name = "S" + name;
    return name;
  }

  // return the number of input arguments for the operator
  const int nargs()
  {
    return mNumArgs;
  }

private:
  std::string mName;
  ID mID;
  std::string mComponent;
  int mNumArgs;
};

class Operators : public std::map< std::string, Operator >
{
  friend class Singleton<Operators>;

public:
  bool isOperator( const std::string oper )
  {
    return find( oper ) != end();
  }
  Operator& getOperator( const std::string oper )
  {
    iterator i = find( oper );
    if( i == end() )
      throw;
    return i->second;
  }
  Operator& getOperatorByComponent( const std::string comp )
  {
    for( iterator i = begin(); i != end(); i++ )
    {
      if( comp == i->second.component() )
        return i->second;
    }
    throw;
  }
  Operator& getOperatorByID( Operator::ID id )
  {
    for( iterator i = begin(); i != end(); i++ )
    {
      if( id == i->second.id() )
        return i->second;
    }
    throw;
  }
  bool isOperatorID( const std::string oper, Operator::ID id )
  {
    iterator i = find( oper );
    return ((i != end()) && (getOperator( oper ).id() == id));
  }
  bool isAssignment( const std::string oper )
  {
    return isOperatorID( oper,Operator::REG );
  }
  bool isUnity( const std::string oper )
  {
    return isOperatorID( oper,Operator::UNITY );
  }
  bool isSelect( const std::string oper )
  {
    return isOperatorID( oper,Operator::SELECT );
  }
  bool isMux( const std::string oper )
  {
    return isOperatorID( oper,Operator::MUX2x1 );
  }
  bool isNOP( const std::string oper )
  {
    return isOperatorID( oper,Operator::NOP );
  }

private:
  typedef std::pair< std::string, Operator > pair_t;

  Operators()
  {
    // NOTE: operator strings that begin with space can be created
    //       by the code, but cannot be introduced by parsing text
    static Operator t_reg( "=",Operator::REG,"REG",1 );
    static Operator t_add( "+",Operator::ADD,"ADD",2 );
    static Operator t_sub( "-",Operator::SUB,"SUB",2 );
    static Operator t_mul( "*",Operator::MUL,"MUL",2 );
    static Operator t_gt( ">",Operator::GT,"COMP",2 );
    static Operator t_lt( "<",Operator::LT,"COMP",2 );
    static Operator t_eq( "==",Operator::EQ,"COMP",2 );
    static Operator t_mux2x1( "?",Operator::MUX2x1,"MUX2x1",3 );
    static Operator t_shr( ">>",Operator::SHR,"SHR",2 );
    static Operator t_shl( "<<",Operator::SHL,"SHL",2 );
    static Operator t_div( "/",Operator::DIV,"DIV",2 );
    static Operator t_mod( "%",Operator::MOD,"MOD",2 );
    static Operator t_inc( " +1",Operator::INC,"INC",1 );
    static Operator t_dec( " -1",Operator::DEC,"DEC",1 );
    static Operator t_unity( "1",Operator::UNITY,"n/a",0 );
    static Operator t_select( ":",Operator::SELECT,"n/a",0 );
    static Operator t_nop( " nop",Operator::NOP,"n/a",0 );
    insert( pair_t( t_reg.name(),t_reg ) );
    insert( pair_t( t_add.name(),t_add ) );
    insert( pair_t( t_sub.name(),t_sub ) );
    insert( pair_t( t_mul.name(),t_mul ) );
    insert( pair_t( t_gt.name(),t_gt ) );
    insert( pair_t( t_lt.name(),t_lt ) );
    insert( pair_t( t_eq.name(),t_eq ) );
    insert( pair_t( t_mux2x1.name(),t_mux2x1 ) );
    insert( pair_t( t_shr.name(),t_shr ) );
    insert( pair_t( t_shl.name(),t_shl ) );
    insert( pair_t( t_div.name(),t_div ) );
    insert( pair_t( t_mod.name(),t_mod ) );
    insert( pair_t( t_inc.name(),t_inc ) );
    insert( pair_t( t_dec.name(),t_dec ) );
    insert( pair_t( t_unity.name(),t_unity ) );
    insert( pair_t( t_select.name(),t_select ) );
    insert( pair_t( t_nop.name(),t_nop ) );
  }
};

#endif//__operator_h__
