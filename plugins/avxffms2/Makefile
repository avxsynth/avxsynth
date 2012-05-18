include ../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../..
endif

SRC_PATH = $(SRCPATH)/plugins/avxffms2/src
OBJ_PATH = ./src

OBJECTS = \
	$(OBJ_PATH)/avsutils_avx.o   \
	$(OBJ_PATH)/ffswscale_avx.o  \
	$(OBJ_PATH)/avssources_avx.o \
	$(OBJ_PATH)/ffpp_avx.o       \
	$(OBJ_PATH)/avxffms2.o

INCLUDES = \
	-I$(SRCPATH)/include          \
	$(FFMS2_INCLUDES)

LIBS = $(FFMS2_LIBS)

DEFINES = -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64

EXECUTABLE = libavxffms2$(SONAME)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS) $(LDFLAGS_DYN)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_DYN) $(INCLUDES) $(DEFINES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	install -T -D $(EXECUTABLE) $(pluginsdir)/$(EXECUTABLE)
