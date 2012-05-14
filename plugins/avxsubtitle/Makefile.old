include ../../common.mk

SRC_PATH         = ./src

#
# Import includes
#
PLUGIN_INCLUDES  = -I../../include 

AVXCOMMON_INCLUDES = -I../../avxcommon/include


# For libpangocairo, you may obtain these by typing the following
# command in console:
# 	pkg-config --cflags --libs pangocairo)
#
ICONV_INCLUDES	     = -I/usr/include

GLIB_INCLUDES        =  `pkg-config --cflags glib-2.0`

LOCAL_INCLUDES   = -I$(SRC_PATH) \
			
INCLUDES         =                      \
                   $(LOCAL_INCLUDES)    \
		   $(GLIB_INCLUDES)                     \
                   $(PLUGIN_INCLUDES)   \
		   $(AVXCOMMON_INCLUDES) \
		   $(ICONV_INCLUDES)
			

#
# Sources/objects
#
OBJECTS          = 							\
    $(SRC_PATH)/AvxSubtitle.o				\
    $(SRC_PATH)/CaptionEntry.o				\
    $(SRC_PATH)/CaptionList.o				\
    $(SRC_PATH)/SrtParser.o				\
    $(SRC_PATH)/SubtitleParser.o			\
    $(SRC_PATH)/Utils.o					\

#
# Libraries
#


SYSTEM_LIBS      = -lpthread    \
                   -lm          \
                   -ldl          

LIBS             = $(SYSTEM_LIBS)  $(AVXCOMMON_LIB) 


#
# Outputs
#
EXECUTABLE       = libavxsubtitle.so

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
	echo ""; echo "Building:";echo ""; make;                      			\
	echo ""; echo "Deploying:"; echo "";                          			\
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_PLUGINS)";   			\
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);		\
	test -d $(AVXSYNTH_DEPLOY_PLUGINS) || mkdir -p $(AVXSYNTH_DEPLOY_PLUGINS);	\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_PLUGINS); 		      			\
	ls -alg $(AVXSYNTH_DEPLOY_PLUGINS);						\
	echo "\n\n\n\n\n"