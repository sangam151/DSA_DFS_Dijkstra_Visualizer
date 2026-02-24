#include <iostream>
#include <QtCore/QtMath>
#include <QtCore/QDebug>
#include <cmath>
#include <QMessageBox>
#include "GridView.h"
#include <QCategoryAxis>
#include <queue>

// Constructor
GridView::GridView(int widthGrid, int heightGrid, int markerSize, QChartView* parent)
    : QChartView(parent)
{
    this->widthGrid = widthGrid;
    this->heightGrid = heightGrid;
    this->markerSize = markerSize;

    chart = new QChart();
    chart->setBackgroundVisible(true);

    freeElements        = new QScatterSeries();
    obstacleElements    = new QScatterSeries();
    visitedElements     = new QScatterSeries();
    nextElements        = new QScatterSeries();
    pathElements        = new QScatterSeries();
    startElement        = new QScatterSeries();
    endElement          = new QScatterSeries();

    startElement->append(QPoint());
    endElement->append(QPoint());

    pathLine = new QLineSeries();

    currentInteraction  = NOINTERACTION;
    currentArrangement  = EMPTY;
    currentAlgorithm    = NOALGO;
    simulationRunning   = false;
}

GridView::~GridView()
{
    delete freeElements;
    delete obstacleElements;
    delete visitedElements;
    delete nextElements;
    delete startElement;
    delete endElement;
}

void GridView::setCurrentInteraction(int index)
{
    currentInteraction = static_cast<INTERACTIONS>(index);
}

void GridView::setCurrentInteraction(INTERACTIONS interaction)
{
    currentInteraction = interaction;
}

void GridView::setSimulationRunning(bool state)
{
    simulationRunning = state;
}

void GridView::setCurrentAlgorithm(int index)
{
    // Only DIJKSTRA (0) and BACKTRACK (1) are valid, but we map directly.
    currentAlgorithm = static_cast<ALGOS>(index);
}

INTERACTIONS GridView::getCurrentInteraction() const
{
    return currentInteraction;
}

ARRANGEMENTS GridView::getCurrentArrangement() const
{
    return currentArrangement;
}

grid& GridView::getGrid()
{
    return gridNodes;
}

ALGOS GridView::getCurrentAlgorithm() const
{
    return currentAlgorithm;
}

bool GridView::getSimulationRunning() const
{
    return simulationRunning;
}

int GridView::getHeightGrid() const
{
    return heightGrid;
}

void GridView::populateGridMap(ARRANGEMENTS arrangement, bool reset)
{
    if (simulationRunning)
    {
        QMessageBox::information(this, "Information", "Please stop the simulation first");
        return;
    }

    if (reset){
        freeElements->removePoints(0, freeElements->points().size());
        obstacleElements->removePoints(0, obstacleElements->points().size());
        visitedElements->removePoints(0, visitedElements->points().size());
        nextElements->removePoints(0, nextElements->points().size());
        pathElements->removePoints(0, pathElements->points().size());
        pathLine->removePoints(0, pathLine->points().size());

        gridNodes.Nodes.clear();

        chart->axes(Qt::Horizontal).first()->setRange(qreal(0.4), qreal(this->widthGrid + 0.5));
        chart->axes(Qt::Vertical).first()->setRange(qreal(0.4), qreal(this->heightGrid + 0.5));
    }

    for (int i = 0; i < heightGrid * widthGrid; i++)
    {
        freeElements->append(QPoint());
        obstacleElements->append(QPoint());
        visitedElements->append(QPoint());
        nextElements->append(QPoint());
        pathElements->append(QPoint());
    }

    Node dummy;
    for (int i = 0; i < widthGrid * heightGrid; i++)
        gridNodes.Nodes.push_back(dummy);

    if (arrangement == EMPTY)
    {
        startElement->replace(0, QPointF(1, heightGrid));
        endElement->replace(0, QPointF(widthGrid, 1));

        int indexGrid = 0;
        for (int y = 1; y <= heightGrid; y++)
        {
            for (int x = 1; x <= widthGrid; x++)
            {
                if (x == startElement->points()[0].x() && y == startElement->points()[0].y()){
                    gridNodes.startIndex = indexGrid;
                } else if (x == endElement->points()[0].x() && y == endElement->points()[0].y()){
                    gridNodes.endIndex = indexGrid;
                } else {
                    freeElements->replace(indexGrid, QPointF(x, y));
                }

                if (gridNodes.Nodes[indexGrid].obstacle)
                    obstacleElements->replace(indexGrid, QPointF());

                Node n;
                n.xCoord = x;
                n.yCoord = y;
                gridNodes.Nodes[indexGrid] = n;
                gridNodes.Nodes[indexGrid].obstacle = false;
                gridNodes.Nodes[indexGrid].visited = false;
                gridNodes.Nodes[indexGrid].nextUp = false;

                visitedElements->replace(indexGrid, QPointF());
                nextElements->replace(indexGrid, QPointF());
                pathElements->replace(indexGrid, QPointF());

                indexGrid++;
            }
        }
    }
    else
    {
        std::cerr << "BAD ARRANGEMENT\n";
    }

    gridNodes.currentIndex = gridNodes.startIndex;
}

