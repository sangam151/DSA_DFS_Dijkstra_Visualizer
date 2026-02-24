#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QElapsedTimer>
#include <QTableWidget>
#include <QTableWidgetItem>
#include "GridView.h"
#include "PathAlgorithm.h"
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Structure to hold comparison data for each algorithm run
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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setupInteractionComboBox();
    void setupAlgorithmsComboBox();
    void setupGridView(QString gridViewName);
    void setupComparisonTable();

    GridView& getGridView();

public slots:
    void on_runButton_clicked();
    void on_resetButton_clicked();
    void on_mazeButton_clicked();
    void on_interactionBox_currentIndexChanged(int index);
    void on_algorithmsBox_currentIndexChanged(int index);
    void onAlgorithmCompleted();
    void onPathfindingSearchCompleted(int nodesVisited, int pathLength);

private slots:
    void on_dialWidth_valueChanged(int value);
    void on_dialHeight_valueChanged(int value);
    void on_sliderMarker_valueChanged(int value);
    void on_sliderMarker_sliderReleased();
    void on_dialWidth_sliderReleased();
    void on_dialHeight_sliderReleased();
    void on_speedSpinBox_valueChanged(int arg1);
    void updateElapsedTime();
    void generateMazeWithAlgorithm(int algorithmEnum);
    void on_clearComparisonButton_clicked();
    void on_deleteSelectedRowButton_clicked();
    void extractAndExportMazeFeatures(int nodesVisited, int pathLength);

private:
    Ui::MainWindow* ui;
    GridView gridView;
    PathAlgorithm pathAlgorithm;
    QTimer* animationTimer;
    QElapsedTimer elapsedTimer;
    QLabel* timeDisplayLabel;

    qint64 pausedTimeOffset;
    QList<AlgorithmComparisonData> comparisonDataList;

    void updateComparisonTable();
    bool mazeCurrentlyGenerated;
};

#endif // MAINWINDOW_H
