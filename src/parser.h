#ifndef __PARSER_H__
#define __PARSER_H__

#include "variable.h"
#include "ioclass.h"
#include "operator.h"
#include "statement.h"
#include "type.h"
#include "singleton.h"
#include <fstream>

namespace builtIn
{
  extern Variable clock;
  extern Variable reset;
  extern Variable dummy1;
  extern Variable dummy2;
}

class Parser
{
  friend class Singleton< Parser > ;

private:
  Parser();

public:
  bool process( std::ifstream& in, Statements& stmts );

private:
  IOClasses& ios;
  Variables& vars;
  Operators& ops;
  Types& types;
};

#endif//__PARSER_H__

