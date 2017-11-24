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
  bool ComponentDatapath( std::ofstream& out, std::string name, Variables& vars, Statements& stmts );
  bool HLSM( std::ofstream& out, std::string name, Variables& vars, Variables& mvars, Statements& stmts );

private:
  Verilog();
  void DeclareTimescale(std::ofstream& out);
  void DeclareModule(std::ofstream& out, std::string name, Variables& vars);
  void DeclareVariableList(std::ofstream& out, Variables& vars);
  void DeclareVariablesHLSM(std::ofstream& out, Variables& vars);
  void EndModule(std::ofstream& out);
};

#endif//__verilog_h__
