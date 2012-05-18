include ../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=..
endif

SRC_PATH = $(SRCPATH)/avxutils/src
OBJ_PATH = ./src

OBJECTS = \
	$(OBJ_PATH)/AvxTextRender.o \
	$(OBJ_PATH)/Path.o          \
	$(OBJ_PATH)/avxlog.o

INCLUDES = \
	-I$(SRCPATH)/include/utils \
	-I$(SRCPATH)/include           \
	$(PANGOCAIRO_INCLUDES)         \
        $(LOG4CPP_INCLUDES)

LIBS = $(PANGOCAIRO_LIBS) $(LOG4CPP_LIBS)

EXECUTABLE = libavxutils$(SONAME)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS) $(LDFLAGS_DYN)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_DYN) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	install -T -D $(EXECUTABLE) $(libdir)/$(EXECUTABLE)
