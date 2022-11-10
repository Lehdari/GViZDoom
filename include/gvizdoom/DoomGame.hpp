//
// Project: GViZDoom
// File: DoomGame.hpp
//
// Copyright (c) 2022 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "gvizdoom/Action.hpp"
#include "gvizdoom/GameConfig.hpp"
#include "d_main.h"


namespace gvizdoom {

// DoomGame serves as the top-level interface for the GViZDoom library
class DoomGame {
public:
    DoomGame();
    DoomGame(const DoomGame&) = delete;
    DoomGame(DoomGame&&);
    DoomGame& operator=(const DoomGame&) = delete;
    DoomGame& operator=(DoomGame&&);
    ~DoomGame();

    void init(GameConfig&& gameConfig);
    void restart(); // TODO is this necessary? Does this do anything?
    bool update(const Action& action); // returns true upon exit
    int getStatus() const;

    // TODO getter(s) for updated game state

private:
    GameConfig                  _gameConfig;
    int                         _status;
    D_DoomMain_Internal_State   _state;
};

} // namespace gvizdoom
