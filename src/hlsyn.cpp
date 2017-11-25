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
  ModuleVariables vars;
  ModelVariables mvars;
  Parser& parser = Singleton< Parser >::instance();
  HLSM& hlsmTools = Singleton< HLSM >::instance();
  Scheduler& scheduler = Singleton< Scheduler >::instance();
  Verilog& verilog = Singleton< Verilog >::instance();
  Statements program;
  Statements hlsm;
  Statements schedule;

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
          if( parser.process( inFile,vars,mvars,program,true ) )
          {
            //##########################################################################################
            //  DEBUG AREA WHERE KEN IS PLAYING AND TRYING THINGS OUT
            //##########################################################################################
            {
              graphType g;
              g.createWeightedGraph(program);
              graphType::vertices_t topo;
              g.topologicalSort(topo);
              std::cout << std::endl << std::endl << "TOPO" << std::endl;
              std::cout << "LP: " << g.longestPath(topo, graphType::UNITY) << std::endl;
              for(auto v = topo.begin(); v != topo.end(); v++)
              {
                std::cout << v->get().getNodeNumber() << "[" << v->get().helper.dist << "]: " << v->get().getNode().get().C_format() << std::endl;
              }
              std::cout << std::endl << std::endl;
              int x=4;x++;
            }
            //##########################################################################################

            if( hlsmTools.CtoHLSM( program, hlsm ) )
            {
              scheduler.process(hlsm,schedule); // TODO: scheduling, etc.

              if( verilog.HLSM( outFile, "", vars, mvars, program/*schedule*/ ))
              {
                DEBUGOUT( "converted %s to %s with latency %g\n",argv[1],argv[3],latency );
              }
              else
              {
                fprintf( stderr,"error while converting to Verilog\n" );
              }
            }
            else
            {
              fprintf( stderr,"error while converting program to HLSM\n" );
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

