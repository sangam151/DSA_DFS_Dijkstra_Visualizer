#include <QChartView>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTime>
#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gridView(30, 30, 19)
    , pathAlgorithm()
{
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

    // Setup dials and sliders
    ui->dialWidth->setValue(gridView.widthGrid);
    ui->dialWidth->setMinimum(5);
    ui->dialWidth->setMaximum(35);
    ui->lcdWidth->display(gridView.widthGrid);

    ui->dialHeight->setValue(gridView.heightGrid);
    ui->dialHeight->setMinimum(5);
    ui->dialHeight->setMaximum(35);
    ui->lcdHeight->display(gridView.heightGrid);

    ui->sliderMarker->setValue(gridView.markerSize);
    ui->lcdMarker->display(gridView.markerSize);

    // Simulation speed
    ui->speedSpinBox->setMaximum(100);
    int speed = ui->speedSpinBox->maximum() / 5;
    ui->speedSpinBox->setValue(speed);

    // Run button initial state
    ui->runButton->setChecked(false);
    ui->runButton->setText("Start PathFinding");

    // Setup grid view
    setupGridView("gridView");

    // Setup combo boxes
    setupInteractionComboBox();
    setupAlgorithmsComboBox();

    // Setup comparison table
    setupComparisonTable();

    // Connect signals from pathAlgorithm to gridView
    connect(&pathAlgorithm, &PathAlgorithm::updatedScatterGridView,
            &gridView, &GridView::handleUpdatedScatterGridView);
    connect(&pathAlgorithm, &PathAlgorithm::updatedLineGridView,
            &gridView, &GridView::handleUpdatedLineGridView);

    // Connect algorithm completion signals
    connect(&pathAlgorithm, &PathAlgorithm::algorithmCompleted,
            this, &MainWindow::onAlgorithmCompleted);
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted,
            this, &MainWindow::onPathfindingSearchCompleted);

    // Connect pathfinding search to feature extraction (for table update)
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted,
            this, &MainWindow::extractAndExportMazeFeatures);

    // Timer setup for elapsed time display
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);
    animationTimer->setInterval(10);

    timeDisplayLabel = new QLabel("Time: 0.000 s", this);
    timeDisplayLabel->setAlignment(Qt::AlignCenter);
    timeDisplayLabel->setStyleSheet("font-weight: bold; color: #34ace0;");
    ui->hLayout->addWidget(timeDisplayLabel);

    pausedTimeOffset = 0;

    // Connect clear and delete buttons for comparison table
    connect(ui->clearComparisonButton, &QPushButton::clicked,
            this, &MainWindow::on_clearComparisonButton_clicked);

    QPushButton* deleteRowButton = new QPushButton("Delete Selected Row", this);
    ui->verticalLayout_2->addWidget(deleteRowButton);
    connect(deleteRowButton, &QPushButton::clicked,
            this, &MainWindow::on_deleteSelectedRowButton_clicked);

    mazeCurrentlyGenerated = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete animationTimer;
}

void MainWindow::setupInteractionComboBox()
{
    ui->interactionBox->setPlaceholderText("--Select Interaction--");
    ui->interactionBox->setCurrentIndex(-1);
    ui->interactionBox->addItem("Add Start");
    ui->interactionBox->addItem("Add Goal");
    ui->interactionBox->addItem("Add Obstacles");
}

void MainWindow::setupAlgorithmsComboBox()
{
    ui->algorithmsBox->setPlaceholderText("--Select Algorithm--");
    ui->algorithmsBox->setCurrentIndex(-1);
    // Only Dijkstra for pathfinding
    ui->algorithmsBox->addItem("Dijkstra's Algorithm");
}

void MainWindow::setupGridView(QString gridViewName)
{
    ui->gridView->setObjectName(gridViewName);
    ui->gridView->setMinimumWidth(700);
    ui->gridView->setMinimumHeight(700);

    gridView.setupNodes();
    pathAlgorithm.setGridNodes(gridView.gridNodes, gridView.widthGrid, gridView.heightGrid);

    gridView.chart->removeAllSeries();
    gridView.chart->addSeries(gridView.freeElements);
    gridView.chart->addSeries(gridView.obstacleElements);
    gridView.chart->addSeries(gridView.visitedElements);
    gridView.chart->addSeries(gridView.nextElements);
    gridView.chart->addSeries(gridView.pathElements);
    gridView.chart->addSeries(gridView.pathLine);
    gridView.chart->addSeries(gridView.startElement);
    gridView.chart->addSeries(gridView.endElement);

    QChart* chart = gridView.createChart();
    ui->gridView->setChart(chart);
}

void MainWindow::setupComparisonTable()
{
    ui->comparisonTable->setColumnCount(8);
    QStringList headers;
    headers << "Algorithm" << "Time (s)" << "Nodes Visited" << "Path Length"
            << "Grid Size" << "Wall Density" << "Dead Ends" << "Branching Factor";
    ui->comparisonTable->setHorizontalHeaderLabels(headers);
    ui->comparisonTable->horizontalHeader()->setStretchLastSection(true);
    ui->comparisonTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->comparisonTable->setSelectionMode(QAbstractItemView::SingleSelection);
}

