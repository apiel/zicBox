BUILD_DIR ?= build
OBJ_DIR ?= $(BUILD_DIR)/obj

ifneq ($(shell uname -m),x86_64)
RPI := -DIS_RPI=1
endif

all:
	make DustConfig LuaConfig

# if liblua is not installed, it should skip
LuaConfig:
	pkg-config --cflags --libs lua && make compile LIBNAME=LuaConfig EXTRA="$(shell pkg-config --cflags --libs lua)" || true

%Config:
	make compile LIBNAME=$@

compile:
	@mkdir -p $(OBJ_DIR)
	g++ -g -c -o $(OBJ_DIR)/$(LIBNAME).o $(LIBNAME).cpp -fPIC $(EXTRA) $(RPI)
	g++ -g -shared -o $(BUILD_DIR)/libzic_$(LIBNAME).so $(OBJ_DIR)/$(LIBNAME).o $(EXTRA) $(RPI)

clean:
	rm -rf $(BUILD_DIR)
