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
    DUMMY = -1, INPUT, OUTPUT, WIRE, REGISTER, VARIABLE
  };
  IOClass( const std::string name_, ID id )
      : mName( name_ ), mID( id )
  {
  }
  const std::string& name()
  {
    return mName;
  }
  ID id()
  {
    return mID;
  }
  bool operator== (const IOClass& a)
  {
    return (a.mName == mName) && (a.mID == mID);
  }
private:
  std::string mName;
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
    static IOClass t_input( "input",IOClass::INPUT );
    static IOClass t_output( "output",IOClass::OUTPUT );
    static IOClass t_wire( "wire",IOClass::WIRE );
    static IOClass t_register( "register",IOClass::REGISTER );
    static IOClass t_variable( "variable",IOClass::VARIABLE );
    static IOClass t_outreg( "output reg",IOClass::OUTPUT );
    insert( pair_t( t_input.name(),t_input ) );
    insert( pair_t( t_output.name(),t_output ) );
    insert( pair_t( t_wire.name(),t_wire ) );
    insert( pair_t( t_register.name(),t_register ) );
    insert( pair_t( t_variable.name(),t_variable ) );
    insert( pair_t( t_outreg.name(),t_outreg ) );
  }
};

#endif//__ioclass_h__
