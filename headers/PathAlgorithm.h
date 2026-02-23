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