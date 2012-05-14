include ../../../common.mk

#
# Import includes
#
COMMON_INCLUDES  = -I../../../include			\
		   -I../../../avxsynth/common/include

FRAMES_SERVER_INCLUDES  = -I../frameserverlib/src			\

#
# Sources/objects
#
SRC_PATH         = ./src
OBJECTS          = 						\
                    $(SRC_PATH)/main.o

#
# Libraries
#
SYSLIBRARIES     = 			\
		   -lpthread		\
		   -lm			\
		   -ldl

FRAMESERVER_LIB                   = -L../frameserverlib -lavxframeserver

AVXCOMMON_LIB                   = -L../../../avxcommon -lavxcommon 

LIBS             = -Wl $(AVXCOMMON_LIB) $(FRAMESERVER_LIB) $(SYSLIBRARIES) 

SYSLDFLAGS       = -g

#
# Outputs
#
EXECUTABLE       = avxSynthFrameServer

#
# Compiler
#
INCLUDES         = $(COMMON_INCLUDES) $(FRAMES_SERVER_INCLUDES)
CFLAGS           = -Wall -g 
COMPILE          = g++ $(CFLAGS) $(INCLUDES)

#
# Linker
#
RUNTIME_LIB_PATH = -Wl,-R$(AVXSYNTH_DEPLOY_PLUGINS) -Wl,-R$(AVXSYNTH_DEPLOY_ROOT) -Wl,-R/usr/local/lib/
LINK             = g++ $(RUNTIME_LIB_PATH) $(LIBS)  

#
# Build procedures/target descriptions
#

default: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK) $(OBJECTS) $(LIBS) -o $(EXECUTABLE)

%.o: %.cpp
	$(COMPILE) -c $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)

build:
	echo ""; echo "Building:";echo ""; make;                      \

rebuild:
	echo ""; echo "Building:";echo ""; make;                      \
	
deploy:
	echo ""; echo "Building:";echo ""; make;                      \
	test -d $(AVXSYNTH_BIN_PATH) || mkdir -p $(AVXSYNTH_BIN_PATH); \
	$(MPLDEPLOY) $(EXECUTABLE) $(AVXSYNTH_BIN_PATH)               \
	
run:
	echo ""; echo "Building:";echo ""; make;                      \
	echo ""; echo "Running :";echo ""; ./$(EXECUTABLE)         

