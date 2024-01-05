# Parallel Maze Solver
## Project Overview
This project is an application developed as a semester project for the course [IFT630 - Processus Concurrents et Parallélisme](https://plandecours.dinf.usherbrooke.ca/pdc/2023-2/IFT630/0/). It is designed to solve mazes using both parallel and sequential algorithms, leveraging OpenCL for parallel processing.
This application showcases the efficient exploration of large maze structures using parallel computing concepts and compares the performance of parallel and sequential methods in maze solving.

For a detailed analysis and report on this project, please refer to the included (project report)(https://github.com/yndongek/parallel-maze-solver/files/13846635/IFT630-PP2.1.pdf).

![image](https://github.com/yndongek/parallel-maze-solver/assets/155771670/1444b3c7-aaa2-463a-a843-1576470d2362)

## Features
- **Parallel Maze Solving:** Utilizes OpenCL for parallel processing, enhancing performance on compatible hardware.
- **Sequential Maze Solving:** Implements a sequential Breadth-First Search (BFS) algorithm for maze solving.
- **Performance Comparison:** Offers insights into the execution time differences between parallel and sequential solutions.
- **Visual Output:** Displays the maze, the path found, and the execution times in the console.


# How to use
## Prerequisites
- C++ Compiler (e.g., GCC)
- OpenCL SDK installed and configured on your platform

## Running the Application
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/username/parallel-maze-solver.git
2. **Navigate to the Project Directory:**
   ```bash
   cd parallel-maze-solver  
3. **Compile the Program:**
   ```bash
   g++ -o maze_solver main.cpp
4. **Run the Application:**
   ```bash
   ./maze_solver
## Input Format
The maze should be in a text file, with **'0'** for empty cells, **'1'** for walls, **'S'** as the start point, and **'D'** as the destination.
