// mainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton> // Includes QPushButton for the new button
#include "GridView.h"
#include "PathAlgorithm.h"
#include "qlabel.h"
#include <QFile>
#include <QTextStream>
#include "PlayerMazeWindow.h" // Includes the new player maze window

QT_BEGIN_NAMESPACE

namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE


//structure to hold comparison data for each algorithm run
struct AlgorithmComparisonData {
    QString algorithmName;
    qint64  timeElapsedMs;
    int     nodesVisited;
    int     pathLength;
    QString gridSize;
    qreal   wallDensity;
    int     numDeadEnds;
    qreal   branchingFactor;
};
