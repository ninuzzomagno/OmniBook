#include"utils.h"

LoadingScreen::LoadingScreen(int n){
    this->max = n;
    this->curr = 0;
}

void LoadingScreen::update(){
    this->curr++;
}

void LoadingScreen::render(){

    OmniBook::NightModeON ? SDL_SetRenderDrawColor(OmniBook::renderer,0,0,0,255) : SDL_SetRenderDrawColor(OmniBook::renderer,255,255,255,255);
    SDL_RenderClear(OmniBook::renderer);

    SDL_Rect rect;

    rect.x = 80;
    rect.y = 440;
    rect.w = 800;
    rect.h = 40;

    SDL_SetRenderDrawColor(OmniBook::renderer, 100, 100, 100, 255); 
    SDL_RenderFillRect(OmniBook::renderer, &rect);

    rect.w = 800 * this->curr / this->max;

    SDL_SetRenderDrawColor(OmniBook::renderer, 0, 120, 255, 255);
    SDL_RenderFillRect(OmniBook::renderer, &rect);

    SDL_RenderPresent(OmniBook::renderer);
}

LoadingScreen::~LoadingScreen(){}