#ifndef __keyword_h__
#define __keyword_h__

#include <string>
#include <list>

class Keyword
{
public:
  enum ID
  {
    INVALID=-1,
    NONE,
    IF,
    ELSE,
    FOR,
    LEFT_PAREN,
    RIGHT_PAREN,
    OPEN_BRACE,
    CLOSE_BRACE,
    SEMICOLON
  };

  Keyword( std::string keyword_, ID id_ ) : mID( id_), keyword( keyword_ ){}
  std::string name() {return keyword;}
  ID id() {return mID;}

private:
  ID mID;
  std::string keyword;
};

class Keywords : public std::list<Keyword>
{
  friend class Singleton<Keywords>;

public:
  Keyword::ID isKeyword(std::string keyword)
  {
    for( iterator i = begin(); i != end(); i++)
    {
      if( i->name() == keyword )
        return i->id();
    }
    return Keyword::INVALID;
  }

private:
  Keywords()
  {
    static Keyword t_if( "if",Keyword::IF );
    static Keyword t_else( "else",Keyword::ELSE );
    static Keyword t_for( "for",Keyword::FOR );
    static Keyword t_left( "(", Keyword::LEFT_PAREN );
    static Keyword t_right( ")", Keyword::RIGHT_PAREN );
    static Keyword t_open( "{", Keyword::OPEN_BRACE );
    static Keyword t_close( "}", Keyword::CLOSE_BRACE );
    static Keyword t_semi( ";", Keyword::SEMICOLON );
    push_back( t_if );
    push_back( t_for );
    push_back( t_else );
    push_back( t_left );
    push_back( t_right );
    push_back( t_open );
    push_back( t_close );
    push_back( t_semi );
  }

};

#endif//__keyword_h__
