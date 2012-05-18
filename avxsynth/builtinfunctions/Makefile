include ../../config.mak

ifeq ($(SRCPATH),.)
        SRCPATH=../..
endif

SRC_PATH = $(SRCPATH)/avxsynth/builtinfunctions/src
OBJ_PATH = ./src

OBJECTS = \
	$(OBJ_PATH)/AllBuiltInPluginsEntryPoint.o \
	$(OBJ_PATH)/audio/audio.o                 \
	$(OBJ_PATH)/core/alignplanar.o            \
	$(OBJ_PATH)/filters/color.o               \
	$(OBJ_PATH)/filters/combine.o             \
	$(OBJ_PATH)/filters/convolution.o         \
	$(OBJ_PATH)/filters/edit.o                \
	$(OBJ_PATH)/filters/field.o               \
	$(OBJ_PATH)/filters/focus.o               \
	$(OBJ_PATH)/filters/fps.o                 \
	$(OBJ_PATH)/filters/greyscale.o           \
	$(OBJ_PATH)/filters/histogram.o           \
	$(OBJ_PATH)/filters/layer.o               \
	$(OBJ_PATH)/filters/levels.o              \
	$(OBJ_PATH)/filters/merge.o               \
	$(OBJ_PATH)/filters/misc.o                \
	$(OBJ_PATH)/filters/planeswap.o           \
	$(OBJ_PATH)/filters/resample.o            \
	$(OBJ_PATH)/filters/resize.o              \
	$(OBJ_PATH)/filters/transform.o           \
	$(OBJ_PATH)/filters/turnfunc.o            \
	$(OBJ_PATH)/filters/turn.o                \
	$(OBJ_PATH)/sources/source.o              \
	$(OBJ_PATH)/convert/convert.o             \
	$(OBJ_PATH)/convert/convert_rgb.o         \
	$(OBJ_PATH)/convert/convert_yuy2.o        \
	$(OBJ_PATH)/convert/convert_yv12.o

INCLUDES = \
	-I$(SRCPATH)/avxsynth/common/include         \
	-I$(SRCPATH)/avxsynth/common/include/audio   \
	-I$(SRCPATH)/avxsynth/common/include/convert \
	-I$(SRCPATH)/avxsynth/common/include/core    \
	-I$(SRCPATH)/avxsynth/common/include/distrib \
	-I$(SRCPATH)/avxsynth/common/include/filters \
	-I$(SRCPATH)/avxsynth/common/include/source  \
	-I$(SRCPATH)/include                         \
	-I$(SRC_PATH)

EXECUTABLE = libavxbtinfncs$(ARNAME)

$(EXECUTABLE): $(OBJECTS)
	$(AR) $(EXECUTABLE) $(OBJECTS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CFLAGS) $(CFLAGS_DYN) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
