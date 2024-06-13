
#include "custom.h"

struct tab_animation {
    float background, border, opacity;
};

bool custom::tab(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags,char* icon,bool v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // animation

    static std::map<ImGuiID, tab_animation> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end()) {
        anim.insert({ id, {0.0f} });
        it_anim = anim.find(id);
    }

    it_anim->second.background = ImLerp(it_anim->second.background, v ? 0.2f : 0.0f, 0.05f * (1.0f - ImGui::GetIO().DeltaTime));
    it_anim->second.border = ImLerp(it_anim->second.border, hovered ? 0.8f : 0.0f, 0.05f * (1.0f - ImGui::GetIO().DeltaTime));
    it_anim->second.opacity = ImLerp(it_anim->second.opacity, v ? 0.8f : 0.4f, 0.1f * (1.0f - ImGui::GetIO().DeltaTime));

    // Render

    window->DrawList->AddRectFilled(bb.Min, bb.Max, (ImColor)ImVec4( 
        (float)245 / 255, 
        (float)40 / 255, 
        (float)145 / 255, 
        it_anim->second.background) 
    , 7.0f);

    window->DrawList->AddRect(bb.Min, bb.Max, (ImColor)ImVec4(
        (float)245 / 255,
        (float)40 / 255,
        (float)145 / 255,
        it_anim->second.border)
    , 7.0f);

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");

    ImVec2 text_size = CalcTextSize(label, NULL, true);
    ImVec2 text_pos = bb.GetCenter() - text_size * 0.5f;

    ImGui::PushFont(iconfont);
    window->DrawList->AddText(ImVec2(pos.x + 10, text_pos.y+4), (ImColor)ImVec4(1.0f, 1.0f, 1.0f, it_anim->second.opacity), icon);
    ImGui::PopFont();

    window->DrawList->AddText(ImVec2(pos.x + 35,text_pos.y), (ImColor)ImVec4(1.0f, 1.0f, 1.0f, it_anim->second.opacity), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool custom::tabchild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    return custom::tabchild_ex(str_id, window->GetID(str_id), size_arg, border, extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoMove);
}

bool custom::tabchild_ex(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags)
{

    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;

    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
    flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

    // Size
    const ImVec2 content_avail = GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
    if (size.x <= 0.0f)
        size.x = ImMax(content_avail.x + size.x, 4.0f); 
    if (size.y <= 0.0f)
        size.y = ImMax(content_avail.y + size.y, 4.0f);
    SetNextWindowSize(size);


    const char* temp_window_name;
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    const float backup_border_size = g.Style.ChildBorderSize;
    if (!border)
        g.Style.ChildBorderSize = 0.0f;
    bool ret = Begin(temp_window_name, NULL, flags);
    g.Style.ChildBorderSize = backup_border_size;

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;
    child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;

    parent_window->DrawList->AddText(child_window->Pos + ImVec2(5, 2), ImColor(255, 255, 255, 129), name);
    ImGui::NewLine();

    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(id + 1, child_window); 
        g.ActiveIdSource = ImGuiInputSource_Nav;
    }
    return ret;
}

struct subtab_animation {
    float opacity, r, g, b;
};

bool custom::subtab(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags, bool v) {
    
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // animation

    static std::map<ImGuiID, subtab_animation> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end()) {
        anim.insert({ id, {0.0f} });
        it_anim = anim.find(id);
    }

    it_anim->second.r = ImLerp(it_anim->second.r, v ? (float)245/255 : (float)255/255 , 0.1f * (1.0f - ImGui::GetIO().DeltaTime));
    it_anim->second.g = ImLerp(it_anim->second.g, v ? (float)40/255 : (float)255/255 , 0.1f * (1.0f - ImGui::GetIO().DeltaTime));
    it_anim->second.b = ImLerp(it_anim->second.b, v ? (float)145/255 : (float)255/255 , 0.1f * (1.0f - ImGui::GetIO().DeltaTime));
    it_anim->second.opacity = ImLerp(it_anim->second.opacity, v ? 0.8f : 0.4f && hovered ? 0.6f : 0.4f, 0.1f * (1.0f - ImGui::GetIO().DeltaTime));

    if (g.LogEnabled)
        LogSetNextTextDecoration("[", "]");

    ImVec2 text_size = CalcTextSize(label, NULL, true);
    ImVec2 text_pos = bb.GetCenter() - text_size * 0.5f;

    window->DrawList->AddText(ImVec2(pos.x, text_pos.y), (ImColor)ImVec4(it_anim->second.r, it_anim->second.g, it_anim->second.b, it_anim->second.opacity), label);
    ImGui::SameLine();
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;

}

bool custom::MenuChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 0));
    return custom::MenuChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoMove);
}

bool custom::MenuChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;
    const ImGuiStyle& style = g.Style;
    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
    flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

    // Size
    const ImVec2 content_avail = GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
    if (size.x <= 0.0f)
        size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
    if (size.y <= 0.0f)
        size.y = ImMax(content_avail.y + size.y, 4.0f);

    float headerHeight = 35.0f; 
    ImVec2 startPosition = parent_window->DC.CursorPos;
    startPosition.y += headerHeight;
    ImVec2 contentSize = ImVec2(size.x, size.y - headerHeight);
    SetNextWindowSize(contentSize);
    SetNextWindowPos(startPosition);


    ImRect bb(parent_window->DC.CursorPos.x, parent_window->DC.CursorPos.y, parent_window->DC.CursorPos.x + size.x, parent_window->DC.CursorPos.y + size.y);
    const char* temp_window_name;
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    parent_window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(30, 30, 30), 10.0f);
    parent_window->DrawList->AddRect(bb.Min, bb.Min + ImVec2(size.x, 35), ImColor(44, 44, 44), 10.0f, ImDrawCornerFlags_Top);
    parent_window->DrawList->AddRectFilledMultiColorRounded(bb.Min, bb.Min + ImVec2(size.x - 45, 35), ImColor(25, 25, 25, 255), ImColor(245, 40, 145, 50), ImColor(245, 40, 145, 0), ImColor(245, 40, 145, 0), ImColor(245, 40, 145, 50), 10.0f, ImDrawCornerFlags_TopLeft);
    parent_window->DrawList->AddRect(bb.Min, bb.Max, ImColor(40, 40, 40), 10.0f);

    parent_window->DrawList->AddText(bb.Min + ImVec2(10, 10), ImColor(255, 110, 185), name);

    ImGui::SetCursorPosY(parent_window->DC.CursorPos.y );

    const float backup_border_size = g.Style.ChildBorderSize;
    if (!border)
        g.Style.ChildBorderSize = 0.0f;
    bool ret = Begin(temp_window_name, NULL, flags);
    g.Style.ChildBorderSize = backup_border_size;

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;
    child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;


    // Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
    // While this is not really documented/defined, it seems that the expected thing to do.
    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    // Process navigation-in immediately so NavInit can run on first frame
    if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(id + 1, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
        g.ActiveIdSource = ImGuiInputSource_Nav;
    }
    ImGui::PopStyleVar(2);
    return ret;
}