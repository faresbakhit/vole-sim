// https://github.com/ocornut/imgui/blob/71c77c081ac36841e682498229088e7678207112/examples/libs/emscripten/emscripten_mainloop_stub.h
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <functional>
static std::function<void()> MainLoopForEmscriptenP;
static void MainLoopForEmscripten() { MainLoopForEmscriptenP(); }
#define EMSCRIPTEN_MAINLOOP_BEGIN MainLoopForEmscriptenP = [&]() { do
#define EMSCRIPTEN_MAINLOOP_END                                                                                        \
	while (0)                                                                                                          \
		;                                                                                                              \
	}                                                                                                                  \
	;                                                                                                                  \
	emscripten_set_main_loop(MainLoopForEmscripten, 0, true)
#else
#define EMSCRIPTEN_MAINLOOP_BEGIN
#define EMSCRIPTEN_MAINLOOP_END
#endif