QChart* GridView::createChart()
{
    populateGridMap(currentArrangement, false);
    setRenderHint(QPainter::Antialiasing);

    freeElements->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    obstacleElements->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    visitedElements->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    nextElements->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    pathElements->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    startElement->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
    endElement->setMarkerShape(QScatterSeries::MarkerShapeRectangle);

    setElementsMarkerSize();

    obstacleElements->setColor(QColor("#1e272e"));
    visitedElements->setColor(QColor("#34ace0"));
    nextElements->setColor(QColor("#ffb142"));
    pathElements->setColor(QColor("#ff5252"));
    startElement->setColor(QColor("#0be881"));
    endElement->setColor(QColor("#ff3f34"));

    freeElements->setOpacity(0.2);
    visitedElements->setOpacity(0.75);
    nextElements->setOpacity(0.75);
    pathElements->setOpacity(0.75);
    startElement->setOpacity(0.95);
    endElement->setOpacity(0.95);

    freeElements->setBorderColor(QColorConstants::Black);
    obstacleElements->setBorderColor(QColorConstants::Black);
    visitedElements->setBorderColor(QColorConstants::Black);
    nextElements->setBorderColor(QColorConstants::Black);
    pathElements->setBorderColor(QColorConstants::Black);
    startElement->setBorderColor(QColorConstants::Black);
    endElement->setBorderColor(QColorConstants::Black);

    chart->addSeries(freeElements);
    chart->addSeries(obstacleElements);
    chart->addSeries(visitedElements);
    chart->addSeries(nextElements);
    chart->addSeries(pathElements);
    chart->addSeries(startElement);
    chart->addSeries(endElement);
    chart->addSeries(pathLine);

    startElement->setName("Start");
    endElement->setName("Goal");
    freeElements->setName("Free");
    obstacleElements->setName("Obstacles");
    visitedElements->setName("Visited");
    nextElements->setName("Next");
    pathElements->setName("Path");

    visitedElements->setPointsVisible(false);
    nextElements->setPointsVisible(false);
    pathElements->setPointsVisible(false);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    QLinearGradient bg;
    bg.setStart(0,0); bg.setFinalStop(0,1);
    bg.setColorAt(0.0, QRgb(0xa8edea));
    bg.setColorAt(1.0, QRgb(0xfed6e3));
    bg.setCoordinateMode(QGradient::ObjectBoundingMode);
    chart->setBackgroundBrush(bg);

    chart->createDefaultAxes();
    chart->setPlotAreaBackgroundVisible(false);

    QList<QAbstractAxis*> axisX = chart->axes(Qt::Horizontal);
    QList<QAbstractAxis*> axisY = chart->axes(Qt::Vertical);
    axisX.first()->setRange(qreal(0.4), qreal(this->widthGrid + 0.4));
    axisY.first()->setRange(qreal(0.4), qreal(this->heightGrid + 0.5));
    axisX.first()->setGridLineVisible(false);
    axisY.first()->setGridLineVisible(false);
    axisX.first()->setVisible(true);
    axisY.first()->setVisible(true);

    connect(freeElements, &QScatterSeries::clicked, this, &GridView::handleClickedPoint);
    connect(obstacleElements, &QScatterSeries::clicked, this, &GridView::handleClickedPoint);

    return chart;
}

void GridView::setElementsMarkerSize()
{
    freeElements->setMarkerSize(markerSize);
    obstacleElements->setMarkerSize(markerSize);
    visitedElements->setMarkerSize(markerSize);
    nextElements->setMarkerSize(markerSize);
    pathElements->setMarkerSize(markerSize);
    startElement->setMarkerSize(markerSize);
    endElement->setMarkerSize(markerSize);
}

