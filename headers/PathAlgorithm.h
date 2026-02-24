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
        return a->localGoal > b->localGoal;
    }
};

class PathAlgorithm : public QObject
{
    Q_OBJECT

public:
    explicit PathAlgorithm(QObject* parent = nullptr);
    virtual ~PathAlgorithm();

    ALGOS getCurrentAlgorithm() const;
    void setCurrentAlgorithm(ALGOS algorithm);
    void setSpeedVizualization(int speed);
    void setSimulationOnGoing(bool onGoing);

    void runAlgorithm(ALGOS algorithm);
    void pauseAlgorithm();
    void resumeAlgorithm();
    void stopAlgorithm();

    // Path planning – Dijkstra only
    void performDijkstraAlgorithm(QPromise<int>& promise);

    // Maze generation – Recursive Backtracker only
    void performRecursiveBackTrackerAlgorithm(QPromise<int>& promise);

    std::vector<Node> retrieveNeighborsGrid(const grid& gridNodes, const Node& currentNode, int widthGrid, int heightGrid);
    void FillNeighboursNode(Node& node);
    void checkGridNode(grid gridNodes, int heightGrid, int widthGrid);

signals:
    void updatedScatterGridView(UPDATETYPES type, int index);
    void updatedLineGridView(QPointF point, bool adding, bool clearPrior = false);
    void algorithmCompleted();
    void pathfindingSearchCompleted(int nodesVisited, int pathLength);

private:
    // Helper for Recursive Backtracker
    std::vector<Node*> getMazeNeighbors(Node* node);
    void connectNodes(Node* a, Node* b);

public:
    ALGOS currentAlgorithm;
    bool running;
    bool simulationOnGoing;
    bool endReached;
    int speedVisualization;

    grid gridNodes;
    int heightGrid;
    int widthGrid;

    QThreadPool pool;
    QFuture<int> futureOutput;

    QString algorithmToString(ALGOS algo);
    void setGridNodes(const grid& newGridNodes, int width, int height);
};

#endif // PATHALGORITHM_H
