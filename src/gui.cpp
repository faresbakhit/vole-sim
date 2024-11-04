#include "example_draw.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif
#include <bitset>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include "emscripten_mainloop_stub.h"
#endif

#include "font_source_code_pro.h"
#include "vole.h"

#define OS_HEX1 std::hex << std::uppercase

static ImVector<ImVec2> canvasPoints;

class CanvasDraw : public vole::ControlUnit::ControlUnit {
	using vole::ControlUnit::ControlUnit;
	vole::ShouldHalt Execute() override {
		canvasPoints.push_back({static_cast<float>(mac->mem[operandXY]), static_cast<float>(mac->mem[operandXY + 1])});
		return vole::ShouldHalt::NO;
	};
	std::string Humanize() override {
		std::ostringstream os;
		os << "Draw the point in cell " << OS_HEX1 << operandXY;
		return os.str();
	};
	~CanvasDraw() = default;
};

void ShowControlWindow(vole::Machine &mac, ImGuiIO &io) {
	static int speed = 5;
	static bool isRunning = false;
	static unsigned int currentTime, lastTime = 0;

	currentTime = SDL_GetTicks();

	if (isRunning && (currentTime > lastTime + 1000 / speed)) {
		if (mac.Step() == vole::ShouldHalt::YES) {
			isRunning = false;
		};
		lastTime = currentTime;
	}

	if (ImGui::Begin("Control", NULL,
					 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
		ImGui::InputScalar("Program Counter", ImGuiDataType_U8, &mac.reg.pc, NULL, NULL, "%02X",
						   ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		ImGui::SeparatorText("Execution");
		if (ImGui::Button("Run until HALT", {io.DisplaySize.x * 1.f / 6.f - 20, 100})) {
			isRunning = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Run one instruction", {io.DisplaySize.x * 1.f / 6.f - 20, 100})) {
			mac.Step();
		}
		ImGui::SeparatorText("Speed");
		ImGui::Text("Specify number of instructions per second (IPS):");
		ImGui::VSliderInt("IPS", {50.f, 240}, &speed, 1, 100, "%d", ImGuiSliderFlags_ClampOnInput);
		ImGui::End();
	}
}

void ShowCanvasWindow() {
	static ImVec2 scrolling(0.0f, 0.0f);

	ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
	ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

	if (canvas_sz.x < 50.0f)
		canvas_sz.x = 50.0f;
	if (canvas_sz.y < 50.0f)
		canvas_sz.y = 50.0f;
	ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

	// Draw border and background color
	ImGuiIO &io = ImGui::GetIO();
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
	draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

	// This will catch our interactions
	const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y);
	const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

	// Context menu (under default mouse threshold)
	ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
	if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
		ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
	if (ImGui::BeginPopup("context")) {
		if (ImGui::MenuItem("Remove one", NULL, false, canvasPoints.Size > 0)) {
			canvasPoints.resize(canvasPoints.size() - 2);
		}
		if (ImGui::MenuItem("Remove all", NULL, false, canvasPoints.Size > 0)) {
			canvasPoints.clear();
		}
		ImGui::EndPopup();
	}

	// Draw grid + all lines in the canvas
	draw_list->PushClipRect(canvas_p0, canvas_p1, true);
	const float GRID_STEP = 64.0f;
	for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
		draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y),
						   IM_COL32(200, 200, 200, 40));
	for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
		draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y),
						   IM_COL32(200, 200, 200, 40));
	for (int n = 0; n < canvasPoints.Size - (canvasPoints.Size % 2); n += 2)
		draw_list->AddLine(ImVec2(origin.x + canvasPoints[n].x, origin.y + canvasPoints[n].y),
						   ImVec2(origin.x + canvasPoints[n + 1].x, origin.y + canvasPoints[n + 1].y),
						   IM_COL32(255, 255, 0, 255), 2.0f);
	draw_list->PopClipRect();
}

void ShowInstructionEditor(vole::Machine &mac) {
	if (ImGui::BeginTable("##Instruction", 4, ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Low Byte", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("High Byte", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("Note", ImGuiTableColumnFlags_WidthStretch);

		for (size_t row = 0; row < (vole::Memory::SIZE >> 1); row++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextDisabled("%02zX", 2 * row);
			ImGui::TableSetColumnIndex(1);
			ImGui::PushID(row);
			ImGui::PushItemWidth(60);
			ImGui::InputScalarN("##", ImGuiDataType_U8, &mac.mem[2 * row], 2, NULL, NULL, "%02X",
								ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_AutoSelectAll);
			ImGui::PopItemWidth();
			ImGui::PopID();
			ImGui::TableSetColumnIndex(2);
			ImGui::TextDisabled("%02zX", 2 * row + 1);
			ImGui::TableSetColumnIndex(3);
			vole::ControlUnit *cu = vole::ControlUnit::Decode(&mac, 2 * row);
			ImGui::TextUnformatted(cu->Humanize().c_str());
			delete cu;
			if (row == mac.reg.pc >> 1) {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(39, 73, 114, 255));
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, IM_COL32(39, 73, 114, 255));
			}
		}

		ImGui::EndTable();
	}
}

