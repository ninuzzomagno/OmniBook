#include"Gui.h"

DocumentView::DocumentView():Screen(){
    this->showUI = false;
    this->doubleTap = false;
    this->showTraslation = false;
    this->showSelection = false;
    this->time_heldDown = 0;
    this->time = 0;
    this->time_traslataion = 0;
    this->doc = nullptr;

    this->battery_texture = IMG_LoadTexture(OmniBook::renderer,"app0:/res/icons/battery_24.png");
    this->wifi_texture = IMG_LoadTexture(OmniBook::renderer,"app0:/res/icons/wifi_24.png");
}

void DocumentView::saveData(){
    OmniBook::saveData(this->currPage-1);
}

void DocumentView::loadDocument(const char*path,int my_p = -1){
    if(this->doc){
        delete this->doc;
    }
    this->doc = new Document(path);
    if(my_p!=-1){
        this->currPage = this->doc->loadPage(my_p);
    }
    else{
        std::ifstream FILE_I("ux0:/data/OMBK00001/library.json");
        int p=0;
        if(FILE_I){
            nlohmann::json j_i;
            FILE_I>>j_i;
            FILE_I.close();
            
            if(j_i.contains(path))
                p = j_i[path];
        }

        this->currPage = this->doc->loadPage(p);
        OmniBook::lb.page = p;
    }
    this->showUI = false;
    this->doubleTap = false;
    this->showTraslation = false;
    this->showSelection = false;
    this->time = 0;
    this->time_traslataion = 0;

    this->scrollBar_rect = {950,0,10,H_SCREEN*float(H_SCREEN)/float(this->doc->getDocH())};
}

void DocumentView::update(){

    if(OmniBook::code_exit == 1){
        this->doc->reload();
        OmniBook::code_exit = 0;
    }

    SDL_Event event;
    while(SDL_PollEvent(&event)){
        ImGui_ImplSDL2_ProcessEvent(&event);

        if(OmniBook::code_exit == 2){
            this->doc->reload();
            OmniBook::code_exit = 0;
        }

        if(this->showUI){
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
                    case SDL_CONTROLLER_BUTTON_BACK:
                            this->showUI = !this->showUI;
                        break;
                }
            }
        }
        else{

            if(event.type == SDL_FINGERDOWN){
                if(!this->showTraslation){

                    this->time_heldDown = SDL_GetTicks();
                    this->mouse.x = event.tfinger.x*W_SCREEN;
                    this->mouse.y = event.tfinger.y*H_SCREEN;
                    
                    if(SDL_GetTicks() - this->time > 300){
                        this->time = SDL_GetTicks();
                        this->doubleTap = false;
                    }
                    else{
                        this->doubleTap = true;
                        this->time = 0;
                    }
                    
                    if(this->doubleTap){
                        this->traslation_rect.x0 = event.tfinger.x*W_SCREEN;
                        this->traslation_rect.y0 = event.tfinger.y*H_SCREEN;
                        this->traslation_rect.x1 = event.tfinger.x*W_SCREEN;
                        this->traslation_rect.y1 = event.tfinger.y*H_SCREEN;
                    }
                }
            }
            else if(event.type == SDL_FINGERMOTION){
                if(!this->showTraslation){
                    if(!this->doubleTap)
                        this->doc->scroll(-event.tfinger.dy*H_SCREEN);
                    else{
                        this->showSelection = true;
                        this->traslation_rect.x1 = event.tfinger.x*W_SCREEN;
                        this->traslation_rect.y1 = event.tfinger.y*H_SCREEN;

                        this->selection_rect.x = this->traslation_rect.x0;
                        this->selection_rect.y = this->traslation_rect.y0;
                        this->selection_rect.w = this->traslation_rect.x1 - this->traslation_rect.x0;
                        this->selection_rect.h = this->traslation_rect.y1 - this->traslation_rect.y0;
                    }
                }
            }
            else if(event.type == SDL_FINGERUP){
                if(this->showTraslation){
                    this->showSelection = false;
                    this->showTraslation = false;
                    this->traslation = "";
                }
                else{
                    if(this->doubleTap){
                        if(SDL_GetTicks() - this->time_traslataion > 2000){
                            this->showSelection = false;
                            this->doubleTap = false;
                            this->traslation = this->doc->translate(this->traslation_rect,OmniBook::tr.code[OmniBook::tr.idx_from],OmniBook::tr.code[OmniBook::tr.idx_to]);
                            if(this->traslation!="")
                                this->showTraslation = true;
                            this->time_traslataion = SDL_GetTicks();
                        }
                        else{
                            this->traslation = "Errore: attendere 2 secondi";
                        }
                    }
                    else{
                        if (this->traslation != ""){
                            this->traslation = this->doc->translate(this->traslation,OmniBook::tr.code[OmniBook::tr.idx_from],OmniBook::tr.code[OmniBook::tr.idx_to]);
                            if(!this->traslation.empty())
                                this->showTraslation = true;
                        }
                        this->time_heldDown = 0;
                    }
                }
            }

            if(event.type == SDL_CONTROLLERBUTTONDOWN){
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_A:
                            this->doc->nightToogle();
                        break;
                    case SDL_CONTROLLER_BUTTON_B: // CIRCLE
                            this->saveData();
                            FileManagerUtils::selected_path_file = "";
                            OmniBook::currentScreen = SCREEN::FILEMANAGER;
                            delete this->doc;
                            this->doc = nullptr;
                        break;
                    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: // L
                            this->currPage = this->doc->loadPreviousPage();
                            this->scrollBar_rect = {950,0,10,H_SCREEN*float(H_SCREEN)/float(this->doc->getDocH())};
                        break;
                    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER://R
                            this->currPage = this->doc->loadNextPage();
                            this->scrollBar_rect = {950,0,10,H_SCREEN*float(H_SCREEN)/float(this->doc->getDocH())};
                        break;
                    case SDL_CONTROLLER_BUTTON_BACK:
                            this->showUI = !this->showUI;
                        break;
                }
            }

            if(this->time_heldDown!=0){
                if(SDL_GetTicks()-this->time_heldDown>800){
                    fz_rect r;
                    this->traslation = this->doc->findWord(this->mouse,r);
                    this->selection_rect.x = r.x0;
                    this->selection_rect.y = r.y0;
                    this->selection_rect.w = r.x1-r.x0;
                    this->selection_rect.h = r.y1-r.y0;
                    this->time_heldDown = 0;
                    this->showSelection=true;
                }
            }

        }

    }
}

