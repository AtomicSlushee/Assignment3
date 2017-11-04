#ifndef __tokenizer_h__
#define __tokenizer_h__

#include <string>
#include <fstream>

namespace delimiters
{
  const std::string whitespace( " \t\n" );
  const std::string csv( "," + whitespace );
}

class Tokenizer
{
public:
  Tokenizer( std::ifstream& in ) : mIn(&in), mLineNum(0)
  {
    if( !mIn->good() )
      throw;
    reload( nextLine() );
  }

  Tokenizer( const std::string line = "" ) : mIn(nullptr), mLineNum(0)
  {
    static std::ifstream dud;
    mIn = &dud;
    reload( line );
  }

  std::string nextLine()
  {
    std::string next;
    if( mIn->is_open())
    {
      if( !mIn->eof())
      {
        mLineNum++;
        std::getline( *mIn,next );
      }
    }
    return next;
  }

  bool isEOF()
  {
    if( mIn->is_open() )
      return mIn->eof();
    return false;
  }

  void reload( const std::string line )
  {
    mLine = line;
    mPos = mLine.empty() ? std::string::npos : 0;
  }

  bool next_thru( std::string& token, const std::string delims = delimiters::whitespace )
  {
    return next( token, delims, true );
  }

  bool next( std::string& token, const std::string delims = delimiters::whitespace, bool ignoreEOL = false )
  {
    bool succeeded = true;

    // first check for anything requeued
    if( !mReQueue.empty() )
    {
      token = mReQueue.front();
      mReQueue.pop_front();
      return succeeded;
    }

    // if already empty, try reading a line
    if( (mPos == std::string::npos) && ignoreEOL )
    {
      reload( nextLine() );
    }

    // skip leading delimiters
    if( mPos != std::string::npos )
    {
      mPos = mLine.find_first_not_of( delims,mPos );
    }

    // extract delimited token
    if( mPos == std::string::npos )
    {
      succeeded = false;
      token.clear();
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

  int lineNum(){return mLineNum;}

  void reQueue(std::string token){mReQueue.push_front(token);}

  void kill()
  {
    reload(nextLine());
  }
private:
  std::ifstream* mIn;
  std::string mLine;
  std::size_t mPos;
  int mLineNum;
  std::list<std::string> mReQueue;
};

#endif//__tokenizer_h__
