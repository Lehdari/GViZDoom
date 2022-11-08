//
// Project: GViZDoom
// File: App.hpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <string>
#include <SDL.h>


namespace gvizdoom {

struct RenderContext {};

class App {
public:
    // Settings for the application
    struct WindowSettings {
        std::string name;
        int64_t width;
        int64_t height;
        int64_t framerateLimit;

        explicit WindowSettings(
            const std::string& name = "GViZDoom",
            int64_t width = 640,
            int64_t height = 480,
            int64_t framerateLimit = 60) :
            name(name),
            width(width),
            height(height),
            framerateLimit(framerateLimit)
        {}
    };

    // Struct for App context to be passed to pipeline functions
    struct Context {
        WindowSettings* windowSettings;
        SDL_Window*     window;
        SDL_GLContext*  glContext;
        bool*           quit;

        Context(App& app) :
            windowSettings  (&app._settings.window),
            window          (app._window),
            glContext       (&app._glCtx),
            quit            (&app._quit)
        {}
    };

    struct Settings {
        WindowSettings window;

        // Pipeline function pointers for event handling and rendering
        void (*handleEvents)(SDL_Event& event, Context& appContext);
        void (*render)(RenderContext& context, Context& appContext);

        explicit Settings(
            const WindowSettings& window                            = WindowSettings(),
            void (*handleEvents)(SDL_Event&, Context& appContext)   = nullptr,
            void (*render)(RenderContext&, Context& appContext)     = nullptr
        ) :
            window          (window),
            handleEvents    (handleEvents),
            render          (render)
        {}
    };

    explicit App(
        const Settings& settings = Settings(),
        RenderContext* renderContext = nullptr);

    ~App();

    void loop(void);

    void setRenderContext(RenderContext* renderContext);

private:
    Settings            _settings;
    SDL_Window*         _window;
    SDL_GLContext       _glCtx;
    bool                _quit; // flag for quitting the application
    uint32_t            _lastTicks;
    uint32_t            _frameTicks;

    App::Context        _appContext;
    RenderContext*      _renderContext;
};

} // namespace gvizdoom
