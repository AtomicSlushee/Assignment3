# Authors: Ken Steele, Andrew Byron and Eric Lichtenstein
# create a new executable for Assignment 3

FILE( GLOB SRCS *.cpp *.h )
add_executable( hlsyn ${SRCS} )