void GridView::handleClickedPoint(const QPointF& point)
{
    QPointF clickedPoint = point;
    QPointF nullPoint;
    int idx = coordToIndex(clickedPoint, widthGrid);

    if (currentInteraction == OBST)
    {
        if (!gridNodes.Nodes[idx].obstacle)
        {
            obstacleElements->replace(idx, clickedPoint);
            freeElements->replace(clickedPoint, nullPoint);
            gridNodes.Nodes[idx].obstacle = true;
        }
        else
        {
            freeElements->replace(idx, clickedPoint);
            obstacleElements->replace(clickedPoint, nullPoint);
            gridNodes.Nodes[idx].obstacle = false;
        }
    }
    else if (currentInteraction == START)
    {
        int prevStart = gridNodes.startIndex;
        QPointF prevStartPoint = startElement->points()[0];

        gridNodes.startIndex = idx;

        if (!gridNodes.Nodes[idx].obstacle)
        {
            startElement->replace(0, clickedPoint);
            if (prevStart != gridNodes.endIndex)
                freeElements->replace(prevStart, prevStartPoint);
            freeElements->replace(idx, nullPoint);
            obstacleElements->replace(idx, nullPoint);
            gridNodes.Nodes[prevStart].obstacle = false;
        }
        else
        {
            startElement->replace(0, clickedPoint);
            if (prevStart != gridNodes.endIndex)
                obstacleElements->replace(prevStart, prevStartPoint);
            freeElements->replace(idx, nullPoint);
            obstacleElements->replace(idx, nullPoint);
            gridNodes.Nodes[prevStart].obstacle = true;
        }
    }
    else if (currentInteraction == END)
    {
        int prevEnd = gridNodes.endIndex;
        QPointF prevEndPoint = endElement->points()[0];

        gridNodes.endIndex = idx;

        if (!gridNodes.Nodes[idx].obstacle)
        {
            endElement->replace(0, clickedPoint);
            if (prevEnd != gridNodes.startIndex)
                freeElements->replace(prevEnd, prevEndPoint);
            freeElements->replace(idx, nullPoint);
            obstacleElements->replace(idx, nullPoint);
            gridNodes.Nodes[prevEnd].obstacle = false;
        }
        else
        {
            endElement->replace(0, clickedPoint);
            if (prevEnd != gridNodes.startIndex)
                obstacleElements->replace(prevEnd, prevEndPoint);
            freeElements->replace(idx, nullPoint);
            obstacleElements->replace(idx, nullPoint);
            gridNodes.Nodes[prevEnd].obstacle = true;
        }
    }
    else if (currentInteraction == NOINTERACTION)
    {
        QMessageBox::information(this, "Information", "Please select an interaction type");
    }
    else if (simulationRunning)
    {
        QMessageBox::information(this, "Information", "Please stop the simulation first");
    }
}

void GridView::setupNodes()
{
    gridNodes.Nodes.clear();
    gridNodes.Nodes.reserve(widthGrid * heightGrid);

    for (int y = 1; y <= heightGrid; ++y)
    {
        for (int x = 1; x <= widthGrid; ++x)
        {
            Node n;
            n.xCoord = x;
            n.yCoord = y;
            n.visited = false;
            n.obstacle = false;
            n.nextUp = false;
            n.globalGoal = INFINITY;
            n.localGoal = INFINITY;
            n.parent = nullptr;
            gridNodes.Nodes.push_back(n);
        }
    }

    gridNodes.startIndex = coordToIndex(1, 1, widthGrid);
    gridNodes.endIndex = coordToIndex(widthGrid, heightGrid, widthGrid);

    freeElements->clear();
    obstacleElements->clear();
    visitedElements->clear();
    nextElements->clear();
    pathElements->clear();
    pathLine->clear();
    startElement->clear();
    endElement->clear();

    for (const auto& node : gridNodes.Nodes) {
        if (!node.obstacle)
            freeElements->append(QPointF(node.xCoord, node.yCoord));
    }
    startElement->append(QPointF(gridNodes.Nodes[gridNodes.startIndex].xCoord, gridNodes.Nodes[gridNodes.startIndex].yCoord));
    endElement->append(QPointF(gridNodes.Nodes[gridNodes.endIndex].xCoord, gridNodes.Nodes[gridNodes.endIndex].yCoord));
}

int coordToIndex(int x, int y, int w)
{
    return (y - 1) * w + (x - 1);
}

qreal GridView::computeDistanceBetweenPoints(const QPointF& a, const QPointF& b)
{
    return qSqrt(qPow(a.x() - b.x(), 2) + qPow(a.y() - b.y(), 2));
}

int coordToIndex(const QPointF& p, int w)
{
    return (p.y() - 1) * w + p.x() - 1;
}

void GridView::AlgorithmView(bool on)
{
    if (on) {
        nextElements->setPointsVisible(true);
        pathElements->setPointsVisible(true);
        visitedElements->setPointsVisible(true);
    }
}

