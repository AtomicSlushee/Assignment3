#include "for.h"
#include "statement.h"

std::string ForLoop::C_format()
{
  std::string out;
  out = "for (" + mInitial->begin()->C_format() + "; " +
        mCondition->begin()->C_format() + "; " +
        mUpdate->begin()->C_format() + " )";
  return out;
}

// stream override to output the statement in Verilog
std::ostream& operator<<( std::ostream& out, ForLoop& a )
{
  out << " // " << a.C_format();
  return out;
}

