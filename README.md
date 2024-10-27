## Fixing the README.md is on my To-Do list, so don't judge me yet

## Turn by Turn Battle game

### Description
Two player turn-by-turn game. Red starts first. Each player gets a bunch of units and can move them around the board. The goal is to kill all the other player's units. Click on a unit to see its movement range. Click on a cell to move the unit there. Click on an enemy unit to attack it. The game ends when one player has no units left. The green tiles are the movement range of the selected unit. The yellow tiles are the attack range of the selected unit. 

Tanks can move by 4 tiles, and attack within 4 tiles. Planes can move by 6 tiles, and attack within 2 tiles. Soldiers can move by 2 tiles, and attack within 1 tile. The tiles are counted in a straight line, not diagonally, and the units can't move through enemy units. Each turn you can either move a unit or attack with a unit. More than that would be too much for the small brains of the units.

### How to run
1. Clone the repository
2. make sure you have SDL2 and SDL2_image installed
3. mkdir build && cd build && cmake .. && make
4. run the Platformer_exe from the build directory