void MainWindow::updateComparisonTable()
{
    ui->comparisonTable->setRowCount(comparisonDataList.size());
    for (int i = 0; i < comparisonDataList.size(); ++i) {
        const AlgorithmComparisonData& data = comparisonDataList.at(i);
        ui->comparisonTable->setItem(i, 0, new QTableWidgetItem(data.algorithmName));
        ui->comparisonTable->setItem(i, 1, new QTableWidgetItem(QString::number(data.timeElapsedMs / 1000.0, 'f', 3)));
        ui->comparisonTable->setItem(i, 2, new QTableWidgetItem(QString::number(data.nodesVisited)));
        ui->comparisonTable->setItem(i, 3, new QTableWidgetItem(QString::number(data.pathLength)));
        ui->comparisonTable->setItem(i, 4, new QTableWidgetItem(data.gridSize));
        ui->comparisonTable->setItem(i, 5, new QTableWidgetItem(QString::number(data.wallDensity, 'f', 4)));
        ui->comparisonTable->setItem(i, 6, new QTableWidgetItem(QString::number(data.numDeadEnds)));
        ui->comparisonTable->setItem(i, 7, new QTableWidgetItem(QString::number(data.branchingFactor, 'f', 4)));
    }
}

GridView& MainWindow::getGridView()
{
    return gridView;
}

void MainWindow::on_runButton_clicked()
{
    if (ui->algorithmsBox->currentIndex() == -1) {
        QMessageBox::information(this, "Information", "Please select a path finding algorithm");
        ui->runButton->setChecked(false);
        ui->runButton->setText("Start PathFinding");
        return;
    }

    if (pathAlgorithm.simulationOnGoing) {
        if (pathAlgorithm.running) {
            pathAlgorithm.pauseAlgorithm();
            gridView.setSimulationRunning(false);
            pathAlgorithm.running = false;
            ui->runButton->setChecked(false);
            ui->runButton->setText("Resume PathFinding");
            animationTimer->stop();
            pausedTimeOffset += elapsedTimer.elapsed();
        } else {
            pathAlgorithm.resumeAlgorithm();
            gridView.setSimulationRunning(true);
            pathAlgorithm.running = true;
            ui->runButton->setChecked(true);
            ui->runButton->setText("Pause PathFinding");
            elapsedTimer.restart();
            animationTimer->start();
        }
    } else {
        pathAlgorithm.running = true;
        pathAlgorithm.simulationOnGoing = true;

        pathAlgorithm.gridNodes = gridView.gridNodes;
        pathAlgorithm.heightGrid = gridView.heightGrid;
        pathAlgorithm.widthGrid = gridView.widthGrid;

        ui->runButton->setCheckable(true);
        ui->runButton->setChecked(true);
        ui->runButton->setText("Pause PathFinding");

        gridView.setSimulationRunning(true);
        gridView.AlgorithmView(true);

        pausedTimeOffset = 0;
        elapsedTimer.start();
        animationTimer->start();

        pathAlgorithm.runAlgorithm(pathAlgorithm.getCurrentAlgorithm());
    }
}

void MainWindow::on_resetButton_clicked()
{
    gridView.populateGridMap(gridView.getCurrentArrangement(), true);

    pathAlgorithm.running = false;
    pathAlgorithm.simulationOnGoing = false;
    ui->runButton->setChecked(false);
    ui->runButton->setText("Start PathFinding");
    gridView.setSimulationRunning(false);

    gridView.setCurrentInteraction(NOINTERACTION);
    ui->interactionBox->setCurrentIndex(-1);
    pathAlgorithm.setCurrentAlgorithm(NOALGO);
    ui->algorithmsBox->setCurrentIndex(-1);

    mazeCurrentlyGenerated = false;

    animationTimer->stop();
    timeDisplayLabel->setText("Time: 0.000 s");
    pausedTimeOffset = 0;
}

void MainWindow::on_mazeButton_clicked()
{
    generateMazeWithAlgorithm(BACKTRACK);
}

void MainWindow::generateMazeWithAlgorithm(int algorithmEnum)
{
    pathAlgorithm.setCurrentAlgorithm(static_cast<ALGOS>(algorithmEnum));
    gridView.setCurrentAlgorithm(algorithmEnum);

    pathAlgorithm.running = true;
    pathAlgorithm.simulationOnGoing = true;

    pathAlgorithm.gridNodes = gridView.gridNodes;
    pathAlgorithm.heightGrid = gridView.heightGrid;
    pathAlgorithm.widthGrid = gridView.widthGrid;

    gridView.setSimulationRunning(true);
    gridView.AlgorithmView(true);

    pausedTimeOffset = 0;
    elapsedTimer.start();
    animationTimer->start();

    pathAlgorithm.runAlgorithm(static_cast<ALGOS>(algorithmEnum));
}

