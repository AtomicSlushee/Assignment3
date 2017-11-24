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
  Variable clock( std::string( "Clk" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "input" ) );
  Variable reset( std::string( "Rst" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "input" ) );
  Variable start( std::string( "Start" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "input" ) );
  Variable done(  std::string( "Done" ),
                  Singleton<Types>::instance().getType( "UInt1" ),
                  Singleton<IOClasses>::instance().getIOClass( "output reg" ) );
  Variable dummy1( std::string( "__na1" ),
                   Singleton<Types>::instance().getType( "UInt1" ),
                   Singleton<IOClasses>::instance().getIOClass( "wire" ) );
  Variable dummy2( std::string( "__na2" ),
                   Singleton<Types>::instance().getType( "UInt1" ),
                   Singleton<IOClasses>::instance().getIOClass( "wire" ) );
}

Parser::Parser()
    : ios( Singleton<IOClasses>::instance() )
    , ops( Singleton<Operators>::instance() )
    , types(Singleton<Types>::instance() )
    , keys( Singleton<Keywords>::instance( ))
    , addedClkRst( false )
    , addedDummies( false )
    , addedStartDone( false )
{
}

bool Parser::process( std::ifstream& in, ModuleVariables& vars, ModelVariables& mvars, Statements& stmts, bool hlsm )
{
  Tokenizer tokens(in);
  if( hlsm )
  {
    addClkReset(vars);
    addStartDone(vars);
    return processMore( mvars, stmts, tokens, Keyword::NONE );
  }
  return processMore( vars, stmts, tokens, Keyword::NONE );
}

bool Parser::processMore( Variables& vars, Statements& stmts, Tokenizer& tokens, Keyword::ID stopOn )
{
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
              if( vars.isVariable(vtoken))
              {
                fprintf(stderr, "error: duplicate variable name %s at line %d\n", vtoken.c_str(), tokens.lineNum());
                return false;
              }
              else
              {
                vars.addVariable( vtoken,type,ioclass );
                DEBUGCOUT<< "added variable " << vtoken << " of type " << ttoken << " with IO class " << token
                << std::endl;
              }
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
        // check for goofball condition where keyword includes a left parenthesis, and fix it
        size_t ppos=token.find(delimiters::leftparen);
        if (std::string::npos != ppos)
        {
          tokens.reQueue(token.substr(ppos));
          token.resize(ppos);
        }

        // now test for keywords
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
                  DEBUGOUT( "begin if-statement on condition %s\n",cond.c_str());
                  if (processMore(vars, if_.getIfTrue(),tokens,Keyword::CLOSE_BRACE))
                  {
                    bool good;
                    if( (good=tokens.next_thru(tmp)) &&
                        (keys.isKeyword(tmp) == Keyword::ELSE) )
                    {
                      DEBUGOUT( "begin else statement on condition %s\n",cond.c_str());
                      if( (tokens.next_thru(tmp)) &&
                          (keys.isKeyword(tmp) == Keyword::OPEN_BRACE))
                      {
                        if( processMore(vars, if_.getIfFalse(),tokens,Keyword::CLOSE_BRACE))
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
                        fprintf(stderr, "missing open brace on if-statement at line %d\n", tokens.lineNum());
                        return false;
                      }
                    }
                    else
                    {
                      if( good ) tokens.reQueue(tmp);
                    }
                    DEBUGOUT( "end if-statement on condtion %s\n", cond.c_str());
                  }
                  else
                  {
                    fprintf(stderr, "missing closing brace on if-statement for condition %s\n", cond.c_str());
                    return false;
                  }
                }
                else
                {
                  fprintf(stderr, "error: invalid if condition variable %s\n",cond.c_str());
                  return false;
                }
              }
              break;
            case Keyword::FOR:
              DEBUGOUT( "starting for-loop at line %d\n",tokens.lineNum());
              if(( tokens.next(tmp) ) &&
                 ( keys.isKeyword(tmp) == Keyword::LEFT_PAREN ))
              {
                ForLoop& for_ = stmts.addForLoop();
                if( tokens.next(tmp) &&
                    processAssignment(vars, for_.getInitial(),tokens,tmp, delimiters::semicolon))
                {
                  DEBUGOUT( "added initial expression\n");
                  if( tokens.next(tmp) &&
                      (keys.isKeyword(tmp)==Keyword::SEMICOLON))
                  {
                    std::string left;
                    std::string logic;
                    std::string right;
                    if( tokens.next(left) && tokens.next(logic) && tokens.next_special(right,delimiters::semicolon) &&
                        vars.isVariable(left) && vars.isVariable(right) && ops.isOperator(logic))
                    {
                      Operator& cmp = ops.getOperator(logic);
                      if( (cmp.id() == Operator::LT) ||
                          (cmp.id() == Operator::GT) ||
                          (cmp.id() == Operator::EQ) )
                      {
                        DEBUGOUT( "adding condition %s %s %s\n",left.c_str(),logic.c_str(),right.c_str());
                        for_.getCondition().addCondition(vars.getVariable(left),ops.getOperator(logic),vars.getVariable(right));
                        if( tokens.next(tmp) &&
                            (keys.isKeyword(tmp)==Keyword::SEMICOLON))
                        {
                          if( tokens.next(tmp) &&
                              processAssignment(vars, for_.getUpdate(),tokens,tmp,delimiters::rightparen))
                          {
                            DEBUGOUT( "added update expression\n");
                            if( tokens.next(tmp) && (keys.isKeyword(tmp)==Keyword::RIGHT_PAREN))
                            {
                              if( tokens.next(tmp) && (keys.isKeyword(tmp)==Keyword::OPEN_BRACE))
                              {
                                DEBUGOUT( "begin for-loop body at line %d\n", tokens.lineNum());
                                if( processMore(vars, for_.getBody(),tokens,Keyword::CLOSE_BRACE))
                                {
                                  DEBUGOUT( "end for-loop at line %d\n", tokens.lineNum());
                                }
                              }
                              else
                              {
                                fprintf( stderr,"error: missing left brace in for-loop body at line %d\n",tokens.lineNum());
                                return false;
                              }
                            }
                            else
                            {
                              fprintf( stderr,"error: missing right parenthesis in for-loop at line %d\n",tokens.lineNum());
                              return false;
                            }
                          }
                          else
                          {
                            fprintf( stderr,"error: invalid for-loop update expression at line %d\n",tokens.lineNum());
                            return false;
                          }
                        }
                        else
                        {
                          fprintf( stderr,"error: missing semicolon in for-loop at line %d\n",tokens.lineNum());
                          return false;
                        }
                      }
                      else
                      {
                        fprintf( stderr,"error: invalid condition comparator in for-loop at line %d\n",tokens.lineNum());
                        return false;
                      }
                    }
                    else
                    {
                      fprintf( stderr,"error: malformed condition in for-loop at line %d\n",tokens.lineNum());
                      return false;
                    }
                  }
                  else
                  {
                    fprintf( stderr,"error: missing semicolon in for-loop at line %d\n",tokens.lineNum());
                    return false;
                  }
                }
                else
                {
                  fprintf( stderr,"error: invalid for-loop initial expression at line %d\n",tokens.lineNum());
                  return false;
                }
              }
              else
              {
                fprintf( stderr,"error: missing left parenthesis in for-loop at line %d\n",tokens.lineNum());
                return false;
              }
              break;
            default:
              fprintf( stderr,"error: %s not yet implemented\n", token.c_str() );
          }
        }
        // must be a variable assignment
        else
        {
          if (!processAssignment( vars, stmts, tokens, token ))
            return false;
        }
      }
    }
  } while( !tokens.isEOF() );

  return true;
}

