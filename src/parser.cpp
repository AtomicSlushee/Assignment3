#include "hlsyn.h"
#include "if.h"
#include "parser.h"
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
    , types(Singleton<Types>::instance() )
    , keys( Singleton<Keywords>::instance( ))
{
}

bool Parser::process( std::ifstream& in, Statements& stmts )
{
  Tokenizer tokens(in);
  return processMore( in, stmts, tokens, Keyword::NONE );
}

bool Parser::processMore( std::ifstream& in, Statements& stmts, Tokenizer& tokens, Keyword::ID stopOn )
{
  bool addedClkRst = false;
  bool addedDummies = false;
  std::string line;

  // for all lines in the circuit file
  do
  {
    std::string token;
    while( tokens.next_thru( token ) )
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
            fprintf( stderr,"error: invalid data type %s on line %d\n",ttoken.c_str(),tokens.lineNum() );
            return false;
          }
        }
        else
        {
          fprintf( stderr,"error: missing data type for IO Class %s on line %d\n",token.c_str(),tokens.lineNum() );
          return false;
        }
        tokens.kill();
      }
      // must be a variable assignment, if, or for-loop
      else
      {
        Keyword::ID k = keys.isKeyword( token );
        if (stopOn == k)
          return true;
        if( k != Keyword::INVALID )
        {
          std::string tmp;
          std::string cond;
          switch( k )
          {
            case Keyword::IF:
              if(( tokens.next(tmp) ) &&
                 ( keys.isKeyword(tmp) == Keyword::LEFT_PAREN ) &&
                 ( tokens.next(cond)) &&
                 ( tokens.next(tmp)) &&
                 ( keys.isKeyword(tmp)==Keyword::RIGHT_PAREN) &&
                 ( tokens.next_thru(tmp) ) &&
                 ( keys.isKeyword(tmp)==Keyword::OPEN_BRACE) )
              {
                if (vars.isVariable(cond))
                {
                  IfStatement& if_ = stmts.addIfStatement(vars.getVariable(cond));
                  DEBUGOUT( "begin if statement on condition %s\n",cond.c_str());
                  if (processMore(in,if_.getIfTrue(),tokens,Keyword::CLOSE_BRACE))
                  {
                    bool good;
                    if( (good=tokens.next_thru(tmp)) &&
                        (keys.isKeyword(tmp) == Keyword::ELSE) )
                    {
                      DEBUGOUT( "begin else statement on condition %s\n",cond.c_str());
                      if( (tokens.next_thru(tmp)) &&
                          (keys.isKeyword(tmp) == Keyword::OPEN_BRACE))
                      {
                        if( processMore(in,if_.getIfFalse(),tokens,Keyword::CLOSE_BRACE))
                        {
                          DEBUGOUT( "end else statement on condition %s\n", cond.c_str());
                        }
                        else
                        {
                          fprintf(stderr, "missing closing brace on else statement for condition %s\n", cond.c_str());
                          return false;
                        }
                      }
                      else
                      {
                        fprintf(stderr, "missing open brace on if statement at line %d\n", tokens.lineNum());
                        return false;
                      }
                    }
                    else
                    {
                      if( good ) tokens.reQueue(tmp);
                    }
                    DEBUGOUT( "end if statement on condtion %s\n", cond.c_str());
                  }
                  else
                  {
                    fprintf(stderr, "missing closing brace on if statement for condition %s\n", cond.c_str());
                    return false;
                  }
                  // TODO: recurse, need to add stop condition to get back
                  // TODO: how to detect an error on the return? perhaps 'false' unexpected here?
                }
                else
                {
                  fprintf(stderr, "error: invalid if condition variable %s\n",cond.c_str());
                  return false;
                }
              }
              break;
            default:
              fprintf( stderr,"error: %s not yet implemented\n", token.c_str() );
          }
        }
        // must be a variable assignment
        else if( vars.isVariable( token ) )
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
                    fprintf( stderr,"error: unknown variable %s on line %d\n",args[2].c_str(),tokens.lineNum() );
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
                    fprintf( stderr,"error: malformed mux on line %d\n",tokens.lineNum() );
                    return false;
                  }
                }
                else
                {
                  fprintf( stderr,"error: malformed assignment on line %d\n",tokens.lineNum() );
                  return false;
                }
              }
              else
              {
                fprintf( stderr,"error: argument for assignment to %s is not a variable on line %d\n",token.c_str(),
                    tokens.lineNum() );
                return false;
              }
            }
            else
            {
              fprintf( stderr,"error: assignment to %s has no arguments on line %d\n",token.c_str(),tokens.lineNum() );
              return false;
            }
            DEBUGCOUT << std::endl;
            tokens.kill();
          }
          else
          {
            fprintf( stderr,"error: no assignment operator for variable %s on line %d\n",token.c_str(),tokens.lineNum() );
            return false;
          }
        }
        else
        {
          fprintf( stderr,"error: undefined variable %s on line %d\n",token.c_str(),tokens.lineNum() );
          return false;
        }
      }
    }
  } while( !tokens.isEOF() );

  return true;
}

