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
#include <QDebug> // Include for qDebug()
#include <QDateTime> // Include for QDateTime::currentMSecsSinceEpoch()
#include <QThread> // Include for QThread::currentThreadId()

//Constructor
PathAlgorithm::PathAlgorithm(QObject* parent): QObject (parent)
{
    // The algorithm is not running at startup
    running = false;
    simulationOnGoing = false;
    endReached = false;

    speedVisualization = 250;
    qDebug() << "PathAlgorithm: Constructor called. Main thread ID:" << QThread::currentThreadId();
}

//Destructor
PathAlgorithm::~PathAlgorithm()
{

}

//Getters/Setters: current Algorithm from gridView
ALGOS PathAlgorithm::getCurrentAlgorithm() const
{
    return currentAlgorithm;
}

//Getters/Setters: current Algorithm from gridView
void PathAlgorithm::setCurrentAlgorithm(ALGOS algorithm)
{
    this->currentAlgorithm = algorithm;
}

void PathAlgorithm::setSpeedVizualization(int speed)
{
    this->speedVisualization = speed;
}

// Getters/Setters: Simulation on going
void PathAlgorithm::setSimulationOnGoing(bool onGoing)
{
    this->simulationOnGoing = onGoing;
}
// Implement the new method:
void PathAlgorithm::setGridNodes(const grid& newGridNodes, int width, int height)
{
    this->gridNodes = newGridNodes; // Copies the entire grid struct, including the Nodes vector and start/end indices
    this->widthGrid = width;        // Update PathAlgorithm's internal width
    this->heightGrid = height;      // Update PathAlgorithm's internal height
    qDebug() << "PathAlgorithm: Internal grid dimensions updated to " << this->widthGrid << "x" << this->heightGrid;
    qDebug() << "PathAlgorithm: Internal gridNodes.Nodes.size() is now: " << this->gridNodes.Nodes.size();
    qDebug() << "PathAlgorithm: Internal startIndex: " << this->gridNodes.startIndex << ", endIndex: " << this->gridNodes.endIndex;

    // IMPORTANT: Also reset the visited/nextUp flags on the new grid, just in case
    for(Node& node: this->gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }
}

std::vector<Node> PathAlgorithm::retrieveNeighborsGrid(const grid& gridNodes, const Node& currentNode, int widthGrid, int heightGrid)
{

    std::vector<Node> neighbors;

    // right: adding +1 to x:
    if (currentNode.xCoord + 1 <= widthGrid)
    {
        int rightIndex = coordToIndex(currentNode.xCoord + 1, currentNode.yCoord, widthGrid);
        neighbors.push_back(gridNodes.Nodes[rightIndex]);

    }

    // down: adding -1 to y:
    if (currentNode.yCoord - 1 >= 1)
    {
        int downIndex = coordToIndex(currentNode.xCoord, currentNode.yCoord -1, widthGrid);
        neighbors.push_back(gridNodes.Nodes[downIndex]);
    }

    // left: adding -1 to x:
    if (currentNode.xCoord - 1 >= 1)
    {
        int leftIndex = coordToIndex(currentNode.xCoord - 1, currentNode.yCoord, widthGrid);
        neighbors.push_back(gridNodes.Nodes[leftIndex]);
    }

    // up: adding +1 to y:
    if (currentNode.yCoord + 1 <= heightGrid)
    {
        int upIndex = coordToIndex(currentNode.xCoord, currentNode.yCoord + 1, widthGrid);
        neighbors.push_back(gridNodes.Nodes[upIndex]);
    }

    return neighbors;
}

QString PathAlgorithm::algorithmToString(ALGOS algo) {
    switch (algo) {
    case BFS: return "BFS";
    case DFS: return "DFS";
    case DIJKSTRA: return "DIJKSTRA";
    case ASTAR: return "ASTAR";
    case BACKTRACK: return "BACKTRACK";
    case NOALGO: return "NOALGO";
    default: return "UNKNOWN_ALGO";
    }
}
void PathAlgorithm::checkGridNode(grid gridNodes, int heightGrid, int widthGrid)
{
    // Display grid
    std::cerr << "State of grid node \n";
    int countVisited = 0; int countObstacle = 0; int countFree = 0;

    for (Node node: gridNodes.Nodes)
    {
        std::cerr << "(" << node.xCoord << ", " <<  node.yCoord << "): ";

        if (node.visited){std::cerr << ": V"; countVisited++;}

        if (node.obstacle){std::cerr << ": O"; countObstacle++;}
        else{std::cerr << ": F"; countFree++;}

        if (node.xCoord == widthGrid){std::cerr << " \n";}
        else{std::cerr << " | ";}

    }
    std::cerr << "Totals: " << "Visited: " << countVisited
              << " - Obstacles: " << countObstacle
              << " - Free:" << countFree << "\n";

    // Check size of vector
    if (static_cast<int>(gridNodes.Nodes.size()) != static_cast<int>(heightGrid * widthGrid))
    {std::cerr << "Number of nodes in gridNodes: " << gridNodes.Nodes.size() << " vs " << heightGrid * widthGrid << " [ISSUE] \n";}
    else{std::cerr << "Number of nodes in gridNodes: " << gridNodes.Nodes.size() << "\n";}

}

