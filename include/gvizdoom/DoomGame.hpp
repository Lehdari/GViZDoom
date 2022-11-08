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


namespace gvizdoom {

class DoomGame {
public:
    void init();
    void update(const Action& action);

private:

};

} // namespace gvizdoom
