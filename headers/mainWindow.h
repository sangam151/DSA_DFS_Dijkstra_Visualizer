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


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor
    MainWindow(QWidget *parent = nullptr);
    // Destructor
    virtual ~MainWindow();

    // Setting up objects
    void setupInteractionComboBox();
    void setupAlgorithmsComboBox();
    void setupGridView(QString gridViewName);
    void setupComparisonTable();

    // Getters
    GridView& getGridView();


public: Q_SIGNALS:
    //void launchedBFS();

public slots:
    // Run the simulation and pause it
    void on_runButton_clicked();

    // Reset the ChartView
    void on_resetButton_clicked();

    // Generate button
    void on_mazeButton_clicked(); // Renamed from on_mazeButton_released() for clarity

    // Handles the different interaction changes
    void on_interactionBox_currentIndexChanged(int index);

    // Handles the different algorithm changes
    void on_algorithmsBox_currentIndexChanged(int index);

    // Action to do when the path algorithm is finished
    void onAlgorithmCompleted();

    // Action to do when the pathfinding search completes (before visualization)
    void onPathfindingSearchCompleted(int nodesVisited, int pathLength); // This slot will now only populate the list

private slots:
    void on_dialWidth_valueChanged(int value);
    void on_dialHeight_valueChanged(int value);
    void on_sliderMarker_valueChanged(int value);
    void on_sliderMarker_sliderReleased();
    void on_dialWidth_sliderReleased();
    void on_dialHeight_sliderReleased();
    void on_speedSpinBox_valueChanged(int arg1);
    void updateElapsedTime(); // slot for timer updates
    void generateMazeWithAlgorithm(int algorithmEnum);
    void on_clearComparisonButton_clicked(); // Slot for clearing all comparison data
    void on_deleteSelectedRowButton_clicked(); // Slot for deleting a selected row

    // Slot to receive and process extracted maze features (and predict difficulty)
    void extractAndExportMazeFeatures(int nodesVisited, int pathLength);

    // Slot for the "Play Yourself" button
    void on_playYourselfButton_clicked();
    // Slot to handle game finished signal from PlayerMazeWindow
    void onPlayerMazeGameFinished(bool won);


private:
    Ui::MainWindow* ui;
    GridView gridView;
    PathAlgorithm pathAlgorithm;
    QTimer* animationTimer;
    QElapsedTimer elapsedTimer;
    QLabel* timeDisplayLabel;

    qint64 pausedTimeOffset;
    QList<AlgorithmComparisonData> comparisonDataList;

    // Function to export features to CSV
    void exportFeaturesToCSV(const AlgorithmComparisonData& dataToExport);

    // Function to update the QTableWidget with all current data
    void updateComparisonTable();

    QPushButton* playYourselfButton; // Declare the new button
    PlayerMazeWindow* playerMazeWindow; // Pointer to the player maze window

    bool mazeCurrentlyGenerated; //Flag to track if a maze is generated
};
#endif // MAINWINDOW_H