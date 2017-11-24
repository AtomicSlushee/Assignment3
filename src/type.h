#ifndef __type_h__
#define __type_h__

#include "width.h"
#include "singleton.h"

#include <string>
#include <sstream>
#include <map>

class Type
{
public:
  Type( const std::string name_, int width_, bool signed_ )
      : mName( name_ ), mWidth( width_ ), mSigned( signed_ )
  {
  }
  const std::string& name()
  {
    return mName;
  }
  int width()
  {
    return mWidth;
  }
  bool isSigned()
  {
    return mSigned;
  }
  bool operator== (const Type& a)
  {
    return (a.mName == mName) && (a.mWidth == mWidth) && (a.mSigned == mSigned);
  }
private:
  std::string mName;
  int mWidth;
  bool mSigned;
};

class Types
{
  friend class Singleton<Types>;

public:
  static Types& instance()
  {
    static Types types;
    return types;
  }
  bool isType( std::string type )
  {
    return mTypes.find( type ) != mTypes.end();
  }
  Type& getType( std::string type )
  {
    types_t::iterator i = mTypes.find( type );
    if( i == mTypes.end() )
      throw -1;
    return i->second;
  }

private:
  typedef std::pair< std::string, Type > pair_t;
  typedef std::map< std::string, Type > types_t;

  Types()
  {
    static std::string Signed( "Int" );
    static std::string Unsigned( "UInt" );
    std::ostringstream tmp;
    for( int i = 0; i < width::count; i++ )
    {
      tmp.str( "" );
      tmp << Signed << width::set[i];
      mTypes.insert( pair_t( tmp.str(),Type( tmp.str(),width::set[i],true ) ) );
      tmp.str( "" );
      tmp << Unsigned << width::set[i];
      mTypes.insert( pair_t( tmp.str(),Type( tmp.str(),width::set[i],false ) ) );
    }
  }

  types_t mTypes;
};

#endif//__type_h__
