#include"Gui.h"

Home::Home() : Screen(){
    this->logo = IMG_LoadTexture(OmniBook::renderer,"app0:/res/texture/logo_home.png");
}

void Home::update(){
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
            if(event.cbutton.button == SDL_CONTROLLER_BUTTON_A){
                OmniBook::NightModeON = !OmniBook::NightModeON;
                OmniBook::updateTheme();
            }
        }
    }
}

void Home::render(){

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(W_SCREEN,H_SCREEN));
    ImGui::Begin("##Home",NULL,ImGuiWindowFlags_NoDecoration);

        ImGui::SetCursorPos(ImVec2(460,40));
        ImGui::Image((void*)this->logo,ImVec2(461,110));

        ImGui::SetCursorPos(ImVec2(40,190));
        ImGui::BeginGroup();
            if(ImGui::Button("Open file",ImVec2(250,60)))
                OmniBook::currentScreen = SCREEN::FILEMANAGER;
            ImGui::SetCursorPosY(290);
            if(ImGui::Button("Controls",ImVec2(250,60))){

            }
            ImGui::SetCursorPosY(390);
            if(ImGui::Button("About",ImVec2(250,60))){

            }
        ImGui::EndGroup();

        if(OmniBook::lb.name!=""){

            ImGui::SetCursorPos(ImVec2(520,190));
            ImGui::PushStyleColor(ImGuiCol_ChildBg,ImVec4(0.15f,0.15f,0.15f,1.f));
            ImGui::BeginChild("##ResumeCard",ImVec2(380,280),true);
                
                float cardWidth = 380.f;
                ImGui::SetCursorPosY(40);
                const char* text1 = OmniBook::lb.name.c_str();
                ImGui::SetCursorPosX((cardWidth - ImGui::CalcTextSize(text1).x)*0.5f);
                ImGui::Text(text1);

                ImGui::SetCursorPosY(80);
                const char*text2 = fmt::format("Last read: {}",OmniBook::lb.page).c_str();
                ImGui::SetCursorPosX((cardWidth-ImGui::CalcTextSize(text2).x)*0.5f);
                ImGui::TextDisabled(text2);

                ImGui::SetCursorPosY(140);
                ImGui::SetCursorPosX((cardWidth-200)*0.5f);
                if(ImGui::Button("Resume",ImVec2(200,70))){
                    DocumentView*d = OmniBook::screens[2];
                    d->loadDocument(OmniBook::lb.path.c_str(),OmniBook::lb.page);
                    FileManagerUtils::selected_path_file = OmniBook::lb.path;
                    OmniBook::currentScreen = SCREEN::DOCVIEW;
                }

            ImGui::EndChild();
            ImGui::PopStyleColor();

        }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),OmniBook::renderer);
    SDL_RenderPresent(OmniBook::renderer);
}

Home::~Home(){
    SDL_DestroyTexture(this->logo);
}