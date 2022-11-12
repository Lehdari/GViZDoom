//
// Project: GViZDoom
// File: App.cpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "gvizdoom_client/App.hpp"


using namespace gvizdoom;


App::App(
    const App::Settings &settings,
    RenderContext* renderContext,
    GameConfig gameConfig
) :
    _settings       (settings),
    _window         (nullptr),
    _quit           (false),
    _lastTicks      (0),
    _frameTicks     (0),
    _appContext     (*this),
    _renderContext  (renderContext)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error: Could not initialize SDL!\n");
        return;
    }

    _window = SDL_CreateWindow(
        _settings.window.name.c_str(),
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        (int)_settings.window.width,
        (int)_settings.window.height,
        SDL_WINDOW_SHOWN);
    if (_window == nullptr) {
        printf("Error: SDL Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    _doomGame = std::make_unique<DoomGame>();
    _doomGame->init(std::move(gameConfig));
}

App::~App()
{
    // Destroy window and quit SDL subsystems
    SDL_DestroyWindow(_window);
    SDL_Quit();
}

void App::loop(void)
{
    if (_doomGame->getStatus()) {
        // DoomGame uninitialized / in error state: cannot run loop
        printf("App::loop: Invalid DoomGame status: %d\n", _doomGame->getStatus());
        return;
    }

    // Application main loop
    while (!_quit) {
#if 0
        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (_settings.handleEvents != nullptr)
                _settings.handleEvents(event, _appContext);
        }
#endif

        // Update game
        _quit = _doomGame->update(Action());
        if (_quit)
            break;

        // User-defined render
        if (_renderContext != nullptr && _settings.render != nullptr)
            _settings.render(*_renderContext, _appContext);

        SDL_Delay(1000/_settings.window.framerateLimit);

        uint32_t curTicks = SDL_GetTicks();
        _frameTicks = curTicks - _lastTicks;
        _lastTicks = curTicks;
    }
}

void App::setRenderContext(RenderContext* renderContext)
{
    _renderContext = renderContext;
}
