# LPG Engine

A simple C++23 game engine made with [axxegro](https://github.com/311Volt/axxegro).
It targets old hardware (OpenGL 2.1 / DX9) and is designed to power relatively unsophisticated 2D and 3D games.

## Status
WIP. The overall architecture concept is mostly complete and coding has begun.

## Goals

This engine is meant to power an unnanounced game that I'm developing. As such, it will be designed specifically to provide a sense of "uniqueness" that would not be likely to emerge from a game made using a mainstream engine. This is to be achieved by:

- employing a number of uncommon/unorthodox rendering techniques
- experimentation within the basic ideas of game engine design

These ideas were born from the observation that video games - especially low-budget ones - are generally shaped by technical limitations as much as anything else.

LPG Engine also aims to:

- achieve high efficiency, in particular low overhead for simpler scenes
- utilize the full power of latest C++ features to address a number of annoyances and challenges usually encountered while making games with existing engines
- be a vehicle for design exploration involving an all-out abuse of any and all static reflection hacks possible
- provide automatic data layout optimization systems to enable some of the benefits associated with data-oriented programming while at the same time allowing programmers to think in a more traditional way

All of the above is also, to some extent, an excuse to develop my own game engine (game engine development is one hell of a drug)