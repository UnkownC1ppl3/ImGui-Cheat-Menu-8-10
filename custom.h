#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/imgui.h"

#include <map>

// font with icons
extern ImFont* iconfont;

using namespace ImGui;

namespace custom {
	bool tab(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags = NULL,char* icon = NULL, bool v = NULL);
	bool tabchild(const char* str_id, const ImVec2& size_arg, bool border = false, ImGuiWindowFlags extra_flags = NULL);
	bool tabchild_ex(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags);
	bool subtab(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags = NULL, bool v = NULL);
	bool MenuChild(const char* str_id, const ImVec2& size_arg, bool border = false, ImGuiWindowFlags extra_flags = NULL);
	bool MenuChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags);
}