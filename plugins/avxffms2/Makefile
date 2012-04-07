include ../../common.mk

SRC_PATH         = ./src

#
# Import includes
#
PLUGIN_INCLUDES  = -I../../include \
	-I$(AVXSYNTH_INCLUDE_PATH)

			
LOCAL_INCLUDES   = -I$(SRC_PATH) \
			
INCLUDES         =                      \
                   $(LOCAL_INCLUDES)    \
                   $(PLUGIN_INCLUDES)   
			

#
# Sources/objects
#
OBJECTS          = 							\
		    $(SRC_PATH)/avsutils_avx.o				\
		    $(SRC_PATH)/ffswscale_avx.o				\
		    $(SRC_PATH)/avssources_avx.o			\
		    $(SRC_PATH)/ffpp_avx.o				\
		    $(SRC_PATH)/avxffms2.o				\

#
# Libraries
#
LIBAV_LIBS = 			\
		   -lavutil 	\
		   -lavformat 	\
		   -lswscale 	\
		   -lpostproc 	\
		   -lavcodec 	\
		   -lffms2


SYSTEM_LIBS      = -lpthread    \
                   -lm          \
                   -ldl          

LIBS             = $(SYSTEM_LIBS) $(LIBAV_LIBS) $(AVXCOMMON_LIB)

#
# Outputs
#
EXECUTABLE       = libavxffms2.so

#
# Compiler
#
CFLAGS           = -pthread -Wall -Wno-unused -fPIC -g -D_REENTRANT -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64
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
	echo ""; echo "Building:";echo ""; make;                      			\
	echo ""; echo "Deploying:"; echo "";                          			\
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_PLUGINS)";          		\
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);		\
	test -d $(AVXSYNTH_DEPLOY_PLUGINS) || mkdir -p $(AVXSYNTH_DEPLOY_PLUGINS);	\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_PLUGINS); 			      		\
	ls -alg $(AVXSYNTH_DEPLOY_PLUGINS);						\
	echo "\n\n\n\n\n"
