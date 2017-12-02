#include "verilog.h"
#include "parser.h"
#include "scheduler.h"

Verilog::Verilog()
{
}

std::string Verilog::Indent( INDENT i = CURRENT )
{
  static std::string in = "  ";
  auto go = [](int indent){std::string x; if(indent>0)while(indent--)x+=in;return x;};
  switch( i )
  {
    case THEN_IN: return go(mIndent++);
    case THEN_OUT: return go(mIndent--);
    case IN_THEN: return go(++mIndent);
    case OUT_THEN: return go(--mIndent);
    case IN_THEN_OUT: return go(mIndent+1);
    case CURRENT:
    default:
      return go(mIndent);
  }
}

bool Verilog::ComponentDatapath( std::ofstream& out, std::string name, Variables& vars, Statements& stmts )
{
  bool success = true;

  setIndent( 0 );
  DeclareTimescale( out );
  DeclareModule( out,"m_" + name,vars );
  Indent( IN );
  DeclareVariableList( out,vars );

  // output the assignments
  for( Statements::iterator i = stmts.begin(); i != stmts.end(); i++ )
  {
    out << Indent() << stmts[i] << std::endl;
  }

  Indent( OUT );
  EndModule( out );

  return success;
}

bool Verilog::HLSM( std::ofstream& out, std::string name, Variables& vars, Variables& mvars, graphType& stmts, graphType::ScheduleID id = graphType::FDS )
{
  bool success = true;
  Scheduler& scheduler = Singleton< Scheduler >::instance();
  Scheduler::partitionMap_t m;

  // build the scheduling map
  scheduler.buildPartTimeMap(m, stmts, id);

  // start with the various module declarations
  setIndent( 0 );
  DeclareModule( out,"HLSM",vars );
  Indent( IN );
  DeclareVariableList( out,vars );
  DeclareVariableList( out,mvars,"Model Variables" );

  out << Indent() << "// Model Procedure" << std::endl;

  // KLUDGE: just put the initial section in by hand
  out << Indent( THEN_IN ) << "initial begin" << std::endl;
  out << Indent() << builtIn::done.name() << " <= 0;" << std::endl;
  out << Indent() << Variables::nameState() << " <= 0;" << std::endl;
  out << Indent( OUT_THEN ) << "end" << std::endl << std::endl;

  // now for the state machine itself
  out << Indent( THEN_IN ) << "always @(posedge " << builtIn::clock.name() << ") begin" << std::endl;

  // first, kludge the reset check and beginning of case statement
  out << Indent( THEN_IN ) << "if (" << builtIn::reset.name() << " == 1)" << std::endl;
  out << Indent( THEN_OUT ) << Variables::nameState() << " <= 0;" << std::endl;
  out << Indent( THEN_IN ) << "else case (" << Variables::nameState() << ")" << std::endl;

  // the zero state is a freebie, but the last part gets filled in by the loop below
  out << Indent( THEN_IN ) << "0: begin" << std::endl;
  out << Indent() << builtIn::done.name() << " <= 0;" << std::endl;
  out << Indent( THEN_IN ) << "if (" << builtIn::start.name() << " == 1)" << std::endl;

  // need to recognize the last statement
  auto lastStmt = std::prev(std::prev(std::prev(m.end())->second.end())->second.end());

  // body using the map of statements passed in
  int lastState = 0;
  for( auto i = std::next(m.begin()); i != m.end(); i++)
  {
    for( auto j = i->second.begin(); j != i->second.end(); j++)
    {
      for( auto k = j->second.begin(); k != j->second.end(); k++ )
      {
        graphType::vertex_t* s = *k;
        if( lastState != s->helper.schedTime[id])
        {
          // handle empty wait states
          while( lastState+1 < s->helper.schedTime[id] )
          {
            lastState++;
            out << Indent( THEN_OUT ) << Variables::nameState() << " <= " << lastState << ";" << std::endl;
            if( 1 == lastState ) Indent(OUT);
            out << Indent() << "end" << std::endl;
            out << Indent( THEN_IN ) << lastState << ": begin" << std::endl;
          }
          // now transition to next state
          lastState = s->helper.schedTime[id];
          out << Indent( THEN_OUT ) << Variables::nameState() << " <= " << lastState << ";" << std::endl;
          if( 1 == lastState ) Indent(OUT);
          out << Indent() << "end" << std::endl;
          out << Indent( THEN_IN ) << lastState << ": begin" << std::endl;
        }
        if( k == lastStmt )
        {
          // last state/statement
          out << Indent() << builtIn::done.name() << " <= 1;" << std::endl;
          out << Indent( THEN_OUT ) << Variables::nameState() << " <= 0;" << std::endl;
          out << Indent() << "end" << std::endl;
        }
        else
        {
          if( s->getNode().get().isElse() )
          {
            out << Indent( THEN_OUT ) << Variables::nameState() << " <= " << m[s->helper.nextPart].begin()->first << ";" << std::endl;
            out << Indent() << "end" << std::endl;
            lastState++;
            out << Indent( THEN_IN ) << lastState << ": begin" << std::endl;
          }
          else if( !s->getNode().get().isNOP() )
          {
            if( s->getNode().get().isIfStatement())
            {
              out << Indent( THEN_IN ) << s->getNode().get().C_format(true) << std::endl;
              out << Indent( THEN_OUT) << Variables::nameState() << " <= " << (++lastState) << ";" << std::endl;
              out << Indent( THEN_OUT) << Variables::nameState() << " <= " << m[s->helper.nextPart].begin()->first << ";" << std::endl;
              out << Indent() << "end" << std::endl;
              out << Indent( THEN_IN ) << lastState << ": begin" << std::endl;
            }
            else
            {
              out << Indent() << s->getNode().get().C_format(true) << ";" << std::endl;
            }
          }
          else
          {
            // nothing to do for these particular NOPs
          }
        }
      }
    }
  }

  out << Indent( OUT_THEN ) << "endcase" << std::endl;
  out << Indent( OUT_THEN ) << "end" << std::endl;

  Indent( OUT );
  EndModule( out );

  return success;
}

void Verilog::DeclareTimescale( std::ofstream& out )
{
  // timescale
  out << Indent() << "`timescale 1ns / 1ps" << std::endl;
}

void Verilog::DeclareModule(std::ofstream& out, std::string name, Variables& vars)
{
  bool comma = false;

  // declare module
  out << Indent() << "module " << name << "(";
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
  out << Indent() << "endmodule" << std::endl;
}

void Verilog::DeclareVariableList(std::ofstream& out, Variables& vars, std::string comment)
{
  // if there's a comment, use it
  if( !comment.empty() )
  {
    out << Indent() << "// " << comment << std::endl;
  }

  // output the variable declarations
  for (Variables::iterator i = vars.begin(); i != vars.end(); i++)
  {
    out << Indent() << vars[i] << std::endl;
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
