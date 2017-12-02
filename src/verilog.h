#ifndef __verilog_h__
#define __verilog_h__

#include "singleton.h"
#include "statement.h"
#include "variable.h"
#include "graphType.hpp"
#include <fstream>
#include <string>

class Verilog
{
  friend class Singleton<Verilog>;

public:
  bool ComponentDatapath( std::ofstream& out, std::string name, Variables& vars, Statements& stmts );
  bool HLSM( std::ofstream& out, std::string name, Variables& vars, Variables& mvars, graphType& stmts, graphType::ScheduleID id );

private:
  Verilog();
  void DeclareTimescale(std::ofstream& out);
  void DeclareModule(std::ofstream& out, std::string name, Variables& moduleVars);
  void DeclareModule(std::ofstream& out, std::string name, Variables& moduleVars, Variables& modelVars);
  void ListVariables(std::ofstream& out, Variables& vars, bool& comma);
  void DeclareVariableList(std::ofstream& out, Variables& vars, std::string comment="");
  void DeclareVariablesHLSM(std::ofstream& out, Variables& vars);
  void EndModule(std::ofstream& out);

private:
  enum INDENT { CURRENT, THEN_IN, IN = THEN_IN, THEN_OUT, OUT = THEN_OUT, IN_THEN, OUT_THEN, IN_THEN_OUT };
  int mIndent = 0;
  std::string Indent(INDENT i);
  void setIndent(int i){mIndent=i;}
};

#endif//__verilog_h__
