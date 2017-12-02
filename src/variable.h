#ifndef __variable_h__
#define __variable_h__

#include "type.h"
#include "ioclass.h"

#include <string>
#include <vector>
#include <iostream>

class Variable
{
public:
  // constructor
  Variable( std::string name_, Type& type_, IOClass& ioclass_ )
      : mName( name_ ), mType( type_ ), mIOClass( ioclass_ ), mConst(false), mValue( 0 )
  {
  }

  // constructor for constant
  Variable( std::string name_, long const_ )
      : mName( name_ ),
        mType(Singleton<Types>::instance().getType("Int32")),
        mIOClass( Singleton<IOClasses>::instance().getIOClass("const")),
        mConst( true ),
        mValue( const_ )
  {
  }

  // get the variable name
  std::string& name()
  {
    return mName;
  }

  // return whether the variable is signed
  bool isSigned()
  {
    return mType.isSigned();
  }

  // return whether the variable is actually a constant
  bool isConstant()
  {
    return mConst;
  }

  // return the bit width of the variable
  int width()
  {
    return mType.width();
  }

  // return the variable IO class ID (see enumerations in ioclass.h)
  IOClass::ID ioClass()
  {
    return mIOClass.id();
  }

  // return the variable in a C format
  std::string C_format()
  {
    std::string out;
    if( mConst )
    {
      out = std::to_string( mValue );
    }
    else
    {
      out = mName;
    }
    return out;
  }

  // stream overload to output the variable declaration in Verilog
  friend std::ostream& operator<<( std::ostream& out, Variable& a )
  {
    if( !a.mConst )
    {
      out << a.mIOClass.vname();
      if( a.width() > 1 )
        out << " [" << a.width() - 1 << ":0]";
      out << " " << a.name() << ";";
    }
    return out;
  }

  bool operator== (const Variable& a)
  {
    return (a.mIOClass == mIOClass) &&
           (a.mType == mType) &&
           (a.mName == mName);
  }

  long getValue()
  {
    return mValue;
  }

private:
  std::string mName;
  Type& mType;
  IOClass& mIOClass;
  bool mConst;
  long mValue;
};

class Variables : public std::vector< std::pair< std::string, Variable >>
{
private:
  typedef std::pair< std::string, Variable > pair_t;
  typedef std::vector< pair_t > vector_t;

public:
  static std::string nameState(){static std::string name="state"; return name;}

public:
  Variable& operator[](iterator i)
  {
    return i->second;
  }
  bool isVariable( std::string variable )
  {
    return Find( variable ) != end();
  }
  Variable& getVariable( std::string variable )
  {
    iterator i = Find( variable );
    if( i == end() )
      throw;
    return i->second;
  }
  Variable& addVariable( std::string variable, Type& type, IOClass& ioclass )
  {
    iterator i = Find( variable );
    if( i != end() )
      throw;
    Variable* pV = new Variable( variable,type,ioclass );
    push_back( pair_t( variable,*pV ) );
    return *pV;
  }
  Variable& addVariable( Variable& variable )
  {
    iterator i = Find( variable.name() );
    if( i != end() )
      throw;
    push_back( pair_t( variable.name(),variable ) );
    return variable;
  }

private:
  vector_t::iterator Find( std::string variable )
  {
    for( auto i = begin(); i != end(); i++ )
      if( i->first == variable )
        return i;
    return end();
  }

protected:
  Variables()
  {
  }
};

class ModuleVariables: public Variables
{
};

class ModelVariables: public Variables
{
};

#endif//__variable_h__
