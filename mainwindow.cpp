#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QTimer>
#include <QImage>
#include <algorithm>
#include <chrono>
#include <ctime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Créer un système de pages
    static QStackedWidget *pages = new QStackedWidget(this);
    pages->setFixedWidth(this->width());
    pages->setFixedHeight(this->height());

    // Paramètres de la première page
    processes->setFixedWidth(pages->width());
    processes->setFixedHeight(pages->height());

    processListScrollArea->setFixedWidth(processes->width());
    processListScrollArea->setFixedHeight(processes->height()*5/6);
    processListScrollArea->setWidgetResizable(true);
    processListScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    processTable->setFixedWidth(processListScrollArea->width());
    processTable->setMinimumHeight(processListScrollArea->height());
    processTable->setShowGrid(true);
    processTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    processTable->setColumnCount(4);
    QStringList headers;
    headers.append("ID Processus");
    headers.append("Temps d'arrivée (s)");
    headers.append("Temps de traitement (s)");
    headers.append("Priorité");
    processTable->setHorizontalHeaderLabels(headers);
    for (int col = 0; col < processTable->columnCount(); col++)
        processTable->setColumnWidth(col, processTable->width()/4);

    // Liste combo pour le choix des algorithmes
    algorithms->addItem("...choisissez un algorithme...");
    algorithms->addItem("First Come First Served");
    algorithms->addItem("Shortest Job First");
    algorithms->addItem("Shortest Remaining Time First");
    algorithms->addItem("Priority Scheduling");
    algorithms->addItem("Round Robin");
    algorithms->addItem("Completely Fair Scheduling");

    quantumBox->setMinimum(1);

    addRandomProcess->setText("Créer un processus quelconque");
    addCustomProcess->setText("Créer un processus personnalisé");
    simulate->setText("Simuler");

    // Connections des boutons aux fonctions respectives
    connect(addRandomProcess, &QPushButton::clicked, this, &MainWindow::addRandomProcessClicked);
    connect(addCustomProcess, &QPushButton::clicked, this, &MainWindow::addCustomProcessClicked);
    connect(simulate, &QPushButton::clicked, [=]()
    {
        simulateClicked(pages);
    });

    // Layout pour les 2 boutons de création de processus
    QVBoxLayout *createProcessLayout = new QVBoxLayout;
    createProcessLayout->addWidget(addRandomProcess);
    createProcessLayout->addWidget(addCustomProcess);

    // Layout pour le choix du quantum
    QHBoxLayout *quantumLayout = new QHBoxLayout;
    QLabel *quantumLabel = new QLabel("\t\t\tQuantum: ");
    quantumLayout->addWidget(quantumLabel);
    quantumLayout->addWidget(quantumBox);

    // Layout pour le choix de l'algorithme
    QVBoxLayout *comboLayout = new QVBoxLayout;
    comboLayout->addWidget(algorithms);
    comboLayout->addLayout(quantumLayout);

    // Layout pour le bas de page
    QHBoxLayout *footer = new QHBoxLayout;
    footer->addLayout(createProcessLayout);
    footer->addLayout(comboLayout);

    // Layout de la première page
    QVBoxLayout *layout = new QVBoxLayout(processes);
    layout->addWidget(processListScrollArea);
    layout->addLayout(footer);
    layout->addWidget(simulate);
    processes->setLayout(layout);

    // Vérifier les états divers
    connect(algorithms, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::checkCombo);
    connect(processTable, &QTableWidget::itemChanged, this, &MainWindow::checkCombo);
    checkCombo();

    // Paramètres de la deuxième page
    simulation->setFixedWidth(pages->width());
    simulation->setMinimumHeight(pages->height());
    simulation->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    simulation->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    simulation->setWidgetResizable(true);

    simulationContainer = new QWidget;
    simulationContainerLayout = new QVBoxLayout(simulationContainer);
    simulationContainer->setLayout(simulationContainerLayout);
    simulation->setWidget(simulationContainer);
    simulationLayout->addWidget(simulationContainer);
    simulation->setLayout(simulationLayout);

    // Paramètres de la troisième page
    gantt->setMinimumWidth(pages->width());
    gantt->setMinimumHeight(pages->height());

    scrollArea = new QScrollArea;
    view = new QGraphicsView;
    scene = new QGraphicsScene;
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    scrollArea->setWidget(view);
    scrollArea->setWidgetResizable(true);

    ganttLayout->addWidget(scrollArea);
    gantt->setLayout(ganttLayout);
    ganttLayout->addWidget(scrollArea);
    setCentralWidget(gantt);

    // Paramètres de pagination
    pages->addWidget(processes);
    pages->addWidget(simulation);
    pages->addWidget(gantt);

    pages->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addRandomProcessClicked()
{
    Process newProcess = Process();
    process_list.push_back(newProcess);
    processTable->insertRow(process_number-1);
    for (int col = 0; col < processTable->columnCount(); ++col)
    {
        QTableWidgetItem *item;
        switch(col)
        {
        case 0:
            item = new QTableWidgetItem(QString::number(newProcess.PID));
            break;
        case 1:
            item = new QTableWidgetItem(QString::number(newProcess.arrival_time));
            break;
        case 2:
            item = new QTableWidgetItem(QString::number(newProcess.burst_time));
            break;
        default:
            item = new QTableWidgetItem(QString::number(newProcess.priority));
            break;
        }
        processTable->setItem(process_number-1, col, item);
    }
}

