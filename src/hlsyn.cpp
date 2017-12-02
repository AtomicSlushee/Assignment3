#include "hlsyn.h"
#include "parser.h"
#include "verilog.h"
#include "scheduler.h"
#include "graphType.hpp"
#include "hlsm.h"
#include <cstdio>
#include <string>

#if !DEBUG_ENABLED
std::ostream bitBucket(0);
#endif

int main( int argc, char* argv[] )
{
  ModuleVariables moduleVars;
  ModelVariables modelVars;
  Parser& parser = Singleton< Parser >::instance();
  Scheduler& scheduler = Singleton< Scheduler >::instance();
  Verilog& verilog = Singleton< Verilog >::instance();
  Statements program;
  graphType schedule;

  if( argc > 3 )
  {
    graphType::ScheduleID sid = graphType::FDS;
    if( argc > 4 )
      sid = static_cast< graphType::ScheduleID >( std::stoi( argv[4] ) );
    std::ifstream inFile( argv[1],std::ifstream::in );
    if( inFile.good() )
    {
      double latency = std::stod( argv[2] );
      if( latency >= 0.0 )
      {
        std::ofstream outFile( argv[3],std::ofstream::out );
        if( outFile.good() )
        {
          if( parser.process( inFile,moduleVars,modelVars,program,true ) )
          {
            if( scheduler.process( program, schedule, latency, modelVars, sid ) )
            {
              if( verilog.HLSM( outFile, "", moduleVars, modelVars, schedule, sid ))
              {
                DEBUGOUT( "converted %s\nto %s\nwith latency %g and\nscheduling algorithm %s\n",argv[1],argv[3],latency,schedule.nameScheduleID(sid).c_str() );
              }
              else
              {
                fprintf( stderr,"error while converting to Verilog\n" );
              }
            }
            else
            {
              fprintf( stderr,"error while scheduling program to HLSM\n" );
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

