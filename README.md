# Dungeon Map Generator

## Overview
This project is a C-based dungeon map generator that creates a grid-based dungeon layout with rooms, corridors, and stair placements. The program generates a random dungeon layout, ensuring connectivity between rooms while adhering to spatial constraints.

## Features
- Generates a grid-based dungeon map with walls and open spaces.
- Randomly places rooms of varying sizes within the dungeon.
- Ensures connectivity between rooms using a pathing algorithm.
- Adds staircases (`<` for up and `>` for down) to facilitate multi-level navigation.
- Includes linked list structures for handling room edges efficiently.

## File Structure
- `mapObj`: Defines the grid-based map structure.
- `loc`: Represents a coordinate pair `(x, y)`.
- `mapEdges`: Stores information about room edges and connectivity.
- `Node`: Defines a linked list structure for managing edges.
- `generate()`: Main function responsible for generating the dungeon.

## Configuration
- The dungeon size is set in the `generate()` function, currently `80x21`.
- You can modify the number of rooms and the minimum/maximum size by adjusting the constraints in `generate()`.

## Dependencies
- Standard C libraries (`stdio.h`, `stdlib.h`, `stdbool.h`, `string.h`, `time.h`, `ctype.h`).
## Example Output
```
----------------------------------------------------------------------------------
|                                                                                |
|  .....      .....      .....                                                  |
|  .....      .....      .....                                                  |
|  .....      .....      .....                                                  |
|  .....######.....######.....                                                  |
|      #                    #                                                   |
|      #                    #                                                   |
|  .....######.....######.....                                                  |
|  .....      .....      .....                                                  |
|  .....      .....      .....                                                  |
|  .....      .....      .....                                                  |
|                                                                                |
----------------------------------------------------------------------------------
```
## Future Enhancements
- Implement additional room shapes and obstacles.
- Introduce special room types such as treasure rooms and monster lairs.
- Improve corridor generation for more natural layouts.

## License
This project is open-source and can be modified freely.