void MainWindow::addCustomProcessClicked()
{
    customProcessWidget = new QWidget;
    customProcessWidget->setWindowTitle("Créer un processus personnalisé");
    customProcessWidget->setFixedHeight(150);
    customProcessWidget->setFixedWidth(300);

    QHBoxLayout *arrivalTimeLayout = new QHBoxLayout();
    QLabel *arrivalTimeLabel = new QLabel();
    arrivalTimeLabel->setText("Temps d'arrivée: ");
    arrivalTimeLabel->setFixedWidth(150);
    arrivalTimeLayout->addWidget(arrivalTimeLabel);
    arrivalTimeLayout->addWidget(arrivalTimeInput);

    QHBoxLayout *burstTimeLayout = new QHBoxLayout();
    QLabel *burstTimeLabel = new QLabel();
    burstTimeLabel->setText("Temps d'exécution: ");
    burstTimeLabel->setFixedWidth(150);
    burstTimeLayout->addWidget(burstTimeLabel);
    burstTimeLayout->addWidget(burstTimeInput);

    QHBoxLayout *priorityLayout = new QHBoxLayout();
    QLabel *priorityLabel = new QLabel();
    priorityLabel->setText("Priorité: ");
    priorityLabel->setFixedWidth(150);
    priorityLayout->addWidget(priorityLabel);
    priorityLayout->addWidget(priorityInput);

    createCustomProcess->setText("Créer");
    createCustomProcess->setEnabled(false);

    connect(arrivalTimeInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    connect(burstTimeInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    connect(priorityInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    connect(createCustomProcess, &QPushButton::clicked, this, &MainWindow::createCustomProcessClicked);

    QVBoxLayout *layout = new QVBoxLayout(customProcessWidget);
    layout->addLayout(arrivalTimeLayout);
    layout->addLayout(burstTimeLayout);
    layout->addLayout(priorityLayout);
    layout->addWidget(createCustomProcess);
    customProcessWidget->setLayout(layout);
    customProcessWidget->show();
}

void MainWindow::checkCreateCustomProcess()
{
    bool ok1, ok2, ok3;
    int value1 = arrivalTimeInput->text().toUInt(&ok1);
    int value2 = burstTimeInput->text().toUInt(&ok2);
    int value3 = priorityInput->text().toUInt(&ok3);
    createCustomProcess->setEnabled(ok1 && ok2 && ok3 &&
                                    value1 >= 0 && value2 >= 1 && value3 >= 1);
}

void MainWindow::createCustomProcessClicked()
{
    customProcessWidget->close();
    Process newProcess = Process(arrivalTimeInput->text().toInt(), burstTimeInput->text().toInt(), priorityInput->text().toInt());
    process_list.push_back(newProcess);
    processTable->insertRow(process_number-1);
    for (int col = 0; col < processTable->columnCount(); ++col)
    {
        QTableWidgetItem *item;
        switch(col)
        {
        case 0:
            item = new QTableWidgetItem(QString::number(newProcess.PID));
            break;
        case 1:
            item = new QTableWidgetItem(QString::number(newProcess.arrival_time));
            break;
        case 2:
            item = new QTableWidgetItem(QString::number(newProcess.burst_time));
            break;
        default:
            item = new QTableWidgetItem(QString::number(newProcess.priority));
            break;
        }
        processTable->setItem(process_number-1, col, item);
    }
    arrivalTimeInput->setText("");
    burstTimeInput->setText("");
    priorityInput->setText("");

    disconnect(arrivalTimeInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    disconnect(burstTimeInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    disconnect(priorityInput, &QLineEdit::textChanged, this, &MainWindow::checkCreateCustomProcess);
    disconnect(createCustomProcess, &QPushButton::clicked, this, &MainWindow::createCustomProcessClicked);
}

void MainWindow::simulateClicked(QStackedWidget *pages)
{
    pages->setCurrentIndex(1);

    std::sort(process_list.begin(), process_list.end(), [&](const Process &p1, const Process &p2) {
        return p1.arrival_time < p2.arrival_time || (p1.arrival_time == p2.arrival_time && p1.priority < p2.priority);
    });

    timer = new QTimer(this);
    switch (algorithms->currentIndex())
    {
    case 1:
        connect(timer, &QTimer::timeout, [=]()
        {
            FCFS(pages);
        });
        break;
    case 2:
        connect(timer, &QTimer::timeout, [=]()
        {
            SJF(pages);
        });
        break;
    case 3:
        connect(timer, &QTimer::timeout, [=]()
        {
            SRTF(pages);
        });
        break;
    case 4:
        connect(timer, &QTimer::timeout, [=]()
        {
            PS(pages);
        });
        break;
    case 5:
        connect(timer, &QTimer::timeout, [=]()
        {
            RoundRobin(pages);
        });
        break;
    case 6:
        connect(timer, &QTimer::timeout, [=]()
        {
            CFS(pages);
        });
        break;
    default:
        break;
    }
    timer->start(100);
}

void MainWindow::checkCombo()
{
    if (algorithms->currentIndex() == 0 || process_list.size() == 0)
        simulate->setDisabled(true);
    else
        simulate->setEnabled(true);

    if (algorithms->currentIndex() != 5)
        quantumBox->setEnabled(false);
    else
        quantumBox->setEnabled(true);
}

void MainWindow::enAttente()
{
    for (Process process: process_list)
        if (process.arrival_time == time)
        {
            process_queue.push_back(process);

            QFrame *processFrame = new QFrame(simulationContainer);
            processFrame->setFixedWidth(simulation->width() - 20);
            processFrame->setFixedHeight(100);
            processFrame->setFrameShape(QFrame::StyledPanel);
            simulationContainerLayout->addWidget(processFrame, Qt::AlignTop);

            QLabel *processPIDLabel = new QLabel("Process " + QString::number(process.PID), processFrame);
            QLabel *processArrivalTimeLabel = new QLabel("Arrival time: " + QString::number(process.arrival_time) + "s", processFrame);
            QLabel *processBurstTimeLabel = new QLabel("Burst time: " + QString::number(process.burst_time) + "s", processFrame);
            QLabel *processRemainingTimeLabel = new QLabel("Remaining time: " + QString::number(process.remaining_time) + "s", processFrame);
            processQueueRemainingTime.push_back(processRemainingTimeLabel);
            QLabel *processPriorityLabel = new QLabel("Priority: " + QString::number(process.priority), processFrame);
            QLabel *processStateLabel = new QLabel("État: En attente", processFrame);
            processQueueStateLabel.push_back(processStateLabel);

            QHBoxLayout *processInfoLayout = new QHBoxLayout(processFrame);
            processInfoLayout->addWidget(processPIDLabel);
            processInfoLayout->addWidget(processArrivalTimeLabel);
            processInfoLayout->addWidget(processBurstTimeLabel);
            processInfoLayout->addWidget(processRemainingTimeLabel);
            processInfoLayout->addWidget(processPriorityLabel);
            processInfoLayout->addWidget(processStateLabel);

            QProgressBar *processProgressBar = new QProgressBar(processFrame);
            processProgressBar->setFixedWidth(processFrame->width());
            processProgressBar->setRange(0, 100);
            processProgressBar->setValue(0);
            processProgressBar->show();
            processQueueProgressBars.push_back(processProgressBar);

            QVBoxLayout *processLayout = new QVBoxLayout(processFrame);
            processLayout->addLayout(processInfoLayout, Qt::AlignTop);
            processLayout->addWidget(processProgressBar, Qt::AlignBottom);
            processFrame->setLayout(processLayout);
            simulationContainerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
        }

    // Dessin du diagramme de GANTT, chaque 0.1s
    drawGanttBars();
}

void MainWindow::afterSimulation(QStackedWidget *pages)
{
    drawTimeAxis();
    QPushButton *goToGantt = new QPushButton("Voir le diagramme de GANTT", simulationContainer);
    connect(goToGantt, &QPushButton::clicked, [=]()
    {
        pages->setCurrentIndex(2);
    });
    simulationContainerLayout->addWidget(goToGantt);
}

void MainWindow::drawGanttBars()
{
    if (process_queue.size() == 0 and nb_processed == 0)
        for (size_t i = 0; i < process_list.size(); ++i)
            scene->addRect(500 + time*scale, 50 + i * (rectangleSize + 5), scale/10, rectangleSize, QPen(Qt::black), QBrush(Qt::black));
    else
    {
        for (size_t i = 0; i < process_list.size(); ++i)
        {
            auto it = std::find_if(process_queue.begin(), process_queue.end(), [=](const Process& process) {
                return process.PID == process_list[i].PID;
            });
            if (it == process_queue.end())
            {
                it = std::find_if(completed.begin(), completed.end(), [=](const Process& process) {
                    return process.PID == process_list[i].PID;
                });
                if (it == completed.end())
                    scene->addRect(500 + time*scale, 50 + i * (rectangleSize + 5), scale/10, rectangleSize, QPen(Qt::black), QBrush(Qt::black));
            }
            else
            {
                it = std::find_if(process_list.begin(), process_list.end(), [=](const Process& process) {
                        return process.PID == process_queue[0].PID;
                    });
                size_t index = std::distance(process_list.begin(), it);
                if (i == index)
                   scene->addRect(500 + time*scale, 50 + i * (rectangleSize + 5), scale/10, rectangleSize, QPen(Qt::green), QBrush(Qt::green));
                else
                    scene->addRect(500 + time*scale, 50 + i * (rectangleSize + 5), scale/10, rectangleSize, QPen(Qt::gray), QBrush(Qt::gray));
            }
        }
    }
}

void MainWindow::drawTimeAxis()
{
    time += 0.1; // L'exécution s'arrête avant la dernière incrémentation

    // Dessin de l'axe du temps
    QGraphicsLineItem *axe = new QGraphicsLineItem(500, 50 + process_list.size()*(rectangleSize + 5), 500 + time*scale, 50 + process_list.size()*(rectangleSize + 5));
    scene->addItem(axe);

    // Ajouter des marqueurs de temps sur l'axe
    for (int t = 0; t <= time; t++)
    {
        QGraphicsTextItem *timeMarker = new QGraphicsTextItem(QString::number(t));
        timeMarker->setPos(500 + scale*t - 10, 55 + process_list.size()*(rectangleSize + 5));
        scene->addItem(timeMarker);
    }

    // Ajouter les étiquettes des processus
    for (size_t i = 0; i < process_list.size(); i++)
    {
        QGraphicsTextItem *label = new QGraphicsTextItem("Process " + QString::number(i+1));
        label->setPos(500 + time*scale + 20, 50 + i * (rectangleSize + 5) + rectangleSize/4);
        scene->addItem(label);
    }

    saveImage();
}

void MainWindow::saveImage()
{
    QImage *image = new QImage(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image->fill(Qt::white); // Fond blanc

    QPainter *painter = new QPainter(image);
    scene->render(painter);
    painter->end();

    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    QString saveAlgo;
    switch (algorithms->currentIndex())
    {
    case 1:
        saveAlgo = "FCFS ";
        break;
    case 2:
        saveAlgo = "SJF ";
        break;
    case 3:
        saveAlgo = "SRTF ";
        break;
    case 4:
        saveAlgo = "PS ";
        break;
    case 5:
        saveAlgo = "Round Robin ";
        break;
    case 6:
        saveAlgo = "CFS ";
        break;
    default:
        break;
    }
    image->save("/home/n/Images/Simulation " + saveAlgo + QString(ctime(&currentTime)) + ".png");
}


void sort_process(sort_type type, vector <Process>& process_list, vector <QProgressBar*> &progressBars, vector <QLabel*> &labels, vector <QLabel*> &states)
{
// Création d'un vecteur d'indices
    vector<size_t> indices(process_list.size());
    for (size_t i = 0; i < indices.size(); ++i)
        indices[i] = i;

    // Réarrangement des trois vecteurs selon l'ordre des indices triés
    vector<Process> sorted_v1(process_list.size());
    vector<QProgressBar*> sorted_v2(progressBars.size());
    vector<QLabel*> sorted_v3(labels.size());
    vector<QLabel*> sorted_v4(states.size());

    switch(type)
    {
    case ARRIVAL_TIME:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return (process_list[i].arrival_time < process_list[j].arrival_time
                    || (process_list[i].arrival_time == process_list[j].arrival_time
                        && process_list[i].priority < process_list[j].priority));
        });
        break;
    case BURST_TIME:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return (process_list[i].burst_time < process_list[j].burst_time
                    || (process_list[i].burst_time == process_list[j].burst_time
                        && process_list[i].priority < process_list[j].priority));
        });
        break;
    case REMAINING_TIME:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return (process_list[i].remaining_time < process_list[j].remaining_time
                    || (process_list[i].remaining_time == process_list[j].remaining_time
                        && process_list[i].priority < process_list[j].priority));
        });
        break;
    case PRIORITY:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            return process_list[i].priority < process_list[j].priority;
        });
        break;
    case PROCESSOR_TIME:
        sort(indices.begin(), indices.end(), [&](size_t i, size_t j) {
            int processorTimei = process_list[i].burst_time - process_list[i].remaining_time;
            int processorTimej = process_list[j].burst_time - process_list[j].remaining_time;
            return (processorTimei < processorTimej
                    || (processorTimei == processorTimej && process_list[i].priority < process_list[j].priority));
        });
        break;
    default:
        break;
    }

    for (size_t i = 0; i < indices.size(); ++i) {
        sorted_v1[i] = process_list[indices[i]];
        sorted_v2[i] = progressBars[indices[i]];
        sorted_v3[i] = labels[indices[i]];
        sorted_v4[i] = states[indices[i]];
    }

    process_list = sorted_v1;
    progressBars = sorted_v2;
    labels = sorted_v3;
    states = sorted_v4;
}
