SRC_DIR=src
OBJ_DIR=temp/linux
SRC_FILES=$(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(OBJ_DIR)/sqlite/sqlite3.o

SDL_CFLAGS=$(shell sdl2-config --cflags)
SDL_LDFLAGS=$(shell sdl2-config --libs)
CPPFLAGS=-std=c++17 -Isrc $(SDL_CFLAGS)
CFLAGS=-Isrc
LDFLAGS=$(SDL_LDFLAGS) -ldl -lpthread -lGL


0x00-watcher: $(OBJ_FILES)
	g++ -o bin/$@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	g++ $(CPPFLAGS) -c -o $@ $<

$(OBJ_DIR)/sqlite/sqlite3.o: $(SRC_DIR)/sqlite/sqlite3.c
	mkdir -p $(@D)
	gcc $(CFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -rf $(OBJ_DIR)