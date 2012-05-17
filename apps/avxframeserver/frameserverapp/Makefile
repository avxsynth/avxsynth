include ../../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../../..
endif

SRC_PATH = $(SRCPATH)/apps/avxframeserver/frameserverapp/src
OBJ_PATH = ./src

OBJECTS = $(OBJ_PATH)/main.o

INCLUDES = \
	-I$(SRCPATH)/apps/avxframeserver/frameserverlib/src \
	-I$(SRCPATH)/avxsynth/common/include                \
	-I$(SRCPATH)/include

LIBS = -L../frameserverlib -lavxframeserver -L../../../avxutils -lavxutils

EXECUTABLE = avxFrameServer$(EXE)

RUNTIME = -Wl,-rpath='$(pluginsdir)':'$(libdir)'

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS) $(RUNTIME)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	install -D $(EXECUTABLE) $(bindir)/$(EXECUTABLE)
