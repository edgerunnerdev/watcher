SRC_DIR=src
OBJ_DIR=temp/linux
SRC_FILES=$(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(OBJ_DIR)/ext/sqlite/sqlite3.o $(OBJ_DIR)/ext/htmlstreamparser.o

SDL_CFLAGS=$(shell sdl2-config --cflags)
SDL_LDFLAGS=$(shell sdl2-config --libs) -lSDL2_image
CURL_CFLAGS=$(shell curl-config --cflags)
CURL_LDFLAGS=$(shell curl-config --libs)
CPPFLAGS=-g -std=c++17 -Isrc -Isrc/ext $(SDL_CFLAGS) $(CURL_CFLAGS) -Wno-format-security
CFLAGS=-g -Isrc -Isrc/ext
LDFLAGS=-g $(SDL_LDFLAGS) $(CURL_LDFLAGS) -ldl -lpthread -lGL -lboost_system -lboost_filesystem -pthread


0x00-watcher: $(OBJ_FILES)
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

.PHONY: clean

clean:
	rm -rf $(OBJ_DIR)