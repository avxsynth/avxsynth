include ../../common.mk

SRC_PATH         = ./src

#
# Import includes
#
PLUGIN_INCLUDES  = -I../../include		\
		   -I../../avxsynth/include

LOCAL_INCLUDES   = -I$(SRC_PATH) \
			
INCLUDES         =                      \
                   $(LOCAL_INCLUDES)    \
                   $(PLUGIN_INCLUDES)   
			

#
# Sources/objects
#
OBJECTS          = 							\
		    $(SRC_PATH)/autocrop.o				

#
# Libraries
#


SYSTEM_LIBS      = -lpthread    \
                   -lm          \
                   -ldl          

LIBS             = $(SYSTEM_LIBS) $(AVXCOMMON_LIB)

#
# Outputs
#
EXECUTABLE       = libautocrop.so

#
# Compiler
#
CFLAGS           = -pthread -Wall -Wno-unused -fPIC -g -D_REENTRANT 
COMPILE          = g++ $(CFLAGS) $(INCLUDES) 

#
# Linker
#
LDFLAGS          = -g -Wl,-Bdynamic -shared
LINK             = g++ $(LDFLAGS)

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

deploy:
	echo ""; echo "Building:";echo ""; make;                      \
	echo ""; echo "Deploying:"; echo "";                          \
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_PLUGINS)";   \
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);		\
	test -d $(AVXSYNTH_DEPLOY_PLUGINS) || mkdir -p $(AVXSYNTH_DEPLOY_PLUGINS);	\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_PLUGINS); 		      \
	ls -alg $(AVXSYNTH_DEPLOY_PLUGINS);			      \
	echo "\n\n\n\n\n"