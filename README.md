# TTYD Plane Flight Optimizer
Given a ramdump at the start of a plane flight, attempt to find an optimal set of inputs which will reach the destination as quickly as possible.

## About
One of the most uncertain areas in terms of optimization is plane flights in Paper Mario: The Thousand-Year Door. Such theories as flying along a [brachistochrone](https://en.wikipedia.org/wiki/Brachistochrone_curve) have been presented, but with limited understanding of the physics engine, they were left without any certainty to back them up. This project aims to determine the optimal flight path (and sequence of inputs to achieve that path) via a [genetic algorithm (GA)](https://en.wikipedia.org/wiki/Genetic_algorithm) using the [GAUL](http://gaul.sourceforge.net/intro.html) library. This relies upon a decompilation of the game's flyMain function, which is ultimately responsible for the plane physics calculations.

The genetic algorithm, in brief, will construct a pool of (random) possible controller input strings (the population, in GA lingo). 
Then, we will run the reverse-engineered physics simulation on each of the initial input strings. 
Based on the results of the simulation, we measure how long each one of those input strings takes to get to the target. 
Those input strings that perform well are then mutated and crossed to produce new inputs based on known-good inputs, and these new candidates are used to build the next pool of possible input strings. 
This process is repeated until the algorithm has devised an optimal input string to quickly fly to the target. 

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
2. Run the following command using a Python version >= 3.7:
   ```
   ./export_player.py --ram A.raw --landingX B --landingY C --platformX D \
   --interferenceX1 E --interferenceX2 F --interferenceY G > playerdats/H.dat
   ```
   For the above command:
   - A is the filename of your RAM dump from step 1
   - B and C are the respective x and y float coordinate values of your desired landing position (can be approximate)
   - D is the x-axis edge of the takeoff platform
   - E, F, and G are only required when there is an interfering piece of collision in the middle of your flight, in which case E and F are x-axis coordinate values and G is a y-axis coordinate value, such that you are considered to be colliding with the interfering collision if you are between E and F and below G
   - H is the filename that you would like to use to represent the scenario provided from step 1
3. Edit the Makefile to add another optimization at the bottom. Copy over one of the existing optimization runs (the ones that have ga\_main as a dependency) and change it to read in the appropriate player.dat file. Also change the name of the output file to something informative. 
4. `make <<condition>>` to spin up the simulation! It will absolutely work the first time, no segfaults or linker errors. 
    5(a). Debug the makefile because it didn't work. If you have installed gaul somewhere unusual (like in your home directory), make sure you edit `GAUL_BASE` to point to this location. 
5. To view the progress, use the included `plotResults.py` script. (Instructions will pop up when you `./plotResults.py` on the console.)


## Constraints and TODO
- This program runs on Linux. MMTrebuchet wishes you luck if you want to cross-compile it for your OS of choice.
