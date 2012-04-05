include ../common.mk

SRC_PATH         = ./src

#
# Import includes
#


# For libpangocairo, you may obtain these by typing the following
# command in console:
# 	pkg-config --cflags --libs pangocairo)
#
ICONV_INCLUDES	     = -I/usr/include
LOG4CPP_INCLUDES     = `log4cpp-config --cflags`
GLIB_INCLUDES        =  `pkg-config --cflags glib-2.0`
PANGO_CAIRO_INCLUDES = \
			-I/usr/include/pango-1.0	\
			-I/usr/include/cairo		\
			-I/usr/include/pixman-1		\
			-I/usr/include/freetype2	\
			-I/usr/include/libpng12  

LOCAL_INCLUDES       = -Iinclude			\
		       -I../include
			
INCLUDES             =                      		\
			$(LOCAL_INCLUDES)    		\
			$(GLIB_INCLUDES)                \
			$(PANGO_CAIRO_INCLUDES) 	\
			$(ICONV_INCLUDES)		\
			$(LOG4CPP_INCLUDES)	

#
# Sources/objects
#
OBJECTS          = 			\
		  $(SRC_PATH)/AvxTextRender.o	\
		  $(SRC_PATH)/Path.o		\
		  $(SRC_PATH)/avxlog.o		\



#
# Libraries
#
PANGO_CAIRO_LIBS = 			\
		   -lpangocairo-1.0 	\
		   -lpango-1.0		\
		   -lcairo		\
		   -lgobject-2.0 	\
		   -lgmodule-2.0 	\
		   -lgthread-2.0 	\
		   -lrt			\
		   -lglib-2.0


LIBS             = -lpthread -lm -ldl $(PANGO_CAIRO_LIBS) $(LOG4CPP_LIBS)

#
# Outputs
#
EXECUTABLE       = libavxcommon.so

#
# Compiler
#
CFLAGS           = -pthread -Wall -Wno-unused -g -fPIC 
COMPILE          = g++ $(CFLAGS) $(INCLUDES) 

#
# Linker
#
LDFLAGS          = -g -Wl,-Bdynamic -shared
LINK             = g++ $(LDFLAGS) $(LIBS)

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
	echo "copying $(EXECUTABLE) to $(AVXSYNTH_DEPLOY_ROOT)";          		\
	test -d $(AVXSYNTH_DEPLOY_ROOT) || mkdir -p $(AVXSYNTH_DEPLOY_ROOT);		\
	cp $(EXECUTABLE) $(AVXSYNTH_DEPLOY_ROOT); 			      		\
	ls -alg $(AVXSYNTH_DEPLOY_ROOT);						\
	echo "\n\n\n\n\n"                			\
