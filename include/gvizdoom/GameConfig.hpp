//
// Project: GViZDoom
// File: GameConfig.hpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <vector>
#include <string>


namespace gvizdoom {

// GameConfig class encapsulates all information required for DoomGame initialization
struct GameConfig {
    int     argc            {0}; // TODO remove raw CLI parameters, use high-level abstraction
    char**  argv            {nullptr};
    bool    interactive     {false};
    bool    singletics      {true};

    // Video parameters
    int     videoWidth      {640};
    int     videoHeight     {480};
    int     videoTrueColor  {true};

    // HUD parameters
    enum HUDType : uint32_t {
        HUD_STATUSBAR = 0,      // original Doom HUD with mugshot and grey bar
        HUD_FLOATING = 1,       // more minimal HUD with floating icons and ammo/health/other amounts
        HUD_ALTERNATIVE = 2,    // alternative floating HUD with more info
        HUD_DISABLED = 3        // no HUD
    }       hudType         {HUD_STATUSBAR};
    int     hudScale        {2};
    bool    renderWeapon    {true};

    // Game parameters
    int     skill           {3};
    int     episode         {1};
    int     map             {1};

    // WAD parameters
    //const std::string& iwadFileName; // TODO
    std::vector<std::string>    pwadFileNames   {};
};

} // namespace gvizdoom
