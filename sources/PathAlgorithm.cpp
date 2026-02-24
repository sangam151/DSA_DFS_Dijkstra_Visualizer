#include "PathAlgorithm.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include <iostream>
#include <queue>
#include <map>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <QtConcurrent>
#include <QFuture>
#include <stack>
#include <list>
#include <QDebug>
#include <QThread>

// Constructor
PathAlgorithm::PathAlgorithm(QObject* parent) : QObject(parent)
{
    running = false;
    simulationOnGoing = false;
    endReached = false;
    speedVisualization = 250;
}

// Destructor
PathAlgorithm::~PathAlgorithm() {}

ALGOS PathAlgorithm::getCurrentAlgorithm() const { return currentAlgorithm; }
void PathAlgorithm::setCurrentAlgorithm(ALGOS algorithm) { currentAlgorithm = algorithm; }
void PathAlgorithm::setSpeedVizualization(int speed) { speedVisualization = speed; }
void PathAlgorithm::setSimulationOnGoing(bool onGoing) { simulationOnGoing = onGoing; }

void PathAlgorithm::setGridNodes(const grid& newGridNodes, int width, int height)
{
    gridNodes = newGridNodes;
    widthGrid = width;
    heightGrid = height;
    for (Node& node : gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }
}

std::vector<Node> PathAlgorithm::retrieveNeighborsGrid(const grid& g, const Node& currentNode, int w, int h)
{
    std::vector<Node> neighbors;
    if (currentNode.xCoord + 1 <= w) {
        int idx = coordToIndex(currentNode.xCoord + 1, currentNode.yCoord, w);
        neighbors.push_back(g.Nodes[idx]);
    }
    if (currentNode.yCoord - 1 >= 1) {
        int idx = coordToIndex(currentNode.xCoord, currentNode.yCoord - 1, w);
        neighbors.push_back(g.Nodes[idx]);
    }
    if (currentNode.xCoord - 1 >= 1) {
        int idx = coordToIndex(currentNode.xCoord - 1, currentNode.yCoord, w);
        neighbors.push_back(g.Nodes[idx]);
    }
    if (currentNode.yCoord + 1 <= h) {
        int idx = coordToIndex(currentNode.xCoord, currentNode.yCoord + 1, w);
        neighbors.push_back(g.Nodes[idx]);
    }
    return neighbors;
}

void PathAlgorithm::FillNeighboursNode(Node& node)
{
    if (node.xCoord + 1 <= widthGrid) {
        int idx = coordToIndex(node.xCoord + 1, node.yCoord, widthGrid);
        node.neighbours.push_back(&gridNodes.Nodes[idx]);
    }
    if (node.yCoord - 1 >= 1) {
        int idx = coordToIndex(node.xCoord, node.yCoord - 1, widthGrid);
        node.neighbours.push_back(&gridNodes.Nodes[idx]);
    }
    if (node.xCoord - 1 >= 1) {
        int idx = coordToIndex(node.xCoord - 1, node.yCoord, widthGrid);
        node.neighbours.push_back(&gridNodes.Nodes[idx]);
    }
    if (node.yCoord + 1 <= heightGrid) {
        int idx = coordToIndex(node.xCoord, node.yCoord + 1, widthGrid);
        node.neighbours.push_back(&gridNodes.Nodes[idx]);
    }
}

void PathAlgorithm::checkGridNode(grid g, int h, int w)
{
    Q_UNUSED(g);
    Q_UNUSED(h);
    Q_UNUSED(w);
}

void PathAlgorithm::runAlgorithm(ALGOS algorithm)
{
    simulationOnGoing = true;
    running = true;
    switch (algorithm) {
    case DIJKSTRA:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performDijkstraAlgorithm, this);
        break;
    case BACKTRACK:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performRecursiveBackTrackerAlgorithm, this);
        break;
    default:
        break;
    }
}

void PathAlgorithm::resumeAlgorithm()
{
    running = true;
    futureOutput.resume();
}

