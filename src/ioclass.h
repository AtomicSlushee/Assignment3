#ifndef __ioclass_h__
#define __ioclass_h__

#include "singleton.h"
#include <string>
#include <map>

class IOClass
{
public:
  enum ID
  {
    DUMMY = -1, INPUT, OUTPUT, WIRE, REGISTER, VARIABLE, CONSTANT
  };
  IOClass( const std::string name_, ID id, const std::string vn )
      : mName( name_ ), vName( vn ), mID( id )
  {
  }
  const std::string& name()
  {
    return mName;
  }
  const std::string& vname()
  {
    return vName;
  }
  ID id()
  {
    return mID;
  }
  bool operator== (const IOClass& a)
  {
    return (a.mName == mName) && (a.mID == mID) && (a.vName == vName);
  }
private:
  std::string mName;
  std::string vName;
  ID mID;
};

class IOClasses : public std::map< std::string, IOClass >
{
  friend class Singleton<IOClasses>;

public:
  bool isIOClass( const std::string ioclass )
  {
    return find( ioclass ) != end();
  }
  IOClass& getIOClass( const std::string ioclass )
  {
    iterator i = find( ioclass );
    if( i == end() )
      throw;
    return i->second;
  }

private:
  typedef std::pair< std::string, IOClass > pair_t;

  IOClasses()
  {
    static IOClass t_input( "input",IOClass::INPUT,"input" );
    static IOClass t_output( "output",IOClass::OUTPUT,"output reg" );
    static IOClass t_wire( "wire",IOClass::WIRE,"wire" );
    static IOClass t_register( "register",IOClass::REGISTER,"reg" );
    static IOClass t_variable( "variable",IOClass::VARIABLE,"reg" );
    static IOClass t_outreg( "output reg",IOClass::OUTPUT,"output reg" );
    static IOClass t_const( "const", IOClass::CONSTANT, "const" );
    insert( pair_t( t_input.name(),t_input ) );
    insert( pair_t( t_output.name(),t_output ) );
    insert( pair_t( t_wire.name(),t_wire ) );
    insert( pair_t( t_register.name(),t_register ) );
    insert( pair_t( t_variable.name(),t_variable ) );
    insert( pair_t( t_outreg.name(),t_outreg ) );
    insert( pair_t( t_const.name(),t_const ) );
  }
};

#endif//__ioclass_h__
