#include "verilog.h"

static const char* indent = "    ";

Verilog::Verilog()
{
}

bool Verilog::process( std::ofstream& out, std::string name, Variables& vars, Statements& stmts )
{
  bool success = true;
  bool comma = false;

  // timescale
  out << "`timescale 1ns / 1ps" << std::endl;

  // declare module
  out << "module m_" << name << "(";
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

  // output the variable declarations
  for (Variables::iterator i = vars.begin(); i != vars.end(); i++)
  {
    out << indent << vars[i] << std::endl;
  }

  out << std::endl;

  // output the assignments
  for (Statements::iterator i = stmts.begin(); i != stmts.end(); i++)
  {
    out << indent << stmts[i] << std::endl;
  }

  // end the module
  out << "endmodule" << std::endl;

  return success;
}

