#include "verilog.h"
#include "parser.h"

static const char* indent = "    ";

Verilog::Verilog()
{
}

bool Verilog::ComponentDatapath( std::ofstream& out, std::string name, Variables& vars, Statements& stmts )
{
  bool success = true;

  DeclareTimescale(out);
  DeclareModule(out,"m_"+name,vars);
  DeclareVariableList(out,vars);

  // output the assignments
  for (Statements::iterator i = stmts.begin(); i != stmts.end(); i++)
  {
    out << indent << stmts[i] << std::endl;
  }

  EndModule(out);

  return success;
}

bool Verilog::HLSM( std::ofstream& out, std::string name, Variables& vars, Variables& mvars, Statements& stmts )
{
  bool success = true;

  DeclareModule( out, "HLSM", vars);
  DeclareVariableList(out,vars);
  DeclareVariableList(out,mvars,"Model Variables");

  out << indent << "// Model Procedure" << std::endl;
  out << indent << "always @(posedge " << builtIn::clock.name() << ") begin" << std::endl;

  //TODO

  out << indent << "end" << std::endl;

  EndModule(out);

  return success;
}

void Verilog::DeclareTimescale(std::ofstream& out)
{
  // timescale
  out << "`timescale 1ns / 1ps" << std::endl;
}

void Verilog::DeclareModule(std::ofstream& out, std::string name, Variables& vars)
{
  bool comma = false;

  // declare module
  out << "module " << name << "(";
  for (Variables::iterator i = vars.begin(); i != vars.end(); i++)
  {
    if( vars[i].ioClass() == IOClass::INPUT )
    {
      if( comma )
        out << ",";
      out << vars[i].name();
      comma = true;
    }
  }
  for (Variables::iterator i = vars.begin(); i != vars.end(); i++)
  {
    if( vars[i].ioClass() == IOClass::OUTPUT )
    {
      if( comma )
        out << ",";
      out << vars[i].name();
      comma = true;
    }
  }
  out << ");" << std::endl;
}

void Verilog::EndModule(std::ofstream& out)
{
  // end the module
  out << "endmodule" << std::endl;
}

void Verilog::DeclareVariableList(std::ofstream& out, Variables& vars, std::string comment)
{
  // if there's a comment, use it
  if( !comment.empty() )
  {
    out << indent << "// " << comment << std::endl;
  }

  // output the variable declarations
  for (Variables::iterator i = vars.begin(); i != vars.end(); i++)
  {
    out << indent << vars[i] << std::endl;
  }

  out << std::endl;
}

//void Verilog::DeclareVariablesHLSM(std::ofstream& out, Variables& vars)
//{
//  out << indent << builtIn::clock << std::endl;
//  out << indent << builtIn::reset << std::endl;
//  out << indent << builtIn::start << std::endl;
//  out << indent << builtIn::done << std::endl;
//
//  out << std::endl;
//}
