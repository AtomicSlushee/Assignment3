#ifndef __tokenizer_h__
#define __tokenizer_h__

#include <string>

namespace delimiters
{
  const std::string whitespace( " \t\n" );
  const std::string csv( "," + whitespace );
}

class Tokenizer
{
public:
  Tokenizer( const std::string line )
      : mLine( line )
  {
    mPos = mLine.empty() ? std::string::npos : 0;
  }
  bool next( std::string& token, const std::string delims = delimiters::whitespace )
  {
    bool succeeded = true;

    // skip leading delimiters
    if( mPos != std::string::npos )
    {
      mPos = mLine.find_first_not_of( delims,mPos );
    }

    // extract delimited token
    if( mPos == std::string::npos )
    {
      succeeded = false;
    }
    else
    {
      std::size_t last = mLine.find_first_of( delims,mPos );
      if( last == std::string::npos )
      {
        token = mLine.substr( mPos );
        mPos = last;
      }
      else
      {
        token = mLine.substr( mPos,last - mPos );
        mPos = mLine.find_first_not_of( delims,last );
      }
    }

    return succeeded;
  }
  void kill()
  {
    mPos = std::string::npos;
  }
private:
  std::string mLine;
  std::size_t mPos;
};

#endif//__tokenizer_h__
