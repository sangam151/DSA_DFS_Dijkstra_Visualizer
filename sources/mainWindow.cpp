#include <QChartView>
#include<QPushButton>
#include <QMessageBox>
#include <QLabel>         // Include QLabel
#include <QTime>
#include <QFile>          // For CSV export
#include <QTextStream>    // For CSV export
#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "GridView.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow), gridView(30, 30, 19), pathAlgorithm()
{
    // Setup of the window
    ui->setupUi(this);


    // Customize chart background
    QLinearGradient backgroundGradient;
    backgroundGradient.setStart(QPointF(0, 0));
    backgroundGradient.setFinalStop(QPointF(0, 1));
    backgroundGradient.setColorAt(0.0, QRgb(0xd2d0d1));
    backgroundGradient.setColorAt(1.0, QRgb(0x4c4547));
    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QBrush brush(backgroundGradient);

    QPalette palette;
    palette.setBrush(QPalette::Window, brush);
    setPalette(palette);

    // Setup
    ui->dialWidth   ->setValue  (gridView.widthGrid);
    ui->dialWidth   ->setMinimum(5);
    ui->dialWidth   ->setMaximum(35);
    ui->lcdWidth    ->display   (gridView.widthGrid);

    ui->dialHeight  ->setValue  (gridView.heightGrid);
    ui->dialHeight  ->setMinimum(5);
    ui->dialHeight  ->setMaximum(35);
    ui->lcdHeight   ->display   (gridView.heightGrid);

    ui->sliderMarker->setValue  (gridView.markerSize);
    ui->lcdMarker   ->display   (gridView.markerSize);

    // Initial Simulation speed
    ui->speedSpinBox->setMaximum(100);
    int speed = ui->speedSpinBox->maximum() / 5;
    ui->speedSpinBox->setValue  (speed);

    // Initial state for the run button
    ui->runButton->setChecked(false); // Ensure it starts in the "play" state
    ui->runButton->setText(QString("Start PathFinding")); // Initial text


    // Setting up the chart view
    setupGridView("gridView");

    // Setting up the Interaction Combo Box
    setupInteractionComboBox();

    // Setting up the Algorithms Combo Box
    setupAlgorithmsComboBox();

    // Setup the comparison table
    setupComparisonTable();

    // A change in the grid view create a change in the chartview
    connect(&pathAlgorithm, &PathAlgorithm::updatedScatterGridView, &gridView, &GridView::handleUpdatedScatterGridView);
    connect(&pathAlgorithm, &PathAlgorithm::updatedLineGridView,    &gridView, &GridView::handleUpdatedLineGridView);

    // Connecting the end signal of path planning to the window
    connect(&pathAlgorithm, &PathAlgorithm::algorithmCompleted, this, &MainWindow::onAlgorithmCompleted);
    //Connecting signal for pathfinding search completion (for timer stop)
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted, this, &MainWindow::onPathfindingSearchCompleted);

    // Connect pathfinding search completed to feature extraction
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted, this, &MainWindow::extractAndExportMazeFeatures);


    // --- Timer Setup ---
    animationTimer = new QTimer(this); // Initialize the QTimer
    // Connecting the timer's timeout signal to our new slot for updating the display
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);
    animationTimer->setInterval(10); // Set update frequency to every 10 milliseconds (0.01 seconds)

    // Initialize the QLabel for time display
    timeDisplayLabel = new QLabel("Time: 0.000 s", this);
    timeDisplayLabel->setAlignment(Qt::AlignCenter);
    timeDisplayLabel->setStyleSheet("font-weight: bold; color: #34ace0;"); // Styling for visibility
    ui->hLayout->addWidget(timeDisplayLabel); // Add the label to your existing horizontal layout
    pausedTimeOffset = 0; //initialize paused time offset

    // Connect the clear comparison button
    connect(ui->clearComparisonButton, &QPushButton::clicked, this, &MainWindow::on_clearComparisonButton_clicked);

    QPushButton* deleteRowButton = new QPushButton("Delete Selected Row", this);
    ui->verticalLayout_2->addWidget(deleteRowButton); // in the comparisonTab's vertical layout
    connect(deleteRowButton, &QPushButton::clicked, this, &MainWindow::on_deleteSelectedRowButton_clicked);

    // NEW: "Play Yourself" button setup
    playYourselfButton = new QPushButton("Play Yourself", this);
    // Assuming you have a layout in your UI where you want to add this button, e.g., ui->hLayout or a new layout
    // For demonstration, let's add it to the existing hLayout next to the time display.
    ui->hLayout->addWidget(playYourselfButton);
    connect(playYourselfButton, &QPushButton::clicked, this, &MainWindow::on_playYourselfButton_clicked);

    playerMazeWindow = nullptr; // Initialize pointer to null
    mazeCurrentlyGenerated = false; // NEW: Initialize the maze generated flag
}
