#ifndef __tokenizer_h__
#define __tokenizer_h__

#include <string>
#include <fstream>

namespace delimiters
{
  const std::string whitespace( " \t\n" );
  const std::string csv( "," + whitespace );
  const std::string semicolon( ";" );
  const std::string rightparen( ")" );
  const std::string leftparen( "(" );
  const std::string none("");
}

class Tokenizer
{
public:
  Tokenizer( std::ifstream& in ) : mIn(&in), mLineNum(0), mSpecial(false)
  {
    if( !mIn->good() )
      throw;
    reload( nextLine() );
  }

  Tokenizer( const std::string line = "" ) : mIn(nullptr), mLineNum(0), mSpecial(false)
  {
    static std::ifstream dud;
    mIn = &dud;
    reload( line );
  }

  bool hitSpecial(){return mSpecial;}

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

  bool next_thru( std::string& token, const std::string delims = delimiters::whitespace, const std::string special = "" )
  {
    return next( token, delims, special, true );
  }

  bool next_special( std::string& token, const std::string special, const std::string delims = delimiters::whitespace)
  {
    return next( token, delims, special, false );
  }

  bool next( std::string& token, const std::string delims = delimiters::whitespace, const std::string special = "", bool ignoreEOL = false )
  {
    bool succeeded = true;

    mSpecial = false;

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
      std::size_t last = mLine.find_first_of( delims+special,mPos );
      if( last == std::string::npos )
      {
        token = mLine.substr( mPos );
        mPos = last;
      }
      else
      {
        token = mLine.substr( mPos,last - mPos );
        mPos = mLine.find_first_not_of( delims,last );
        mSpecial = (mLine.find_first_of(special,mPos) == mPos);
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
  bool mSpecial;
};

#endif//__tokenizer_h__
