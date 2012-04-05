include ../../common.mk

#
# Import includes
#
COMMON_INCLUDES  = \
			-I../../include 		\
			-I../common/include		\
			-I../common/include/audio 	\
			-I../common/include/distrib 	\
			-I../common/include/convert	\
			-I../common/include/core 	\
			-I../common/include/filters 	\
			-I../common/include/source

LOCAL_INCLUDES   = -I$(SRC_PATH)
INCLUDES         =                      \
                   $(LOCAL_INCLUDES)    \
                   $(COMMON_INCLUDES)

#
# Sources/objects
#
SRC_PATH         = ./src
SRC_PATH_AUDIO	 = $(SRC_PATH)/audio
SRC_PATH_CORE	 = $(SRC_PATH)/core
SRC_PATH_FILTERS = $(SRC_PATH)/filters
SRC_PATH_SOURCES = $(SRC_PATH)/sources
SRC_PATH_CONVERT = $(SRC_PATH)/convert

OBJECTS          = 									\
		    $(SRC_PATH)/AllBuiltInPluginsEntryPoint.o				\
		    $(SRC_PATH_AUDIO)/audio.o						\
		    $(SRC_PATH_CORE)/alignplanar.o					\
		    $(SRC_PATH_FILTERS)/color.o						\
		    $(SRC_PATH_FILTERS)/combine.o					\
		    $(SRC_PATH_FILTERS)/convolution.o					\
		    $(SRC_PATH_FILTERS)/edit.o						\
		    $(SRC_PATH_FILTERS)/field.o						\
		    $(SRC_PATH_FILTERS)/focus.o						\
		    $(SRC_PATH_FILTERS)/fps.o						\
		    $(SRC_PATH_FILTERS)/greyscale.o					\
		    $(SRC_PATH_FILTERS)/histogram.o					\
		    $(SRC_PATH_FILTERS)/layer.o						\
		    $(SRC_PATH_FILTERS)/levels.o					\
		    $(SRC_PATH_FILTERS)/merge.o						\
		    $(SRC_PATH_FILTERS)/misc.o						\
		    $(SRC_PATH_FILTERS)/planeswap.o					\
		    $(SRC_PATH_FILTERS)/resample.o					\
		    $(SRC_PATH_FILTERS)/resize.o					\
		    $(SRC_PATH_FILTERS)/transform.o					\
		    $(SRC_PATH_FILTERS)/turnfunc.o					\
		    $(SRC_PATH_FILTERS)/turn.o						\
		    $(SRC_PATH_SOURCES)/source.o					\
		    $(SRC_PATH_CONVERT)/convert.o					\
		    $(SRC_PATH_CONVERT)/convert_rgb.o					\
		    $(SRC_PATH_CONVERT)/convert_yuy2.o					\
		    $(SRC_PATH_CONVERT)/convert_yv12.o

#
# Libraries
#
LIBS             = -lpthread -lm -ldl $(AVXCOMMON_LIB)

#
# Outputs
#
EXECUTABLE       = libavxbtinfncs.so

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
	echo ""; echo "Building:";echo ""; make;                      		\
	echo ""; echo "Deploying:"; echo "";                          		\
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_ROOT)";      		\
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);	\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_ROOT); 		      		\
	ls -alg $(AVXSYNTH_DEPLOY_ROOT);					\
	echo "\n\n\n\n\n"