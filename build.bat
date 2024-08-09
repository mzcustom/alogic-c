del alogic-c.exe
del index.*

gcc main.c -o alogic-c.exe -L lib/ -lraylib -lwinmm -lgdi32
emcc -o index.html main.c -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I. -I C:/raylib-master/raylib-master/src -I C:/raylib-master/raylib-master/src/external -L. -L C:/raylib-master/raylib-master/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 --shell-file C:/raylib-master/raylib-master/src/minshell.html C:/raylib-master/raylib-master/src/web/libraylib.a -DPLATFORM_WEB -s EXPORTED_FUNCTIONS=["_free","_malloc","_main"] -s EXPORTED_RUNTIME_METHODS=ccall
