

static bool checkbox = false;
static bool checkbox2 = false;
static int slider = 0;

static bool i[5] = {false, false, false, false, false};

static int item;

static const char* items[5] = { "item 1", "item 2", "item 3", "item 4", "item 5"};

static char text[256] = "";

static int key;

static int key2;

static float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;