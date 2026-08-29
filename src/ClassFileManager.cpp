#include"Gui.h"

namespace FileManagerUtils{
    std::string current_dir;
    std::string selected_path_file;
    std::vector<FileManagerUtils::FileItem>items;
}

FileManager::FileManager():Screen(){
    FileManagerUtils::current_dir = "ux0:";
    FileManagerUtils::selected_path_file = "";
}

void FileManager::update(){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        ImGui_ImplSDL2_ProcessEvent(&event);

        ImGuiIO& io = ImGui::GetIO();

        if(event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION){
            float mouse_x = event.tfinger.x * W_SCREEN;
            float mouse_y = event.tfinger.y * H_SCREEN;
            
            io.AddMousePosEvent(mouse_x, mouse_y);
            io.AddMouseButtonEvent(0, true);
        }
        else if (event.type == SDL_FINGERUP){
            io.AddMouseButtonEvent(0, false);
        }

        if(event.type == SDL_CONTROLLERBUTTONDOWN){
            switch (event.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_A:
                        
                    break;
                case SDL_CONTROLLER_BUTTON_B: // O
                        OmniBook::currentScreen = SCREEN::HOME;
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: // L
                        
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER://R
                        
                    break;
                case SDL_CONTROLLER_BUTTON_BACK: //SELECT
                        
                    break;
            }
        }
    }

    if(FileManagerUtils::selected_path_file!=""){
        OmniBook::currentScreen = SCREEN::DOCVIEW;
        DocumentView*d = OmniBook::screens[2];
        d->loadDocument(FileManagerUtils::selected_path_file.c_str());
        FileManagerUtils::items.clear();
    }
}

void FileManager::render(){
    SDL_SetRenderDrawColor(OmniBook::renderer,0,0,0,255);
    SDL_RenderClear(OmniBook::renderer);

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f,0.1f,0.1f,1.f);
    style.Colors[ImGuiCol_Text] = ImVec4(1.f,1.f,1.f,1.f);

    ImGui::SetNextWindowPos(ImVec2(W_SCREEN/2, H_SCREEN/2),ImGuiCond_Always,ImVec2(0.5f,0.5f));
    ImGui::SetNextWindowSize(ImVec2(W_SCREEN,H_SCREEN));

    FileManagerUtils::items = FileManagerUtils::GetFileList(FileManagerUtils::current_dir.c_str());

    ImGui::Begin("Libreria");

    for (const auto& item : FileManagerUtils::items) {
        // Prepariamo l'etichetta con un'icona testuale
        std::string icon = item.isDir ? "[DIR] " : "[BOOK] ";
        std::string label = icon + item.name;

        if (item.isDir) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.7f, 1.0f, 1.0f));

        if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0, 45))) {
            if (item.isDir) {
                if (item.name == "..") {
                    std::size_t ind = FileManagerUtils::current_dir.find_last_of('/');
                    if(ind>4)
                        FileManagerUtils::current_dir = FileManagerUtils::current_dir.substr(0,ind);
                    else
                        FileManagerUtils::current_dir = "ux0:";
                } else {
                    FileManagerUtils::current_dir += "/" + item.name;
                }
                FileManagerUtils::items = FileManagerUtils::GetFileList(FileManagerUtils::current_dir.c_str());
            } else {
                OmniBook::lb.name = item.name;
                OmniBook::lb.page = -1;
                FileManagerUtils::selected_path_file = FileManagerUtils::current_dir + "/" + item.name;
                OmniBook::lb.path = FileManagerUtils::selected_path_file;
            }
        }

        if (item.isDir) ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),OmniBook::renderer);

    SDL_RenderPresent(OmniBook::renderer);
}

FileManager::~FileManager(){}