# DSA Mini Project: Maze Generation & Shortest Path Finding

## Project Overview

This is a 3rd semester mini project for **COMP 201** that demonstrates the practical application of fundamental Data Structures and Algorithms (DSA) concepts. The project implements a complete maze generation system using **Depth-First Search (DFS)** and then solves it using **Dijkstra's Algorithm** to find the shortest path from start to exit.

## Contributors

- **Sangam Poudel**
- **Mahesh Panta**
- **Asmit Panthi**

## Project Description

### Objectives

1. **Maze Generation**: Uses DFS algorithm to create a random, solvable maze
2. **Shortest Path Finding**: Applies Dijkstra's algorithm to find the optimal path from start to finish
3. **Visualization**: Displays the maze generation process and the solution path
4. **Learning**: Demonstrates practical implementation of two core graph algorithms

### Algorithms Used

#### 1. Depth-First Search (DFS) for Maze Generation
- **Time Complexity**: O(rows × cols)
- **Space Complexity**: O(rows × cols) for the recursion stack
- Creates a perfect maze with guaranteed solution
- Ensures every cell is reachable from every other cell

#### 2. Dijkstra's Algorithm for Shortest Path
- **Time Complexity**: O((V + E) log V) with priority queue
- **Space Complexity**: O(V) for distance and visited arrays
- Finds the shortest path in the generated maze
- Handles weighted edges if applicable

## Features

- ✅ Procedurally generated mazes using DFS
- ✅ Visual representation of the maze
- ✅ Shortest path calculation using Dijkstra's algorithm
- ✅ Step-by-step visualization of algorithm execution
- ✅ Configurable maze dimensions
- ✅ Random maze generation for each run

## Technologies & Tools

- **Language**: C++
- **Compilation**: Standard C++ compiler (g++, clang, MSVC)
- **Graphics** (if applicable): OpenGL / SDL2 / or text-based visualization
- **IDE**: Visual Studio Code





## Algorithm Explanation

### Maze Generation (DFS)

1. Start from a random cell
2. Mark current cell as visited
3. Find all unvisited neighboring cells
4. Randomly pick one and recursively visit it
5. Backtrack when no unvisited neighbors exist
6. Continue until all cells are visited


![path](https://github.com/user-attachments/assets/3d83b95a-f17b-4082-a08e-04d4d43aaebe)


### Shortest Path (Dijkstra's)

1. Initialize distances to all cells as infinity (except start = 0)
2. Use a priority queue to always process the nearest unvisited cell
3. Update distances of neighboring cells
4. Continue until destination is reached or all reachable cells are processed
5. Reconstruct path by backtracking from destination to start

![alt text](path.jpeg)

## Sample Output

The program will display:
- Generated maze with walls and paths
- Starting point (S) and destination (D)
- Shortest path highlighted through the maze
- Distance/step count to reach the destination

## Usage Instructions

1. Run the program
2. A maze of configurable size will be generated
3. The DFS algorithm creates the maze structure
4. Dijkstra's algorithm finds and displays the shortest path
5. View the results showing the optimal route

## Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|-----------|-----------------|------------------|
| Maze Generation (DFS) | O(n·m) | O(n·m) |
| Dijkstra's Algorithm | O((V+E) log V) | O(V) |
| Overall | O(n·m log(n·m)) | O(n·m) |

*where n = rows, m = columns, V = vertices, E = edges*

## Learning Outcomes

- Understanding of DFS and its applications in maze generation
- Practical implementation of Dijkstra's shortest path algorithm
- Graph traversal and pathfinding techniques
- Recursion and backtracking concepts
- Data structure usage (stacks, queues, priority queues)

## References

- Introduction to Algorithms (CLRS)
- Data Structures and Algorithm Analysis in C++
- Dijkstra's Algorithm: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
- Depth-First Search: https://en.wikipedia.org/wiki/Depth-first_search

## Course Information

- **Course Code**: COMP 202
- **Semester**: 3rd Semester
- **Project Type**: Mini Project
- **Institution**: Kathmandu University

## Notes

- The maze is guaranteed to be solvable
- All cells in the maze are reachable
- The solution provided is optimal (shortest)
- Random seed can be modified to generate different mazes

---
