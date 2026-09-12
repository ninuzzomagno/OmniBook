#include"BasicIncludes.h"
#include"Gui.h"

namespace OmniBook{
    Screen*screens[3];

    SCREEN currentScreen;

    SDL_Window*window;
    SDL_Renderer*renderer;

    bool NightModeON;

    Traslation tr;

    ImFont*my_font;

    int code_exit;

    LastBook lb;
};

int power_callback(int notifyID,int notifyCount, int powerInfo, void*pCommon){
    if(OmniBook::currentScreen == SCREEN::DOCVIEW){
        if(powerInfo &SCE_POWER_CB_APP_RESUMING)
            OmniBook::code_exit = 1;
        else if(powerInfo & SCE_POWER_CB_BUTTON_PS_PRESS){
            DocumentView*d = OmniBook::screens[2];
            d->saveData();
            OmniBook::code_exit = 2;
        }
    }
    return 0;
}

void debug_color(SDL_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
	sceKernelDelayThread(1000000);
}

void OmniBook::updateTheme(){
    ImGuiStyle&style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 16.0f;
    style.ChildRounding     = 16.0f;
    style.FrameRounding     = 30.0f;
    style.PopupRounding     = 12.0f;
    style.GrabRounding      = 12.0f;

    style.WindowPadding     = ImVec2(20, 20);
    style.FramePadding      = ImVec2(20, 15); // Bottoni alti e comodi per il touch
    style.ItemSpacing       = ImVec2(10, 15);
    style.WindowBorderSize  = 0.0f; // Rimuoviamo i bordi per un look "Flat"
    style.ChildBorderSize   = 0.0f;

    if(OmniBook::NightModeON){
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.13f, 1.f);
        colors[ImGuiCol_Button]   = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_Text]     = ImVec4(1.f, 1.f, 1.f, 1.00f);
        colors[ImGuiCol_ChildBg]  = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    
        colors[ImGuiCol_Button]           = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);
        colors[ImGuiCol_ButtonHovered]    = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
        colors[ImGuiCol_ButtonActive]     = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
        
    }
    else{
        colors[ImGuiCol_WindowBg]         = ImVec4(0.00f, 0.06f, 0.39f, 1.00f);
        colors[ImGuiCol_ChildBg]          = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);
        
        // Bottoni Standard (Blu Material)
        colors[ImGuiCol_Button]           = ImVec4(0.10f, 0.45f, 0.91f, 1.00f);
        colors[ImGuiCol_ButtonHovered]    = ImVec4(0.26f, 0.52f, 0.96f, 1.00f);
        colors[ImGuiCol_ButtonActive]     = ImVec4(0.05f, 0.28f, 0.63f, 1.00f);
        
        colors[ImGuiCol_Text]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    }
}

void OmniBook::init_net(){
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);

    int net_mem_size = 1024 * 1024; // 1MB
    void* net_memory = memalign(64, net_mem_size); 
    
    if (net_memory == NULL) {
        while(1);
    }

    SceNetInitParam netInitParam;
    netInitParam.memory = net_memory;
    netInitParam.size = net_mem_size;
    netInitParam.flags = 0;
    
    sceNetInit(&netInitParam);
    sceNetCtlInit();

    sceSslInit(32 * 1024);
    sceHttpInit(1024 * 1024);

    sceHttpsDisableOption(0x01);
}

