include ../../common.mk

#
# Import includes
#
COMMON_INCLUDES  = \
			-I../../include 		\
			-I../common/include 		\
			-I../common/include/audio 	\
			-I../common/include/core 	\
			-I../common/include/filters	\
			-I../builtinfunctions		\
			-I../../avxcommon/include

LOCAL_INCLUDES   = -I$(SRC_PATH) -I./include
INCLUDES         =                      		\
                   $(LOCAL_INCLUDES)    		\
                   $(COMMON_INCLUDES)

#
# Sources/objects
#
SRC_PATH         = ./src
SRC_PATH_AUDIO	 = $(SRC_PATH)/audio
SRC_PATH_CORE	 = $(SRC_PATH)/core
SRC_PATH_FILTERS = $(SRC_PATH)/filters
SRC_PATH_SOURCES = $(SRC_PATH)/sources

OBJECTS          = 									\
		    $(SRC_PATH_AUDIO)/convertaudio.o					\
		    $(SRC_PATH_CORE)/avxsynth_c.o					\
		    $(SRC_PATH_CORE)/avxsynth.o						\
		    $(SRC_PATH_CORE)/cache.o						\
		    $(SRC_PATH_CORE)/main.o						\
		    $(SRC_PATH_CORE)/memcpy_amd.o					\
 		    $(SRC_PATH_CORE)/plugins.o						\
 		    $(SRC_PATH_CORE)/parser/expression.o				\
 		    $(SRC_PATH_CORE)/parser/script.o					\
		    $(SRC_PATH_CORE)/parser/scriptparser.o				\
 		    $(SRC_PATH_CORE)/parser/tokenizer.o					\
		    $(SRC_PATH_FILTERS)/conditional/conditional.o			\
		    $(SRC_PATH_FILTERS)/conditional/conditional_functions.o		\
		    $(SRC_PATH_FILTERS)/conditional/conditional_reader.o		\
		    $(SRC_PATH_FILTERS)/debug.o						\
		    $(SRC_PATH_FILTERS)/text-overlay.o					\
		    $(SRC_PATH_SOURCES)/avi/cpuaccel.o					\
		    $(SRC_PATH)/windowsPorts.o						


#
# Libraries
#

AVISYNTH_BUILT_IN_FUNCTIONS = -L$(AVXSYNTH_DEPLOY_ROOT) -lavxbtinfncs

LIBS             = -lpthread -lm -ldl $(AVXCOMMON_LIB) $(AVISYNTH_BUILT_IN_FUNCTIONS) 

#
# Outputs
#
EXECUTABLE       = libavxsynth.so

#
# Compiler
#
CFLAGS           = -pthread -Wall -Wno-unused -fPIC -g -D_REENTRANT -fpermissive
CONSTANT_STRINGS = -DAVXSYNTH_PLUGIN_RELATIVE_PATH='"$(AVXSYNTH_DEPLOY_REL_PATH)"'
COMPILE          = g++ $(CFLAGS) $(INCLUDES) $(CONSTANT_STRINGS)

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
	echo ""; echo "Building:";echo ""; make;                      		\
	echo ""; echo "Deploying:"; echo "";                          		\
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_ROOT)";		\
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);	\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_ROOT); 				\
	ls -alg $(AVXSYNTH_DEPLOY_ROOT);					\
	echo "\n\n\n\n\n"
