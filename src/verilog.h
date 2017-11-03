#ifndef __verilog_h__
#define __verilog_h__

#include "singleton.h"
#include "statement.h"
#include "variable.h"
#include <fstream>
#include <string>

class Verilog
{
  friend class Singleton<Verilog>;

public:
  bool process( std::ofstream& out, std::string name, Variables& vars, Statements& stmts );

private:
  Verilog();
};

#endif//__verilog_h__
