TODO
====

- Interface refactor
  - Create unified top-level interface
  - Set action -> update -> get state -> repeat
  - Considerations
    - Will OpenGL / Vulkan stuff be required to be removed?
    - How to handle the events? Wrap raw keyboard / mouse actions? Or add custom abstraction?
- State variables
  - Depth buffer
  - Semantic label buffer (floor, ceiling, walls, objects, items, enemies)
  - Automap
  - Visible items enumeration
