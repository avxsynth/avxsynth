include ../../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../../..
endif

SRC_PATH = $(SRCPATH)/apps/avxframeserver/frameserverlib/src
OBJ_PATH = ./src

OBJECTS = \
	$(OBJ_PATH)/avxSynthAppInterface.o \
	$(OBJ_PATH)/utilities.o

INCLUDES = \
	-I$(SRCPATH)/avxsynth/common/include \
	-I$(SRCPATH)/include

LIBS = 

EXECUTABLE = libavxframeserver$(ARNAME)

$(EXECUTABLE): $(OBJECTS)
	$(AR) $(EXECUTABLE) $(OBJECTS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
