include ../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../..
endif

SRC_PATH = $(SRCPATH)/plugins/autocrop/src
OBJ_PATH = ./src

OBJECTS = $(OBJ_PATH)/autocrop.o

INCLUDES = \
	-I$(SRCPATH)/include

EXECUTABLE = libautocrop$(SONAME)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS) $(LDFLAGS_DYN)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_DYN) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	install -T -D $(EXECUTABLE) $(DESTDIR)$(pluginsdir)/$(EXECUTABLE)