void DocumentView::render(){

    OmniBook::NightModeON ? SDL_SetRenderDrawColor(OmniBook::renderer,0,0,0,255) : SDL_SetRenderDrawColor(OmniBook::renderer,255,255,255,255);
    SDL_RenderClear(OmniBook::renderer);

    this->doc->render();
    
    this->displayUI();
    
    SDL_RenderPresent(OmniBook::renderer);
}

void DocumentView::displayUI(){
    this->displaySelection();

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    this->displayScrollBar();
    
    this->displayTraslation();

    this->displayTopBottomBar();        

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(),OmniBook::renderer);
}

void DocumentView::displayScrollBar(){
    this->scrollBar_rect.y=this->doc->getScrollY() * float(H_SCREEN) / float(this->doc->getDocH());
    SDL_SetRenderDrawBlendMode(OmniBook::renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(OmniBook::renderer, 100, 100, 100, 100);
    SDL_RenderFillRect(OmniBook::renderer, &this->scrollBar_rect);
}

void DocumentView::displaySelection(){
    if(this->showSelection){
        SDL_SetRenderDrawBlendMode(OmniBook::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(OmniBook::renderer, 0, 120, 255, 100); // Blu trasparente
        SDL_RenderFillRect(OmniBook::renderer, &this->selection_rect);
    }
}

void DocumentView::displayTraslation(){
    if(this->showTraslation){
        
        SDL_SetRenderDrawBlendMode(OmniBook::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(OmniBook::renderer, 0, 0, 0, 150); // Nero al 60% di opacità
        SDL_RenderFillRect(OmniBook::renderer, NULL); // Rettangolo pieno su tutto lo schermo

        ImGuiStyle& style = ImGui::GetStyle();
        style.FontSizeBase = 32.0f;
        if(OmniBook::NightModeON){
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f,0.1f,0.1f,1.f);
            style.Colors[ImGuiCol_Text] = ImVec4(1.f,1.f,1.f,1.f);
        }
        else{
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9f,0.9f,0.9f,1.f);
            style.Colors[ImGuiCol_Text] = ImVec4(0.f,0.f,0.f,1.f);
        }

        ImGui::SetNextWindowPos(ImVec2(W_SCREEN/2, H_SCREEN/2),ImGuiCond_Always,ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2((W_SCREEN-200),0));

        ImGui::Begin("##Traduzione",NULL,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
        
        ImGui::PushFont(OmniBook::my_font);
        ImGui::TextWrapped("%s",this->traslation.c_str());
        ImGui::PopFont();

        ImGui::End();
    }
}

void DocumentView::displayTopBottomBar(){
    if(this->showUI){

        SDL_Rect top = {0,0,W_SCREEN,40};

        OmniBook::NightModeON ? SDL_SetRenderDrawColor(OmniBook::renderer, 25, 25, 25, 255) : SDL_SetRenderDrawColor(OmniBook::renderer, 230, 230, 230, 255);
        SDL_RenderFillRect(OmniBook::renderer, &top);

        top.x = 10;
        top.y = 8;
        top.w = 24;
        top.h = 24;

        OmniBook::NightModeON ? SDL_SetTextureColorMod(this->wifi_texture,255,255,255) : SDL_SetTextureColorMod(this->wifi_texture,0,0,0);
        SDL_RenderCopy(OmniBook::renderer,this->wifi_texture,NULL,&top);

        top.x = 936;
        top.w = 14;

        SDL_Rect source;
        source.y = 0;
        source.w = 14;
        source.h = 24;
        
        int perc = scePowerGetBatteryLifePercent();

        if(perc>=0 && perc<20)
            source.x = 0;
        else if(perc>=20 && perc<40)
            source.x = 14;
        else if(perc>=40 && perc<60)
            source.x = 28;
        else if(perc>=60 && perc<80)
            source.x = 42;
        else if(perc>=80 && perc<=100)
            source.x = 56;

        OmniBook::NightModeON ? SDL_SetTextureColorMod(this->battery_texture,255,255,255) : SDL_SetTextureColorMod(this->battery_texture,0,0,0);
        SDL_RenderCopy(OmniBook::renderer,this->battery_texture,&source,&top);

        ImGuiStyle& style = ImGui::GetStyle();
        style.FontSizeBase = 16.0f;
        if(OmniBook::NightModeON){
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f,0.1f,0.1f,1.f);
            style.Colors[ImGuiCol_Text] = ImVec4(1.f,1.f,1.f,1.f);
            // style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
            // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        }
        else{
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.9f,0.9f,0.9f,1.f);
            style.Colors[ImGuiCol_Text] = ImVec4(0.f,0.f,0.f,1.f);
            // style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.6f, 1.0f, 0.5f);
            // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2f, 0.6f, 1.0f, 0.8f);
        }
        
        ImGui::SetNextWindowPos(ImVec2(W_SCREEN/2, H_SCREEN-(H_bottombar/2)),ImGuiCond_Always,ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2(W_SCREEN-100,H_bottombar+30));

        ImGui::Begin("BottomBar",NULL,ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

        const char*text = fmt::format("{}/{}",this->currPage,this->doc->getTotalPages()).c_str();
        ImGui::SetCursorPos(ImVec2((W_SCREEN-100 - ImGui::CalcTextSize(text).x)/2,12));
        ImGui::Text(text);
        ImVec2 c = ImGui::GetCursorPos();
        c = ImVec2(20+c.x,10+c.y);
        ImGui::SetCursorPos(c);
        ImGui::BeginGroup();
        ImGui::Text("From:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        if(ImGui::BeginCombo("##Lang from",OmniBook::tr.lang[OmniBook::tr.idx_from].c_str(),ImGuiComboFlags_None)){
            for(int n=0;n<5;n++){
                const bool is_selected = (n == OmniBook::tr.idx_from);
                if(ImGui::Selectable(OmniBook::tr.lang[n].c_str(),is_selected))
                    OmniBook::tr.idx_from = n;
            }
            ImGui::EndCombo();
        }
        
        ImGui::EndGroup();
        
        c = ImVec2((W_SCREEN-100)/2+40,c.y);
        ImGui::SetCursorPos(c);
        ImGui::BeginGroup();
        ImGui::Text("To:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200);
        if(ImGui::BeginCombo("##Lang to",OmniBook::tr.lang[OmniBook::tr.idx_to].c_str(),ImGuiComboFlags_None)){
            for(int n=0;n<5;n++){
                const bool is_selected = (n == OmniBook::tr.idx_to);
                if(ImGui::Selectable(OmniBook::tr.lang[n].c_str(),is_selected))
                    OmniBook::tr.idx_to = n;
            }
            ImGui::EndCombo();
        }
        
        ImGui::EndGroup();

        ImGui::End();
    }
}

DocumentView::~DocumentView(){
    delete this->doc;
    SDL_DestroyTexture(this->battery_texture);
    SDL_DestroyTexture(this->wifi_texture);
}