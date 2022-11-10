//
// Project: GViZDoom
// File: DoomGame.cpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "gvizdoom/DoomGame.hpp"
#include "gvizdoom/gzdoom_main_wrapper.hpp"
#include "engineerrors.h"

#include <utility>


using namespace gvizdoom;


DoomGame::DoomGame(DoomGame&& other) :
    _gameConfig (other._gameConfig),
    _status     (other._status),
    _state      (other._state)
{
}

DoomGame& DoomGame::operator=(DoomGame&& other)
{
    _gameConfig = other._gameConfig;
    _status     = other._status;
    _state      = other._state;
    return *this;
}

DoomGame::~DoomGame()
{
#if 0 // TODO
    if (_status == 0) { // if status is not 0, everything has already been cleaned up
        D_DoomMain_Internal_Cleanup();
        GameMain_Cleanup();
    }
#endif
}

void DoomGame::init(GameConfig&& gameConfig)
{
    gzdoom_main_init(gameConfig.argc, gameConfig.argv);
    _status = GameMain_Init(_state);
    if (_status != 0) {
        GameMain_Cleanup();
        return;
    }

    _status = GameMain_Loop(_state); // TODO obviously move game loop out of init :D
    GameMain_Cleanup();
}

void DoomGame::restart()
{
#if 0 // TODO
    try {
        D_DoomMain_Internal_Cleanup();
        D_DoomMain_Internal_ReInit(_state);
    }
    catch (const CExitEvent& exit)    // This is a regular exit initiated from deeply nested code.
    {
        _status = exit.Reason();
    }
    catch (const std::exception& error) {
        D_DoomMain_Internal_Cleanup();
        I_ShowFatalError(error.what());
        _status = -1;
    }

    if (_status != 0)
        GameMain_Cleanup();
#endif
}

bool DoomGame::update(const Action& action)
{
    return true; // TODO just quit for now
}
