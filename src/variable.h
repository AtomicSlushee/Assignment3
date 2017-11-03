#ifndef __variable_h__
#define __variable_h__

#include "type.h"
#include "ioclass.h"

#include <string>
#include <map>
#include <iostream>

class Variable
{
public:
  // constructor
  Variable( std::string name_, Type& type_, IOClass& ioclass_ )
      : mName( name_ ), mType( type_ ), mIOClass( ioclass_ )
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

  // stream overload to output the variable declaration in Verilog
  friend std::ostream& operator<<( std::ostream& out, Variable& a )
  {
    out << a.mIOClass.name();
    if( a.width() > 1 )
      out << " [" << a.width() - 1 << ":0]";
    out << " " << a.name() << ";";
    return out;
  }

private:
  std::string mName;
  Type& mType;
  IOClass& mIOClass;
};

class Variables : public std::map< std::string, Variable >
{
  friend class Singleton<Variables>;

private:
  typedef std::pair< std::string, Variable > pair_t;

public:
  Variable& operator[](iterator i)
  {
    return i->second;
  }
  bool isVariable( std::string variable )
  {
    return find( variable ) != end();
  }
  Variable& getVariable( std::string variable )
  {
    iterator i = find( variable );
    if( i == end() )
      throw;
    return i->second;
  }
  Variable& addVariable( std::string variable, Type& type, IOClass& ioclass )
  {
    iterator i = find( variable );
    if( i != end() )
      throw;
    Variable* pV = new Variable( variable,type,ioclass );
    insert( pair_t( variable,*pV ) );
    return *pV;
  }
  Variable& addVariable( Variable& variable )
  {
    iterator i = find( variable.name() );
    if( i != end() )
      throw;
    insert( pair_t( variable.name(),variable ) );
    return variable;
  }

private:
  Variables()
  {
  }
};

#endif//__variable_h__