void OmniBook::init_graphic(){
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");
    SDL_setenv("VITA_DISABLE_TOUCH_BACK", "1", 1);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

    IMG_Init(IMG_INIT_PNG);

    if (SDL_NumJoysticks() > 0) {
        if (SDL_IsGameController(0)) {
            SDL_GameControllerOpen(0);
        }
    }

    int cbid = sceKernelCreateCallback("PowerCallback",0,power_callback,NULL);
    if(cbid>=0)
        scePowerRegisterCallback(cbid);

    OmniBook::window = SDL_CreateWindow("OmniBook",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,W_SCREEN,H_SCREEN,0);
    OmniBook::renderer = SDL_CreateRenderer(OmniBook::window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = true;
    config.RasterizerMultiply = 1.2f;

    OmniBook::code_exit = 0;
    OmniBook::my_font = io.Fonts->AddFontFromFileTTF("app0:/res/font/DroidSans.ttf",30.f,&config,io.Fonts->GetGlyphRangesDefault());

    ImGui_ImplSDL2_InitForSDLRenderer(OmniBook::window,OmniBook::renderer);
    ImGui_ImplSDLRenderer2_Init(OmniBook::renderer);
}

void OmniBook::init(){
    OmniBook::init_graphic();

    OmniBook::init_net();

    OmniBook::screens[0] = new Home();
    OmniBook::screens[1] = new FileManager();
    OmniBook::screens[2] = new DocumentView();

    OmniBook::currentScreen = SCREEN::HOME;
    OmniBook::NightModeON = false;

    OmniBook::tr.idx_from = 1;
    OmniBook::tr.idx_to = 0;

    OmniBook::loadData();

    OmniBook::updateTheme();
}

void OmniBook::mainloop(){
    while(OmniBook::currentScreen != SCREEN::EXIT){
        sceKernelCheckCallback();
        OmniBook::screens[int(OmniBook::currentScreen)]->update();
        
        if(!OmniBook::code_exit!=0)
            OmniBook::screens[int(OmniBook::currentScreen)]->render();
    }
}

void OmniBook::cleanup(){
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(OmniBook::renderer);
    SDL_DestroyWindow(OmniBook::window);

    for (int i = 0; i < 3; i++) {
        if (screens[i] != nullptr) {
            delete screens[i];
            screens[i] = nullptr;
        }
    }

    SDL_Quit();
}

void OmniBook::saveData(int p){
    sceIoMkdir("ux0:/data/OMBK00001", 0777);
    nlohmann::json j_p;

    j_p["NightMode"] = OmniBook::NightModeON ? 1 : 0;
    j_p["Tr"]["From"] = OmniBook::tr.idx_from;
    j_p["Tr"]["To"] = OmniBook::tr.idx_to;

    std::ofstream FILE_O("ux0:/data/OMBK00001/config.json");
    FILE_O<<j_p.dump(4);
    FILE_O.close();

    std::ifstream FILE_P("ux0:/data/OMBK00001/library.json");
    j_p = nlohmann::json::object();
    if(FILE_P){
        FILE_P>>j_p;
        FILE_P.close();
    }
    j_p[FileManagerUtils::selected_path_file] = p;
    j_p["last"]["path"] = FileManagerUtils::selected_path_file;
    
    OmniBook::lb.page = p;
    
    std::size_t id = FileManagerUtils::selected_path_file.find_last_of('/');
    j_p["last"]["name"] = FileManagerUtils::selected_path_file.substr(id+1,FileManagerUtils::selected_path_file.length());
    j_p["last"]["page"] = p;

    FILE_O.open("ux0:/data/OMBK00001/library.json");
    FILE_O<<j_p.dump(4);
    FILE_O.close();
}

void OmniBook::loadData(){
    std::ifstream FILE_I("ux0:/data/OMBK00001/config.json");
    if(FILE_I){
        nlohmann::json j_i;
        FILE_I>>j_i;
        FILE_I.close();

        OmniBook::tr.idx_from = j_i["Tr"]["From"];
        OmniBook::tr.idx_to = j_i["Tr"]["To"];

        OmniBook::NightModeON = j_i["NightMode"] == 1 ? true : false;
    }
    OmniBook::lb.name = "";
    OmniBook::lb.page = -1;
    FILE_I.open("ux0:/data/OMBK00001/library.json");
    if(FILE_I){
        nlohmann::json j_i;
        FILE_I>>j_i;
        FILE_I.close();
        
        if(j_i.contains("last")){
            OmniBook::lb.name = j_i["last"]["name"];
            OmniBook::lb.page = j_i["last"]["page"];
            OmniBook::lb.path = j_i["last"]["path"];
        }

    }
}