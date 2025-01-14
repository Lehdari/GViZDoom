#include <gtest/gtest.h>
#include <gvizdoom/DoomGame.hpp>

#include "Action.hpp"
#include "TestUtils.hpp"

#include <gvizdoom/GameState.hpp>


using namespace gvizdoom;


TEST(TestActions, AllActions)
{
    auto& doomGame = DoomGame::instance();
    GameConfig gameConfig{0, nullptr, false, true, 640, 480, true, GameConfig::HUD_STATUSBAR, 2, true, 3, 1, 4};
    doomGame.init(gameConfig);

    std::vector<Action> actions;
    
    for (size_t i = 0; i < 50; ++i)
        actions.emplace_back(Action::Key::ACTION_ATTACK, 0);

    for (size_t i = 0; i < 50; ++i)
        actions.emplace_back(static_cast<int>(Action::Key::ACTION_FORWARD | Action::Key::ACTION_ATTACK), 100);

    for (size_t i = 0; i < 50; ++i)
        actions.emplace_back(Action::Key::ACTION_FORWARD, 0);


    for (const auto& a : actions)
        doomGame.update(a);
    
    ASSERT_EQ(doomGame.getGameState<GameState::LevelFinished>(), false);
    ASSERT_EQ(doomGame.getGameState<GameState::NumberOfKills>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::Health>(), 100);
    ASSERT_EQ(doomGame.getGameState<GameState::Armor>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::AttackDown>(), false);
    ASSERT_EQ(doomGame.getGameState<GameState::UseDown>(), false);
    ASSERT_EQ(doomGame.getGameState<GameState::WeaponState>(), 143U);
    ASSERT_EQ(doomGame.getGameState<GameState::ItemCount>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::SecretCount>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::DamageCount>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::BonusCount>(), 0);
    ASSERT_EQ(doomGame.getGameState<GameState::OnGround>(), true);
    ASSERT_GE(doomGame.getGameState<GameState::PlayerAngle>(), 120.00);
    ASSERT_LE(doomGame.getGameState<GameState::PlayerAngle>(), 120.99);
    ASSERT_GE(doomGame.getGameState<GameState::PlayerPos>()(0), -943.999);
    ASSERT_LE(doomGame.getGameState<GameState::PlayerPos>()(0), -943.0);
    ASSERT_GE(doomGame.getGameState<GameState::PlayerPos>()(1), 1600.89);
    ASSERT_LE(doomGame.getGameState<GameState::PlayerPos>()(1), 1600.91);
    ASSERT_GE(doomGame.getGameState<GameState::PlayerPos>()(2), -0.001);
    ASSERT_LE(doomGame.getGameState<GameState::PlayerPos>()(2), 0.001);
    ASSERT_FALSE(doomGame.getGameState<GameState::PlayerDead>());
}