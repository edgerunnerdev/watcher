TEMP=temp/linux
WATCHER_SHARED_OBJ_DIR=$(TEMP)/watcher_shared
WATCHER_SHARED_LIB_DIR=libs/watcher_shared


#####################################################################
# 0x00-watcher
#####################################################################

SRC_DIR=src/0x00-watcher
SRC_FILES=$(shell find $(SRC_DIR) -name "*.cpp")
OBJ_DIR=$(TEMP)/0x00-watcher
OBJ_FILES=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(OBJ_DIR)/ext/sqlite/sqlite3.o $(OBJ_DIR)/ext/htmlstreamparser.o
PLUGINS_FOLDER=bin/plugins

SDL_CFLAGS=$(shell sdl2-config --cflags)
SDL_LDFLAGS=$(shell sdl2-config --libs) -lSDL2_image
CURL_CFLAGS=$(shell curl-config --cflags)
CURL_LDFLAGS=$(shell curl-config --libs)
CPPFLAGS=-g -std=c++17 -I$(SRC_DIR) -I$(SRC_DIR)/ext -Isrc/watcher_shared $(SDL_CFLAGS) $(CURL_CFLAGS) -Wno-format-security
CFLAGS=-g -I$(SRC_DIR) -I$(SRC_DIR)/ext
LDFLAGS=-g $(SDL_LDFLAGS) $(CURL_LDFLAGS) -ldl -lpthread -lGL -lboost_system -lboost_filesystem -pthread

0x00-watcher: $(OBJ_FILES) $(WATCHER_SHARED_LIB_DIR)/watcher_shared.a $(PLUGINS_FOLDER)/geolocation.so
	g++ -o bin/$@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	g++ $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/ext/sqlite/sqlite3.o: $(SRC_DIR)/ext/sqlite/sqlite3.c
	mkdir -p $(@D)
	gcc $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/ext/htmlstreamparser.o: $(SRC_DIR)/ext/htmlstreamparser.c
	mkdir -p $(@D)
	gcc $(CFLAGS) -c -o $@ $<


#####################################################################
# watcher_shared: library containing shared code between 0x00-watcher 
# and its plugins.
#####################################################################

WATCHER_SHARED_SRC_FILES=$(shell find src/watcher_shared -name "*.cpp")
WATCHER_SHARED_OBJ_FILES=$(patsubst src/watcher_shared/%.cpp,$(WATCHER_SHARED_OBJ_DIR)/%.o,$(WATCHER_SHARED_SRC_FILES))
WATCHER_SHARED_CPP_FLAGS=-g -std=c++17 -Isrc/watcher_shared $(SDL_CFLAGS)
$(WATCHER_SHARED_OBJ_DIR)/%.o: src/watcher_shared/%.cpp
	mkdir -p $(@D)
	g++ $(WATCHER_SHARED_CPP_FLAGS) -c -o $@ $<

$(WATCHER_SHARED_LIB_DIR)/watcher_shared.a: $(WATCHER_SHARED_OBJ_FILES)
	mkdir -p $(WATCHER_SHARED_LIB_DIR)
	ar -cvr $(WATCHER_SHARED_LIB_DIR)/watcher_shared.a $(WATCHER_SHARED_OBJ_FILES)


#####################################################################
# Plugins
#####################################################################

PLUGINS_CPP_FLAGS=-g -std=c++17 -fPIC -shared -Isrc/watcher_shared -I$(SRC_DIR)/ext
$(PLUGINS_FOLDER)/geolocation.so: src/geolocation/geolocation.cpp src/geolocation/geolocation.h
	g++ $(PLUGINS_CPP_FLAGS) -Isrc/geolocation -o $@ $<


#####################################################################
# Support actions
#####################################################################

.PHONY: clean
clean:
	rm -rf $(TEMP)
	rm -rf $(WATCHER_SHARED_LIB_DIR)

.PHONY: cleanlib
cleanlib:
	rm -rf $(WATCHER_SHARED_OBJ_FILES)
	rm -rf $(WATCHER_SHARED_LIB_DIR)
