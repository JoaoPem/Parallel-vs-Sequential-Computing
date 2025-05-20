# Breadth-First Search (BFS) Algorithm

## Overview

Breadth-First Search (BFS) is a classic graph traversal algorithm that explores all reachable vertices from a given starting vertex layer by layer.

The algorithm visits all vertices at a distance of one edge from the start vertex first, then all vertices at a distance of two edges, and so on.

This makes BFS especially useful for finding the shortest path (in terms of number of edges) between two vertices in unweighted graphs.

---

## How BFS Works

- Start from the source vertex and enqueue it.
- Dequeue a vertex, visit all its immediate neighbors, and enqueue them if they haven't been visited.
- For each vertex visited, keep track of the path taken to reach it.
- Repeat until the target vertex is found or all vertices reachable from the source have been explored.

By keeping track of the paths, BFS can reconstruct all possible routes between two points in the graph.

---

## Project Description

This project implements BFS on a graph where nodes represent departments of a company (e.g., HR, IT, Finance, Sales) and edges represent connections between departments.

### Two BFS Approaches

1. **Sequential BFS:**  
   Explores all possible paths from the source to the destination department sequentially, printing each complete path found.

2. **Parallel BFS:**  
   Creates a thread for each immediate neighbor of the start vertex. Each thread runs an independent BFS, potentially speeding up the search on larger graphs.

---

## Performance Comparison

The program measures and compares the execution time of both approaches to highlight the performance differences.

---

## Output

- Prints all possible paths from the start department to the end department.
- Displays execution time for the sequential BFS.
- Displays execution time for the parallel BFS.

---

## How to Run

Compile the program with `gcc` (or any compatible C compiler) and link with pthread library:

```bash
gcc -o bfs bfs.c -lpthread
./bfs
