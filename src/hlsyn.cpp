#include "hlsyn.h"
#include "parser.h"
#include "verilog.h"
#include <cstdio>
#include <string>

#if !DEBUG_ENABLED
std::ostream bitBucket(0);
#endif

int main( int argc, char* argv[] )
{
  Parser& parser = Singleton< Parser >::instance();
  Verilog& verilog = Singleton< Verilog >::instance();
  Variables& vars = Singleton< Variables >::instance();
  Statements program;

  if( argc > 3 )
  {
    std::ifstream inFile( argv[1],std::ifstream::in );
    if( inFile.good() )
    {
      double latency = std::stod( argv[2] );
      if( latency >= 0.0 )
      {
        std::ofstream outFile( argv[3],std::ofstream::out );
        if( outFile.good() )
        {
          if( parser.process( inFile,program ) )
          {
            if( verilog.process( outFile, "", vars, program ))
            {
              DEBUGOUT( "converted %s to %s with latency %g\n",argv[1],argv[3],latency );
            }
            else
            {
              fprintf(stderr, "error while converting to Verilog\n" );
            }
          }
          else
          {
            fprintf( stderr,"error while processing netListFile\n" );
          }
          outFile.close();
        }
        else
        {
          fprintf( stderr,"error: invalid verilog filename\n" );
        }
      }
      else
      {
        fprintf( stderr,"error: invalide latency value\n" );
      }
      inFile.close();
    }
    else
    {
      fprintf( stderr,"error: invalid net list filename\n" );
    }
  }
  else
  {
    fprintf( stdout,"usage: hlsyn <netListFile> <latency> <verilogFile>\n" );
  }

  return 0;
}