void PathAlgorithm::runAlgorithm(ALGOS algorithm)
{
    simulationOnGoing=true;
    running=true;
    qDebug() << "PathAlgorithm: runAlgorithm called. Thread ID:" << QThread::currentThreadId();

    switch (algorithm) {

    case DIJKSTRA:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performDijkstraAlgorithm, this);
        break;


    case NOALGO:
        std::cerr <<"NO ALGO \n";
    default:
        break;

    }

}

void PathAlgorithm::resumeAlgorithm()
{
    running = true;
    futureOutput.resume();
    qDebug() << "PathAlgorithm: futureOutput.resume() called.";
}

void PathAlgorithm::pauseAlgorithm()
{
    running = false;
    futureOutput.suspend();
    qDebug() << "PathAlgorithm: futureOutput.suspend() called.";
}

void PathAlgorithm::stopAlgorithm()
{
    running = false;
    futureOutput.cancel();
    qDebug() << "PathAlgorithm: futureOutput.cancel() called.";
}

void PathAlgorithm::performDijkstraAlgorithm(QPromise<int>& promise)
{
    qDebug() << "Dijkstra: Algorithm started in worker thread:" << QThread::currentThreadId();
    promise.suspendIfRequested();
    if (promise.isCanceled()) {
        emit pathfindingSearchCompleted(0, 0);
        return;
    }

    // Reset node properties for a new run
    for(Node& node: gridNodes.Nodes)
    {
        node.neighbours.clear();
        FillNeighboursNode(node); // Ensure neighbors are filled
        node.localGoal      = INFINITY;
        node.parent         = nullptr;
        node.visited        = false; // Reset visited flag
    }




     auto distance = [](Node* a, Node* b)
    {
        return sqrtf(   (a->xCoord - b->xCoord) * (a->xCoord - b->xCoord)
                     +(a->yCoord - b->yCoord) * (a->yCoord - b->yCoord));
    };

    Node* nodeStart = &(gridNodes.Nodes[gridNodes.startIndex]);
    Node* nodeEnd = &(gridNodes.Nodes[gridNodes.endIndex]);

    nodeStart->localGoal = 0.0f;

    // Use std::priority_queue for Dijkstra
    std::priority_queue<Node*, std::vector<Node*>, CompareNodesDijkstra> nodesToTest;
    nodesToTest.push(nodeStart);

    int nodesVisitedCount = 0; // Counter for visited nodes

    while(!nodesToTest.empty())
    {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            qDebug() << "Dijkstra: Algorithm cancelled during loop.";
            emit pathfindingSearchCompleted(nodesVisitedCount, 0);
            return;
        }

        Node* nodeCurrent = nodesToTest.top();
        nodesToTest.pop();

        // If this node has already been visited (meaning we found a shorter path to it earlier), skip it.
        if (nodeCurrent->visited) {
            continue;
        }

        nodeCurrent->visited = true; // Mark as visited after extracting from PQ
        nodesVisitedCount++; // Increment when a node is processed (removed from open list)

        int indexCurrent = coordToIndex(nodeCurrent->xCoord, nodeCurrent->yCoord, widthGrid);
        if (indexCurrent != gridNodes.startIndex && indexCurrent != gridNodes.endIndex) {
            emit updatedScatterGridView(VISIT, indexCurrent);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(speedVisualization));

        if (nodeCurrent == nodeEnd) { // Goal reached
            break;
        }

        for (Node* nodeNeighbour: nodeCurrent->neighbours)
        {
            if(!nodeNeighbour->obstacle) // Only consider non-obstacle neighbors
            {
                float potentialLowerGoal = nodeCurrent->localGoal + distance(nodeCurrent, nodeNeighbour);
                if (potentialLowerGoal < nodeNeighbour->localGoal){
                    nodeNeighbour->parent = nodeCurrent;
                    nodeNeighbour->localGoal = potentialLowerGoal;

                    // Always push to PQ if a better path is found, even if potentially "visited" by a longer path
                    nodesToTest.push(nodeNeighbour);

                    // Only emit NEXT if it's not the end node and it hasn't been visited in a finalized path yet
                    if (!nodeNeighbour->visited && nodeNeighbour != nodeEnd) {
                        emit updatedScatterGridView(NEXT, coordToIndex(nodeNeighbour->xCoord, nodeNeighbour->yCoord, widthGrid));
                    }
                }
            }
        }
    }
}