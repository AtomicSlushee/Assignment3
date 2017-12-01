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
          if( parser.process( inFile,moduleVars,modelVars,program,true ) )
          {
#if 0
            //##########################################################################################
            //  DEBUG AREA WHERE KEN IS PLAYING AND TRYING THINGS OUT
            //##########################################################################################
            {
              graphType g;
              int groups = g.createWeightedGraph(program);
              graphType::vertices_t topo;
              g.topologicalSort(topo);
              double lp = g.longestPath(topo, graphType::UNITY);
              // let's try a scheduling algorithm
              scheduler.ASAP(g);
              scheduler.ALAP(g, latency);

              //gather some stats
              struct Stats
              {
                int mindist;
                int maxdist;
                int runsum;
              };
              std::map<int,Stats> stats;
              for(int g=1; g<=groups; g++)
              {
                stats[g].mindist = 99999; //min distance
                stats[g].maxdist = -1;   //max distance
              }
              for(auto v = topo.begin(); v != topo.end(); v++)
              {
                int d = static_cast<int>(0.1 + v->get().helper.dist);
                int g = v->get().helper.partition;
                if (d < stats[g].mindist) stats[g].mindist = d;
                if (d > stats[g].maxdist) stats[g].maxdist = d;
              }
              std::cout << std::endl << "PARTITION:MIN/MAX --> ";
              for(int g=1; g<=groups; g++)
              {
                std::cout << " " << g << ":" << stats[g].mindist << "/" << stats[g].maxdist;
                if( g == 1)
                  stats[g].runsum = 0;
                else
                  stats[g].runsum = stats[g-1].runsum + stats[g-1].maxdist - stats[g-1].mindist;
              }
              std::cout << std::endl;


              std::cout << std::endl << "TOPO" << std::endl;
              std::cout << "LP: " << lp << std::endl;
              for(auto v = topo.begin(); v != topo.end(); v++)
              {
                int g = v->get().helper.partition;
                int d = static_cast<int>(0.1 + v->get().helper.dist);
                int myState = stats[g].runsum + d - stats[g].mindist + g;
                std::cout << "<" << myState << ">" << v->get().getNodeNumber() << "[" << v->get().helper.dist << "]{" << v->get().helper.partition << "}(" << v->get().helper.schedTime[graphType::ASAP] << "): " << v->get().getNode().get().C_format() << std::endl;
              }
              std::cout << std::endl << std::endl;


              // I want to be able to copy graphs, or lists of vertices at least
              graphType g2(g);
              graphType::vertices_t v2;
              v2 = topo;
              graphType g3(topo);
              graphType g4 = g;

              // can I find a statement in a graph???
              for( auto s = program.begin(); s != program.end(); s++)
              {
                auto i = g.findStatement(*s);
                std::string c = i->get().getNode().get().C_format();
                int x=4;x++;
              }

              int x=4;x++;
            }
            //##########################################################################################
#endif
            if( scheduler.process( program, schedule, latency, modelVars ) )
            {
              if( verilog.HLSM( outFile, "", moduleVars, modelVars, schedule ))
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