void PathAlgorithm::pauseAlgorithm()
{
    running = false;
    futureOutput.suspend();
}

void PathAlgorithm::stopAlgorithm()
{
    running = false;
    futureOutput.cancel();
}

// -----------------------------------------------------------------
// Dijkstra Implementation
// -----------------------------------------------------------------
void PathAlgorithm::performDijkstraAlgorithm(QPromise<int>& promise)
{
    qDebug() << "Dijkstra: started in thread" << QThread::currentThreadId();
    promise.suspendIfRequested();
    if (promise.isCanceled()) {
        emit pathfindingSearchCompleted(0, 0);
        return;
    }

    // Reset nodes
    for (Node& node : gridNodes.Nodes) {
        node.neighbours.clear();
        FillNeighboursNode(node);
        node.localGoal = INFINITY;
        node.parent = nullptr;
        node.visited = false;
    }

    auto distance = [](Node* a, Node* b) {
        float dx = a->xCoord - b->xCoord;
        float dy = a->yCoord - b->yCoord;
        return sqrtf(dx*dx + dy*dy);
    };

    Node* start = &gridNodes.Nodes[gridNodes.startIndex];
    Node* goal  = &gridNodes.Nodes[gridNodes.endIndex];
    start->localGoal = 0.0f;

    std::priority_queue<Node*, std::vector<Node*>, CompareNodesDijkstra> open;
    open.push(start);

    int visitedCount = 0;

    while (!open.empty()) {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            emit pathfindingSearchCompleted(visitedCount, 0);
            return;
        }

        Node* cur = open.top();
        open.pop();

        if (cur->visited) continue;
        cur->visited = true;
        visitedCount++;

        int curIdx = coordToIndex(cur->xCoord, cur->yCoord, widthGrid);
        if (curIdx != gridNodes.startIndex && curIdx != gridNodes.endIndex)
            emit updatedScatterGridView(VISIT, curIdx);

        std::this_thread::sleep_for(std::chrono::milliseconds(speedVisualization));

        if (cur == goal) break;

        for (Node* nb : cur->neighbours) {
            if (nb->obstacle) continue;
            float newCost = cur->localGoal + distance(cur, nb);
            if (newCost < nb->localGoal) {
                nb->parent = cur;
                nb->localGoal = newCost;
                open.push(nb);
                int nbIdx = coordToIndex(nb->xCoord, nb->yCoord, widthGrid);
                if (!nb->visited && nbIdx != gridNodes.endIndex)
                    emit updatedScatterGridView(NEXT, nbIdx);
            }
        }
    }

    int pathLength = 0;
    if (goal->parent != nullptr) {
        std::vector<int> path;
        Node* p = goal;
        while (p != nullptr) {
            path.insert(path.begin(), coordToIndex(p->xCoord, p->yCoord, widthGrid));
            p = p->parent;
        }
        pathLength = path.size() - 1;

        emit pathfindingSearchCompleted(visitedCount, pathLength);

        // Visualize path
        emit updatedLineGridView(QPointF(goal->xCoord, goal->yCoord), true, true);
        for (size_t i = path.size() - 1; i > 0; --i) {
            if (promise.isCanceled()) break;
            int idx = path[i];
            emit updatedScatterGridView(PATH, idx);
            emit updatedLineGridView(QPointF(gridNodes.Nodes[idx].xCoord, gridNodes.Nodes[idx].yCoord), true, false);
            std::this_thread::sleep_for(std::chrono::milliseconds(speedVisualization));
        }
        emit updatedLineGridView(QPointF(start->xCoord, start->yCoord), true, false);
    } else {
        emit pathfindingSearchCompleted(visitedCount, 0);
    }

    // Reset visited flags
    for (Node& node : gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }

    emit algorithmCompleted();
}

QString PathAlgorithm::algorithmToString(ALGOS algo)
{
    switch (algo) {
    case DIJKSTRA: return "DIJKSTRA";
    case BACKTRACK: return "BACKTRACK";
    default: return "UNKNOWN";
    }
}