void ShowMainMemory(vole::Machine &mac, ImGuiIO &io) {
	ImGui::BeginChild("Memory Table", {0, io.DisplaySize.y * 1.f / 2.f - 100}, ImGuiChildFlags_None,
					  ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginTable("##Memory", 4,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersInnerV)) {
		ImGui::TableSetupColumn("Address");
		ImGui::TableSetupColumn("Hex");
		ImGui::TableSetupColumn("Bin");
		ImGui::TableSetupColumn("Float");
		ImGui::TableHeadersRow();

		for (size_t row = 0; row < vole::Memory::SIZE; row++) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%02zX", row);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%02X", mac.mem[row]);

			ImGui::TableSetColumnIndex(2);
			std::bitset<8> bin(mac.mem[row]);
			ImGui::TextUnformatted(bin.to_string().c_str());

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%08f", vole::Float::Decode(mac.mem[row]));
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
	if (ImGui::Button("Clear Memory"))
		ImGui::OpenPopup("Clear Memory?");

	if (ImGui::BeginPopupModal("Clear Memory?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Are you sure you want to reset all memory cells?");
		ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			mac.mem.Reset();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void ShowRegisters(vole::Machine &mac, ImGuiIO &io) {
	ImGui::BeginChild("Registers Table", ImVec2(0, io.DisplaySize.y * 1.f / 2.f - 100), ImGuiChildFlags_None,
					  ImGuiWindowFlags_None);
	if (ImGui::BeginTable("##Registers", 4,
						  ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersInnerV)) {
		ImGui::TableSetupColumn("Register");
		ImGui::TableSetupColumn("Hex");
		ImGui::TableSetupColumn("Bin");
		ImGui::TableSetupColumn("Float");
		ImGui::TableHeadersRow();

		for (size_t row = 0; row < 16; row++) {
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%01zX", row);

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%02X", mac.reg[row]);

			ImGui::TableSetColumnIndex(2);
			std::bitset<8> bin(mac.reg[row]);
			ImGui::TextUnformatted(bin.to_string().c_str());

			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%08f", vole::Float::Decode(mac.reg[row]));
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
	if (ImGui::Button("Clear Registers")) {
		mac.reg.Reset();
	}
}

class GraphicalSreen : public vole::Screen {
	void clear() { std::cout << u8"\033[2J\033[1;1H"; }

	void write(uint8_t c) { std::cout << (char)c; }

	~GraphicalSreen() = default;
};


inline static std::array<vole::ControlUnitBuilder, 16> ExtendedControlUnitFactory = {
	vole::NothingBuilder,
	[](vole::Machine *mac, uint8_t at) { return new vole::Load1(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Load2(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Store(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Move(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Add1(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Add2(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Or(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::And(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Xor(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Rotate(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Jump(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new vole::Halt(mac, at); },
	[](vole::Machine *mac, uint8_t at) { return new CanvasDraw(mac, at); },
	vole::UnusedBuilder,
	vole::UnusedBuilder,
};

// Main code
int main(int, char **) {
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char *glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char *glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
						SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags =
		(SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window *window = SDL_CreateWindow("faresbakhit/vole-sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
										  720, window_flags);
	if (window == nullptr) {
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.IniFilename = nullptr;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	io.Fonts->AddFontFromMemoryCompressedTTF(font_source_code_pro_compressed_data, font_source_code_pro_compressed_size,
											 21.0f);
	io.Fonts->AddFontDefault();

	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	vole::Screen* scr = new GraphicalSreen;
	vole::Machine mac = vole::Machine(nullptr, ExtendedControlUnitFactory);
	*mac.mem.Array() = vole::example::DRAW;
	const ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

	// Main loop
	bool done = false;
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_BEGIN
#else
	while (!done)
#endif
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
				event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		ImGui::SetNextWindowPos({io.DisplaySize.x * 1.f / 3.f, 0});
		if (ImGui::Begin("Control", NULL, windowFlags)) {
			ShowControlWindow(mac, io);
			ImGui::End();
		}

		ImGui::SetNextWindowSize({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		ImGui::SetNextWindowPos({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		if (ImGui::Begin("Canvas", NULL, windowFlags)) {
			ShowCanvasWindow();
			ImGui::End();
		}

		ImGui::SetNextWindowSize({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y});
		ImGui::SetNextWindowPos({0, 0});
		if (ImGui::Begin("Instruction Editor", NULL, windowFlags)) {
			ShowInstructionEditor(mac);
			ImGui::End();
		}

		ImGui::SetNextWindowSize({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		ImGui::SetNextWindowPos({io.DisplaySize.x * 2.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		if (ImGui::Begin("Main Memory", NULL, windowFlags)) {
			ShowMainMemory(mac, io);
			ImGui::End();
		}

		ImGui::SetNextWindowSize({io.DisplaySize.x * 1.f / 3.f, io.DisplaySize.y * 1.f / 2.f});
		ImGui::SetNextWindowPos({io.DisplaySize.x * 2.f / 3.f, 0});
		if (ImGui::Begin("Registers", NULL, windowFlags)) {
			ShowRegisters(mac, io);
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
					 clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}
#ifdef __EMSCRIPTEN__
	EMSCRIPTEN_MAINLOOP_END;
#endif

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	delete scr;

	return 0;
}
