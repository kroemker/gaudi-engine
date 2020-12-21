# gaudi-engine
Gaudi engine a.k.a. Flagfish is a C++ chess engine based off [Flagfish](https://github.com/kroemker/flagfish). It contains many improvements over the old engine including principal variation search, hashing, logging, better evaluation and time management. In contrast to its ancestor it only supports the UCI protocol. You can play against it on [Lichess](https://lichess.org/@/Flagfish).

## Features
- 0x88 board representation
- principal variation with quiescence search
- move ordering
- delta pruning
- zobrist hashing
- transposition & evaluation hash tables
- UCI protocol support
- time management
- logging

## Usage
The engine can be start with the following command:
``` 
gaudi-engine [-t] [-uci]
```

- **-t** : runs a number of hardcoded tests
- **-uci** : starts the uci protocol handler
