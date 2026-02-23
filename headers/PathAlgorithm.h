#ifndef PATHALGORITHM_H
#define PATHALGORITHM_H
#include <QObject>
#include <QDebug>

#include <QtConcurrent>
#include <QFuture>
#include "GridView.h"
#include <queue> // for std::priority_queue
#include <cmath> // For fabsf

// For Dijkstra: orders by localGoal (min-heap)
struct CompareNodesDijkstra {
    bool operator()(Node* a, Node* b) {
        // For a min-heap, return true if 'a' has lower priority than 'b' (i.e., a's goal is greater than b's)
        return a->localGoal > b->localGoal;
    }
};

// For AStar: orders by globalGoal (min-heap)
struct CompareNodesAStar {
    bool operator()(Node* a, Node* b) {
        // For a min-heap, return true if 'a' has lower priority than 'b' (i.e., a's goal is greater than b's)
        return a->globalGoal > b->globalGoal;
    }
};

//Build walls list between adjacent odd-indexed cells
struct Wall {
    int wallIndex;
    int cell1Index;
    int cell2Index;
};
bool unionSet(int a ,int b,std::vector<int> &parent,std::vector<int>& rank);
int findSet(int x,std::vector<int>& parent);
class PathAlgorithm : public QObject
{

    Q_OBJECT
public:

    //Constructor
    explicit PathAlgorithm(QObject* parent = nullptr);

    //Destructor
    virtual ~PathAlgorithm();

    //Getters/Setters: current Algorithm from gridView
    ALGOS getCurrentAlgorithm() const;
    void setCurrentAlgorithm(ALGOS algorithm);

    //Getters/Setters: speed visualization
    void setSpeedVizualization(int speed);

    // Getters/Setters: Simulation on going
    void setSimulationOnGoing(bool onGoing);

    // Running pausing and canceling algorithms
    void runAlgorithm(ALGOS algorithm);
    void pauseAlgorithm();
    void resumeAlgorithm();
    void stopAlgorithm();

    // Path planning Algorithms
    void performDijkstraAlgorithm(QPromise<int>& promise);
    

  

    // Retrieving the neighbors of a point in a grid
    std::vector<Node> retrieveNeighborsGrid(const grid& gridNodes, const Node& currentNode, int widthGrid, int heightGrid);
    void FillNeighboursNode(Node& node);

    void checkGridNode(grid gridNodes, int heightGrid, int widthGrid);
    public: Q_SIGNALS:
    void updatedScatterGridView (UPDATETYPES VISIT,     int currentIndex);
    void updatedLineGridView    (QPointF currentPoint,  bool addingPoint,   bool clearPriorToUpdate=false);

    void algorithmCompleted(); // Emitted after visualization (e.g., for maze)
    void pathfindingSearchCompleted(int nodesVisited, int pathLength); // <--- NEW SIGNAL: Emitted when path is found, before visualization
private:
    // Helper functions
    void addFrontierCells(Node* node, std::vector<Node*>& frontier);
    std::vector<Node*> getMazeNeighbors(Node* node);
    void connectNodes(Node* a, Node* b);
public:

    ALGOS currentAlgorithm;
    bool running;
    bool simulationOnGoing;
    bool endReached;
    int speedVisualization;

    // grid nodes manipulated by the path planning object
    grid gridNodes;
    int heightGrid;
    int widthGrid;

    // Multithreading
    QThreadPool pool;
    QFuture<int> futureOutput;
public:
    QString algorithmToString(ALGOS algo);
public:
    //  new method to update to grids:
    void setGridNodes(const grid& newGridNodes, int width, int height);
};

#endif // PATHALGORITHM_H