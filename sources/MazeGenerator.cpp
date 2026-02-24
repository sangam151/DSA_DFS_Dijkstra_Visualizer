#include "PathAlgorithm.h"
#include <QtConcurrent>
#include <QFuture>
#include <iostream>
#include <stack>
#include <chrono>
#include <thread>
#include <QDebug>
#include <QThread>

// Recursive Backtracker maze generation
void PathAlgorithm::performRecursiveBackTrackerAlgorithm(QPromise<int>& promise)
{
    qDebug() << "Maze (Recursive Backtracker): started in thread" << QThread::currentThreadId();
    promise.suspendIfRequested();
    if (promise.isCanceled()) {
        emit pathfindingSearchCompleted(0, 0);
        return;
    }

    // Initialize all cells as obstacles (except start/end)
    for (int idx = 0; idx < widthGrid * heightGrid; ++idx) {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            emit pathfindingSearchCompleted(0, 0);
            return;
        }
        if (idx != gridNodes.startIndex && idx != gridNodes.endIndex) {
            gridNodes.Nodes[idx].obstacle = true;
            emit updatedScatterGridView(FREETOOBSTACLE, idx);
        }
    }

    std::stack<Node*> stack;
    // Random starting cell for maze generation
    int startMazeIdx = rand() % (widthGrid * heightGrid);
    Node* startMaze = &gridNodes.Nodes[startMazeIdx];
    stack.push(startMaze);
    startMaze->visited = true;
    if (startMaze->obstacle) {
        startMaze->obstacle = false;
        emit updatedScatterGridView(OBSTACLETOFREE, startMazeIdx);
    }

    int visitedCount = 1;
    const int step = 2; // carve paths moving 2 cells

    while (visitedCount < widthGrid * heightGrid && !stack.empty()) {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            emit pathfindingSearchCompleted(0, 0);
            return;
        }

        Node* cur = stack.top();
        std::vector<int> possibleDirs; // 0:E,1:S,2:W,3:N

        // Check two-step neighbors
        // East
        if (cur->xCoord + step <= widthGrid &&
            !gridNodes.Nodes[coordToIndex(cur->xCoord + step, cur->yCoord, widthGrid)].visited) {
            possibleDirs.push_back(0);
        }
        // South (y decreases)
        if (cur->yCoord - step >= 1 &&
            !gridNodes.Nodes[coordToIndex(cur->xCoord, cur->yCoord - step, widthGrid)].visited) {
            possibleDirs.push_back(1);
        }
        // West
        if (cur->xCoord - step >= 1 &&
            !gridNodes.Nodes[coordToIndex(cur->xCoord - step, cur->yCoord, widthGrid)].visited) {
            possibleDirs.push_back(2);
        }
        // North (y increases)
        if (cur->yCoord + step <= heightGrid &&
            !gridNodes.Nodes[coordToIndex(cur->xCoord, cur->yCoord + step, widthGrid)].visited) {
            possibleDirs.push_back(3);
        }

        if (!possibleDirs.empty()) {
            int dir = possibleDirs[rand() % possibleDirs.size()];
            int nx = cur->xCoord, ny = cur->yCoord;
            int pathX = cur->xCoord, pathY = cur->yCoord;

            switch (dir) {
            case 0: // East
                nx = cur->xCoord + step;
                pathX = cur->xCoord + step - 1;
                break;
            case 1: // South
                ny = cur->yCoord - step;
                pathY = cur->yCoord - step + 1;
                break;
            case 2: // West
                nx = cur->xCoord - step;
                pathX = cur->xCoord - step + 1;
                break;
            case 3: // North
                ny = cur->yCoord + step;
                pathY = cur->yCoord + step - 1;
                break;
            }

            // Carve the path cell
            int pathIdx = coordToIndex(pathX, pathY, widthGrid);
            gridNodes.Nodes[pathIdx].obstacle = false;
            emit updatedScatterGridView(OBSTACLETOFREE, pathIdx);

            // Carve the destination cell
            int destIdx = coordToIndex(nx, ny, widthGrid);
            Node* dest = &gridNodes.Nodes[destIdx];
            dest->obstacle = false;
            dest->visited = true;
            emit updatedScatterGridView(OBSTACLETOFREE, destIdx);

            stack.push(dest);
            visitedCount++;
        } else {
            stack.pop();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(speedVisualization));
    }

    // Reset visited flags for future pathfinding
    for (Node& node : gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }

    emit algorithmCompleted();
    emit pathfindingSearchCompleted(0, 0); // maze generation: nodesVisited=0, pathLength=0
}
