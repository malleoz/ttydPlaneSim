# TTYD Plane Flight Optimizer
Given a ramdump at the start of a plane flight, attempt to find an optimal set of inputs which will reach the destination as quickly as possible.

## About
One of the most uncertain areas in terms of optimization is plane flights in Paper Mario: The Thousand-Year Door. Such theories as flying along a [brachistochrone](https://en.wikipedia.org/wiki/Brachistochrone_curve) have been presented, but with limited understanding of the physics engine, they were left without any certainty to back them up. This project aims to determine the optimal flight path (and sequence of inputs to achieve that path) via a genetic algorithm. This relies upon a decompilation of the game's flyMain function, which is ultimately responsible for the plane physics calculations.

## How to Run
1. Use Debug Dolphin Emulator to dump the game's RAM at the start of a plane flight.
   - Go to your Dolphin folder in Command Prompt and run the following: `Dolphin.exe -d`
   - Boot up the game
   - On a Z-axis edge of the plane panel (such that you can nosedive right away), breakpoint at one of the following addresses:
   ```
   NTSC_U: 0x8009cae0
   NTSC_J: 0x8009b1e8
   ```
   - Go to View and toggle Memory
   - Click `Dump MRAM`
2. Open `export_player.py` and modify `landingX` and `landingY` which correspond to the first coordinates for which you contact the landing wall and stand on the landing platform respectively.
3. TODO: Run `Some provided batch script` to export the necessary data from the RAM dump and pipe that data into the simulation program.
4. Let program run.

## Constraints and TODO
- We use the assumption that we are never above a plane panel. This assumption means that all plane panels must be activated along the Z-axis edge only in scenarios for which there is no floor past the edge. For now, decompiling collision object data is a bit extreme, as most plane flights will occur on the Z-axis edge in the TAS to build speed faster.
- To make execution a bit easier, a .bat script will be constructed to automate piping data from `export_player.py` into the simulation program.