void MainWindow::on_interactionBox_currentIndexChanged(int index)
{
    gridView.setCurrentInteraction(index);
    animationTimer->stop();
    timeDisplayLabel->setText("Time: 0.000 s");
}

void MainWindow::on_algorithmsBox_currentIndexChanged(int index)
{
    // Only Dijkstra (index 0) is available
    if (index == 0) {
        gridView.setCurrentAlgorithm(DIJKSTRA);
        pathAlgorithm.setCurrentAlgorithm(DIJKSTRA);
    }
}

void MainWindow::onAlgorithmCompleted()
{
    gridView.setSimulationRunning(false);
    pathAlgorithm.setSimulationOnGoing(false);
    pathAlgorithm.running = false;
    ui->runButton->setChecked(false);
    ui->runButton->setText("Start PathFinding");

    if (pathAlgorithm.getCurrentAlgorithm() == BACKTRACK) {
        mazeCurrentlyGenerated = true;
    }
}

void MainWindow::onPathfindingSearchCompleted(int nodesVisited, int pathLength)
{
    animationTimer->stop();
    qint64 totalElapsed = pausedTimeOffset + elapsedTimer.elapsed();

    AlgorithmComparisonData data;
    if (pathAlgorithm.getCurrentAlgorithm() == BACKTRACK) {
        data.algorithmName = "Recursive Backtracker (Maze)";
    } else if (pathAlgorithm.getCurrentAlgorithm() == DIJKSTRA) {
        data.algorithmName = "Dijkstra (Pathfinding)";
    } else {
        data.algorithmName = "Unknown";
    }
    data.timeElapsedMs = totalElapsed;
    data.nodesVisited = nodesVisited;
    data.pathLength = pathLength;
    data.gridSize = QString("%1x%2").arg(gridView.widthGrid).arg(gridView.heightGrid);
    data.wallDensity = 0.0;
    data.numDeadEnds = 0;
    data.branchingFactor = 0.0;

    comparisonDataList.append(data);
    updateComparisonTable();

    // extractAndExportMazeFeatures will be called separately via signal
}

void MainWindow::extractAndExportMazeFeatures(int nodesVisited, int pathLength)
{
    Q_UNUSED(nodesVisited);
    Q_UNUSED(pathLength);

    if (comparisonDataList.isEmpty()) return;

    AlgorithmComparisonData& current = comparisonDataList.last();
    current.wallDensity = gridView.calculateWallDensity();
    current.numDeadEnds = gridView.countDeadEnds();
    current.branchingFactor = gridView.calculateBranchingFactor();

    updateComparisonTable();

    // CSV export removed as per requirements.
}

void MainWindow::on_dialWidth_valueChanged(int value)
{
    ui->lcdWidth->display(value);
}

void MainWindow::on_dialHeight_valueChanged(int value)
{
    ui->lcdHeight->display(value);
}

void MainWindow::on_sliderMarker_valueChanged(int value)
{
    ui->lcdMarker->display(value);
}

void MainWindow::on_sliderMarker_sliderReleased()
{
    gridView.markerSize = ui->lcdMarker->value();
    gridView.setElementsMarkerSize();
}

void MainWindow::on_dialWidth_sliderReleased()
{
    gridView.widthGrid = ui->lcdWidth->value();
    gridView.populateGridMap(gridView.getCurrentArrangement(), true);
    mazeCurrentlyGenerated = false;
}

void MainWindow::on_dialHeight_sliderReleased()
{
    gridView.heightGrid = ui->lcdHeight->value();
    gridView.populateGridMap(gridView.getCurrentArrangement(), true);
    mazeCurrentlyGenerated = false;
}

void MainWindow::on_speedSpinBox_valueChanged(int arg1)
{
    pathAlgorithm.setSpeedVizualization(ui->speedSpinBox->maximum() / arg1);
}

void MainWindow::updateElapsedTime()
{
    qint64 elapsed = pausedTimeOffset + elapsedTimer.elapsed();
    timeDisplayLabel->setText(QString("Time: %1 s").arg(elapsed / 1000.0, 0, 'f', 3));
}

void MainWindow::on_clearComparisonButton_clicked()
{
    comparisonDataList.clear();
    ui->comparisonTable->setRowCount(0);
    QMessageBox::information(this, "Comparison Table", "All comparison data cleared.");
}

void MainWindow::on_deleteSelectedRowButton_clicked()
{
    QModelIndexList selected = ui->comparisonTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, "Delete Row", "Please select a row to delete.");
        return;
    }

    for (int i = selected.count() - 1; i >= 0; --i) {
        int row = selected.at(i).row();
        ui->comparisonTable->removeRow(row);
        comparisonDataList.removeAt(row);
    }
    QMessageBox::information(this, "Delete Row", "Selected row(s) deleted.");
}
