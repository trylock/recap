# ReCap

In the game Path of Exile (PoE), it's desirable to have high enough resistances for each type of damage. There are crafting recipes which give you extra resistances but they can only be used once per equipment slot. Moreover, resistances cannot surpass a certain threshold and crafting costs in-game currency. ReCap finds an assignment of crafting recipes to equipment slots which caps your resistances and minimizes the expected cost of crafting.

## Running the tool

ReCap has a command line interface `recap_cli`. It expects path to a file with recipes and required resistances. The rest of the arguments are optional.

### Options:
- `--input` or `-i`: specifies path to a file with available recipes (you can use `data/recipes.csv` from this repository)
- `--required` or `-r`: list of required resistances in order: fire, cold, lightning, and chaos. Values are separated by spaces. If you only specify first few values, the rest of the values will be set to 0.
- `--armour` or `-a` (default 7): number of armour slots 
- `--jewelery` or `-j` (default 3): number of jewelery slots 

For example, following command finds an assignment which has at least 43% fire, 76% cold, 12% lightning and 13% chaos resistance.

`recap_cli -i ../data/recipes.csv -r 43 76 12 13` 

## Building

It requires:
- `cmake` version 3.1 or newer
- C++17 compliant compiler
- TBB 2020 (`apt install libtbb-dev` on ubuntu)
- Boost 1.71 (`apt install libboost-all-dev` on ubuntu) 

I've tested it on Ubuntu 20.10 and Windows 10 (in WSL2). 

Build:
- `git submodule update --init --recursive` (fetch the CSV library)
- `mkdir build && cd build` (create a build directory)
- `cmake ..`
- `make` (if you've used Makefiles with cmake)
