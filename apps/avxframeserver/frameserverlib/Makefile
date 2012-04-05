include ../../../common.mk

#
# Import includes
#
COMMON_INCLUDES  = -I../../../include			\
		   -I../../../avxsynth/common/include

#
# Sources/objects
#
SRC_PATH         = ./src
OBJECTS          = 						\
                    $(SRC_PATH)/avxSynthAppInterface.o				\
		    $(SRC_PATH)/utilities.o


SYSLDFLAGS       = -g

#
# Outputs
#
EXECUTABLE       = libavxframeserver.a

#
# Compiler
#
INCLUDES         = $(COMMON_INCLUDES)
CFLAGS           = -Wall -Wno-unused -g
COMPILE          = g++ $(CFLAGS) $(INCLUDES)

#
# Linker
#
LDFLAGS          = 
LINK             = ar rcs $(EXECUTABLE)

#	
# Build procedures/target descriptions
#

default: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK) $(OBJECTS) 

%.o: %.cpp
	$(COMPILE) -c $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)

rebuild:
	echo ""; echo "Building:";echo ""; make; 

