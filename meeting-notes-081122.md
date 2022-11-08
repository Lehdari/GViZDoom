8.11.2022 Doom meet

- Introduction to transformers
- [GViZDoom](https://github.com/Lehdari/GViZDoom)

# GViZDoom

- A good high-level interface can be found [here](https://github.com/Farama-Foundation/ViZDoom/blob/master/doc/DoomGame.md)
- GVZDoom is
  - c++
  - ML research platform (especially RL)
- GVZDoom makes it easier for us to
  - have a programmable interface to DooM
  - run training and playing really fast
  - modify the engine

## Idea

- Don't use HW renderer, HW is reserved for ML
  - Render on CPU, train on GPU
- Implement useful ML features
- Develop our own ML algos on GVZDoom
- Los Pollo y huevos problemos


## TODO

- Benchmark the truecolor renderer
  - Compare it with the original sw renderer
- Make RL interface
  - Make a DoomGame class
  - Implement a good interface
  - Make a demo that uses DoomGame to:
    - init a game
    - render a screen
    - make some random actions in a doom map
    - update the game state accordingly
    - close gracefully (no crashing pls)
- Save state
  - Try several different actions from the state