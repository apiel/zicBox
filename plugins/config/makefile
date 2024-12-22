MAKEFLAGS += -j$(shell nproc)

ifneq ($(shell uname -m),x86_64)
SUB_DIR := arm
else
SUB_DIR := x86
endif
BUILD_DIR ?= build/$(SUB_DIR)
OBJ_DIR ?= $(BUILD_DIR)/obj

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
endif

INC=-I../..

all:
	make LuaConfig JsConfig DustConfig

# if liblua is not installed, it should skip
LuaConfig:
	pkg-config --cflags --libs lua && make compile LIBNAME=LuaConfig EXTRA="$(shell pkg-config --cflags --libs lua)" || true

JsConfig:
	pkg-config --cflags --libs duktape && make compile LIBNAME=JsConfig EXTRA="$(shell pkg-config --cflags --libs duktape)" || true

DustConfig:
	make compile LIBNAME=DustConfig

%Config:
	make compile LIBNAME=$@

compile:
	@mkdir -p $(OBJ_DIR)
	g++ -g -c -o $(OBJ_DIR)/$(LIBNAME).o $(LIBNAME).cpp -fPIC $(INC) $(EXTRA) $(RPI)
	g++ -g -shared -o $(BUILD_DIR)/libzic_$(LIBNAME).so $(OBJ_DIR)/$(LIBNAME).o $(INC) $(EXTRA) $(RPI)

clean:
	rm -rf $(BUILD_DIR)