bool Parser::processAssignment( Variables& vars, Statements& stmts, Tokenizer& tokens, std::string token, std::string stopOn )
{
  if( vars.isVariable( token ) )
  {
    Variable& result = vars.getVariable( token );
    std::string etoken;
    if( tokens.next( etoken ) && ops.isAssignment( etoken ) )
    {
      DEBUGCOUT<< "assigning " << token << " equals ";
      std::vector< std::string > args;
      while( tokens.next_special( etoken, stopOn ) )
      {
        args.push_back( etoken );
        if( tokens.hitSpecial())
          break;
      }
      if( args.size() )
      {
        if( vars.isVariable( args[0] ) )
        {
          Variable& input1 = vars.getVariable( args[0] );
          if( args.size() == 1 )
          {
            DEBUGCOUT << "variable " << args[0] << " with REG";
            addClkReset(vars);
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
                addDummies(vars);
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
      if( stopOn == delimiters::none )
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

  return true;
}

void Parser::addClkReset(Variables& v)
{
  if( !addedClkRst )
  {
    v.addVariable( builtIn::clock );
    v.addVariable( builtIn::reset );
    addedClkRst = true;
  }
}

void Parser::addDummies(Variables& v)
{
  if( !addedDummies )
  {
    v.addVariable( builtIn::dummy1 );
    v.addVariable( builtIn::dummy2 );
    addedDummies = true;
  }
}

void Parser::addStartDone(Variables& v)
{
  if( !addedStartDone )
  {
    v.addVariable( builtIn::start );
    v.addVariable( builtIn::done );
    addedStartDone = true;
  }
}
