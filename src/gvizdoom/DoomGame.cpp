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
    // TODO should this be moved into deinit function or something because of potential exceptions?
#if 0
    if (_status == 0) {
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

    // "GameMain_Loop":
    try {
        // Old D_DoomMain_Internal
        D_DoomMain_Internal_ReInit(_state);
        _status = 0;
    }
    catch (const CExitEvent& exit)    // This is a regular exit initiated from deeply nested code.
    {
        _status = exit.Reason();
        return;
    }
    catch (const std::exception& error) {
        I_ShowFatalError(error.what());
        _status = -1;
        return;
    }

    while (true) {
        try {
            DoomLoopCycle(_state.lasttic);
        }
        catch (const CExitEvent& exit)    // This is a regular exit initiated from deeply nested code.
        {
            _status = exit.Reason();
            break;
        }
        catch (const std::exception& error) {
            I_ShowFatalError(error.what());
            _status = -1;
            return;
        }
    }

    try {
        D_DoomMain_Internal_Cleanup();
        _status = 0;
    }
    catch (const CExitEvent& exit)    // This is a regular exit initiated from deeply nested code.
    {
        _status = exit.Reason();
        return;
    }
    catch (const std::exception& error) {
        I_ShowFatalError(error.what());
        _status = -1;
        return;
    }

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
#if 0
    try {
        DoomLoopCycle(_state.lasttic);
    }
    catch (const CExitEvent& exit)    // This is a regular exit initiated from deeply nested code.
    {
        _status = exit.Reason();
        return true;
    }
    catch (const std::exception& error) {
        I_ShowFatalError(error.what());
        _status = -1;
    }

    return false;
#endif
    return true; // TODO just quit for now
}
