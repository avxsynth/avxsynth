include ../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../..
endif

SRC_PATH = $(SRCPATH)/apps/AVXEdit/src
OBJ_PATH = ./src

OBJECTS = \
	$(OBJ_PATH)/main.o            \
	$(OBJ_PATH)/mainwindow.o      \
	$(OBJ_PATH)/moc_mainwindow.o  \
	$(OBJ_PATH)/qrc_application.o

INCLUDES = \
	-I$(SRCPATH)/apps/avxframeserver/frameserverlib/src \
	-I$(SRCPATH)/avxsynth/common/include                \
	-I$(SRCPATH)/include                                \
	$(QTGUI_INCLUDES)

LIBS = -L../avxframeserver/frameserverlib -lavxframeserver -L../../avxutils -lavxutils $(QTGUI_LIBS)

EXECUTABLE = AVXEdit$(EXE)

RUNTIME = -Wl,-rpath='$(pluginsdir)':'$(libdir)'

MOC = moc-qt4
RCC = rcc

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(EXECUTABLE) $(OBJECTS) $(LIBS) $(LDFLAGS) $(RUNTIME)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_PATH)/moc_mainwindow.o: $(SRC_PATH)/mainwindow.h
	$(MOC) $(INCLUDES) $(DEFINES) $< -o $(OBJ_PATH)/moc_mainwindow.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $(OBJ_PATH)/moc_mainwindow.cpp -o $@

$(OBJ_PATH)/qrc_application.o: $(SRC_PATH)/../application.qrc
	$(RCC) -name application $< -o $(OBJ_PATH)/qrc_application.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c $(OBJ_PATH)/qrc_application.cpp -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	install -D $(EXECUTABLE) $(bindir)/$(EXECUTABLE)