bool GridView::handleUpdatedScatterGridView(UPDATETYPES type, int idx)
{
    switch (type) {
    case VISIT:         replaceNextbyVisited(idx); break;
    case NEXT:          replaceFreebyNext(idx);    break;
    case PATH:          replaceVisitedbyPath(idx); break;
    case FREE:          replaceNextbyFree(idx);    break;
    case FREETOOBSTACLE: replaceFreebyObstacle(idx); break;
    case OBSTACLETOFREE: replaceObstaclebyFree(idx); break;
    default: break;
    }
    return true;
}

bool GridView::handleUpdatedLineGridView(QPointF p, bool add, bool clear)
{
    if (clear) pathLine->clear();
    updateLine(p, add);
    return true;
}

void GridView::replaceFreebyVisited(int idx)
{
    QList<QPointF> pts = freeElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        freeElements->replace(idx, QPointF());
        visitedElements->replace(idx, pts[idx]);
    }
}

void GridView::replaceFreebyObstacle(int idx)
{
    QList<QPointF> pts = freeElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        freeElements->replace(idx, QPointF());
        obstacleElements->replace(idx, pts[idx]);
        gridNodes.Nodes[idx].obstacle = true;
    }
}

void GridView::replaceObstaclebyFree(int idx)
{
    QList<QPointF> pts = obstacleElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        obstacleElements->replace(idx, QPointF());
        freeElements->replace(idx, pts[idx]);
        gridNodes.Nodes[idx].obstacle = false;
    }
}

void GridView::replaceNextbyVisited(int idx)
{
    QList<QPointF> pts = nextElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        nextElements->replace(idx, QPointF());
        visitedElements->replace(idx, pts[idx]);
    }
}

void GridView::replaceVisitedbyPath(int idx)
{
    QList<QPointF> pts = visitedElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        visitedElements->replace(idx, QPointF());
        pathElements->replace(idx, pts[idx]);
    }
}

void GridView::replaceFreebyNext(int idx)
{
    QList<QPointF> pts = freeElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        freeElements->replace(idx, QPointF());
        nextElements->replace(idx, pts[idx]);
    }
}

void GridView::replaceNextbyFree(int idx)
{
    QList<QPointF> pts = nextElements->points();
    if (idx < pts.size() && pts[idx] != QPointF()) {
        nextElements->replace(idx, QPointF());
        freeElements->replace(idx, pts[idx]);
    }
}

void GridView::updateLine(QPointF p, bool adding)
{
    if (adding)
        pathLine->append(p);
    else
        pathLine->replace(pathLine->points().size() - 1, p);
}

int GridView::getPathLength() const { return 0; }

float GridView::calculateWallDensity() const
{
    int total = widthGrid * heightGrid;
    if (total == 0) return 0;
    int walls = 0;
    for (const Node& n : gridNodes.Nodes)
        if (n.obstacle) walls++;
    return static_cast<float>(walls) / total;
}

int GridView::countDeadEnds() const
{
    int dead = 0;
    for (const Node& n : gridNodes.Nodes) {
        if (n.obstacle) continue;
        int wallCount = 0;
        int dx[] = {0,0,1,-1};
        int dy[] = {1,-1,0,0};
        for (int i=0; i<4; ++i) {
            int nx = n.xCoord + dx[i];
            int ny = n.yCoord + dy[i];
            if (nx < 1 || nx > widthGrid || ny < 1 || ny > heightGrid) {
                wallCount++;
            } else {
                int idx = coordToIndex(nx, ny, widthGrid);
                if (gridNodes.Nodes[idx].obstacle) wallCount++;
            }
        }
        if (wallCount == 3) dead++;
    }
    return dead;
}

float GridView::calculateBranchingFactor() const
{
    int totalNeighbors = 0;
    int nonWall = 0;
    for (const Node& n : gridNodes.Nodes) {
        if (n.obstacle) continue;
        nonWall++;
        int cnt = 0;
        int dx[] = {0,0,1,-1};
        int dy[] = {1,-1,0,0};
        for (int i=0; i<4; ++i) {
            int nx = n.xCoord + dx[i];
            int ny = n.yCoord + dy[i];
            if (nx >=1 && nx<=widthGrid && ny>=1 && ny<=heightGrid) {
                int idx = coordToIndex(nx, ny, widthGrid);
                if (!gridNodes.Nodes[idx].obstacle) cnt++;
            }
        }
        totalNeighbors += cnt;
    }
    return nonWall ? static_cast<float>(totalNeighbors) / nonWall : 0;
}
