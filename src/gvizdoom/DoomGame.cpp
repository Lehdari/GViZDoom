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

#include <utility>


using namespace gvizdoom;


void DoomGame::init(GameConfig gameConfig)
{
    _gameConfig = std::move(gameConfig);
}

void DoomGame::update(const Action& action)
{

}
