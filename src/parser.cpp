#include "hlsyn.h"
#include "parser.h"
#include "tokenizer.h"
#include "latency.h"
#include <cstdio>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <libgen.h>

// define the clock and reset, just in case we need them
namespace builtIn
{
  Variable clock( std::string( "__clk" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "input" ) );
  Variable reset( std::string( "__rst" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "input" ) );
  Variable dummy1( std::string( "__na1" ),
                   Singleton<Types>::instance().getType( "UInt1" ),
                   Singleton<IOClasses>::instance().getIOClass( "wire" ) );
  Variable dummy2( std::string( "__na2" ),
                   Singleton<Types>::instance().getType( "UInt1" ),
                   Singleton<IOClasses>::instance().getIOClass( "wire" ) );
}

Parser::Parser()
    : ios( Singleton<IOClasses>::instance() )
    , vars( Singleton<Variables>::instance() )
    , ops( Singleton<Operators>::instance() )
    , types (Singleton<Types>::instance() )
{
}

bool Parser::process( std::ifstream& in, Statements& stmts )
{
  bool addedClkRst = false;
  bool addedDummies = false;
  std::string line;
  int lineNum = 0;

  // for all lines in the circuit file
  while( std::getline( in,line ) )
  {
    lineNum++;
    Tokenizer tokens( line );
    std::string token;
    while( tokens.next( token ) )
    {
      // skip comments explicitly
      if( (token.length() > 1) && (token[0] == '/') && (token[1] == '/') )
        break; // go to next line

      // is it an IOClass declaration?
      if( ios.isIOClass( token ) )
      {
        std::string ttoken;
        if( tokens.next( ttoken ) )
        {
          if( types.isType( ttoken ) )
          {
            Type& type = types.getType( ttoken );
            IOClass& ioclass = ios.getIOClass( token );
            std::string vtoken;
            while( tokens.next( vtoken,delimiters::csv ) )
            {
              vars.addVariable( vtoken,type,ioclass );
              DEBUGCOUT<< "added variable " << vtoken << " of type " << ttoken << " with IO class " << token
              << std::endl;
            }
          }
          else
          {
            fprintf( stderr,"error: invalid data type %s on line %d\n",ttoken.c_str(),lineNum );
            return false;
          }
        }
        else
        {
          fprintf( stderr,"error: missing data type for IO Class %s on line %d\n",token.c_str(),lineNum );
          return false;
        }
        tokens.kill();
      }
      // must be a variable assignment
      else
      {
        if( vars.isVariable( token ) )
        {
          Variable& result = vars.getVariable( token );
          std::string etoken;
          if( tokens.next( etoken ) && ops.isAssignment( etoken ) )
          {
            DEBUGCOUT<< "assigning " << token << " equals ";
            std::vector< std::string > args;
            while( tokens.next( etoken ) )
            {
              args.push_back( etoken );
            }
            if( args.size() )
            {
              if( vars.isVariable( args[0] ) )
              {
                Variable& input1 = vars.getVariable( args[0] );
                if( args.size() == 1 )
                {
                  DEBUGCOUT << "variable " << args[0] << " with REG";
                  if( !addedClkRst )
                  {
                    vars.addVariable( builtIn::clock );
                    vars.addVariable( builtIn::reset );
                    addedClkRst = true;
                  }
                  stmts.addAssignment( ops.getOperatorByID( Operator::REG ),result,input1,Assignment::dummyvar(),
                      Assignment::dummyvar(),builtIn::clock,builtIn::reset );
                }
                else if( (args.size() == 3) && (ops.isOperator( args[1] )) )
                {
                  Operator& op = ops.getOperator( args[1] );
                  bool unity = ops.isUnity( args[2] );
                  if( (op.id() == Operator::ADD) && unity )
                  {
                    DEBUGCOUT << "INC 1";
                    stmts.addAssignment( ops.getOperatorByID( Operator::INC ),result,input1 );
                  }
                  else if( (op.id() == Operator::SUB) && unity )
                  {
                    DEBUGCOUT << "DEC 1";
                    stmts.addAssignment( ops.getOperatorByID( Operator::DEC ),result,input1 );
                  }
                  else if( vars.isVariable( args[2] ) )
                  {
                    Variable& input2 = vars.getVariable( args[2] );
                    DEBUGCOUT << args[0] << " " << op.component() << " " << args[2];
                    if( (op.id() == Operator::GT) || (op.id() == Operator::LT) || (op.id() == Operator::EQ) )
                    {
                      if( !addedDummies )
                      {
                        vars.addVariable( builtIn::dummy1 );
                        vars.addVariable( builtIn::dummy2 );
                        addedDummies = true;
                      }
                      stmts.addAssignment( op,result,input1,input2,Assignment::dummyvar(),builtIn::dummy1,
                          builtIn::dummy2 );
                    }
                    else
                    {
                      stmts.addAssignment( op,result,input1,input2 );
                    }
                  }
                  else
                  {
                    fprintf( stderr,"error: unknown variable %s on line %d\n",args[2].c_str(),lineNum );
                    return false;
                  }
                }
                else if( (args.size() == 5) && (ops.isMux( args[1] )) )
                {
                  if( vars.isVariable( args[2] ) && vars.isVariable( args[4] ) && ops.isSelect( args[3] ) )
                  {
                    DEBUGCOUT << "MUX " << args[0] << " selects " << args[2] << " or " << args[4];
                    stmts.addAssignment( ops.getOperatorByID( Operator::MUX2x1 ),result,vars.getVariable( args[0] ),
                        vars.getVariable( args[2] ),vars.getVariable( args[4] ) );
                  }
                  else
                  {
                    fprintf( stderr,"error: malformed mux on line %d\n",lineNum );
                    return false;
                  }
                }
                else
                {
                  fprintf( stderr,"error: malformed assignment on line %d\n",lineNum );
                  return false;
                }
              }
              else
              {
                fprintf( stderr,"error: argument for assignment to %s is not a variable on line %d\n",token.c_str(),
                    lineNum );
                return false;
              }
            }
            else
            {
              fprintf( stderr,"error: assignment to %s has no arguments on line %d\n",token.c_str(),lineNum );
              return false;
            }
            DEBUGCOUT << std::endl;
            tokens.kill();
          }
          else
          {
            fprintf( stderr,"error: no assignment operator for variable %s on line %d\n",token.c_str(),lineNum );
            return false;
          }
        }
        else
        {
          fprintf( stderr,"error: undefined variable %s on line %d\n",token.c_str(),lineNum );
          return false;
        }
      }
    }
  }

  return true;
}

