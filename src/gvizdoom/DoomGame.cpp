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


using namespace gvizdoom;


DoomGame::DoomGame() :
    _status (-1)
{
}

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
    GameMain_Cleanup();
}

void DoomGame::init(GameConfig&& gameConfig)
{
    gzdoom_main_init(gameConfig.argc, gameConfig.argv);
    _status = GameMain_Init(_state);
    if (_status != 0) {
        GameMain_Cleanup();
        return;
    }

    try {
        D_DoomMain_Internal_ReInit(_state);
        _status = 0;
    }
    catch (const CExitEvent& exit)
    {
        _status = exit.Reason();
        return;
    }
    catch (const std::exception& error) {
        I_ShowFatalError(error.what());
        _status = -1;
        return;
    }
}

void DoomGame::restart()
{
    // TODO does this do anything? should the whole function be removed?
    try {
        D_DoomMain_Internal_Cleanup();
        D_DoomMain_Internal_ReInit(_state);
    }
    catch (const CExitEvent& exit)
    {
        _status = exit.Reason();
    }
    catch (const std::exception& error) {
        D_DoomMain_Internal_Cleanup();
        I_ShowFatalError(error.what());
        _status = -1;
    }
}

bool DoomGame::update(const Action& action)
{
    try {
        // run one cycle
        DoomLoopCycle(_state);
    }
    catch (const CExitEvent& exit) // This is a regular exit initiated from deeply nested code.
    {
        _status = exit.Reason();
        return true;
    }
    catch (const std::exception& error) {
        I_ShowFatalError(error.what());
        _status = -1;
    }

    return false;
}

int DoomGame::getStatus() const
{
    return _status;
}
