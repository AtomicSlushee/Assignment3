#ifndef __PARSER_H__
#define __PARSER_H__

#include "variable.h"
#include "ioclass.h"
#include "operator.h"
#include "statement.h"
#include "type.h"
#include "singleton.h"
#include "keyword.h"
#include "tokenizer.h"
#include <fstream>

namespace builtIn
{
  extern Variable clock;
  extern Variable reset;
  extern Variable start;
  extern Variable done;
  extern Variable dummy1;
  extern Variable dummy2;
}

class Parser
{
  friend class Singleton< Parser > ;

private:
  Parser();

public:
  bool process( std::ifstream& in, ModuleVariables& vars, ModelVariables& mvars, Statements& stmts, bool hlsm=false );

private:
  bool processMore( Variables& vars, Statements& stmts, Tokenizer& tokens, Keyword::ID stopOn );
  bool processAssignment( Variables& vars, Statements& stmts, Tokenizer& tokens, std::string token, std::string stopOn = delimiters::none );
  void addClkReset(Variables& v);
  void addDummies(Variables& v);
  void addStartDone(Variables& v);
  IOClasses& ios;
  Operators& ops;
  Types& types;
  Keywords& keys;
  bool addedClkRst;
  bool addedDummies;
  bool addedStartDone;
};

#endif//__PARSER_H__

