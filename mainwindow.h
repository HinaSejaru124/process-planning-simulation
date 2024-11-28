#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QSpinBox>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <process>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    vector <Process> process_list;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    // Instanciation des 3 pages de la fenêtre
    QWidget *processes = new QWidget(this);
    QScrollArea *simulation = new QScrollArea(this);
    QWidget *gantt = new QWidget(this);

    // Instanciation des widgets de portée globale
    QScrollArea *processListScrollArea = new QScrollArea;
    QTableWidget *processTable = new QTableWidget(processListScrollArea);
    QComboBox *algorithms = new QComboBox;
    QSpinBox *quantumBox = new QSpinBox;

    // Fenêtre pop-up
    QWidget *customProcessWidget;
    QLineEdit *arrivalTimeInput = new QLineEdit(customProcessWidget);
    QLineEdit *burstTimeInput = new QLineEdit(customProcessWidget);
    QLineEdit *priorityInput = new QLineEdit(customProcessWidget);

    // Instanciation des boutons de la fenêtre
    QPushButton *addRandomProcess = new QPushButton;
    QPushButton *addCustomProcess = new QPushButton;
    QPushButton *simulate = new QPushButton;
    QPushButton *createCustomProcess = new QPushButton;

    // Layout de la deuxième page
    QVBoxLayout *simulationLayout = new QVBoxLayout;
    QWidget *simulationContainer;
    QVBoxLayout *simulationContainerLayout;

    // Layout de la troisième page
    QVBoxLayout *ganttLayout = new QVBoxLayout(gantt);
    QScrollArea *scrollArea;
    QGraphicsView *view;
    QGraphicsScene *scene;
    vector <QGraphicsRectItem*> ganttBars;

    // Paramètres globaux des algorithmes de planification
    vector <Process> process_queue;
    vector <Process> completed;
    vector <QProgressBar*> processQueueProgressBars;
    vector <QLabel*> processQueueRemainingTime;
    vector <QLabel*> processQueueStateLabel;
    QTimer *timer;
    float time = 0;
    size_t nb_processed = 0;
    bool all_complete = false;

    void enAttente(); // Rechercher les processsus arrivants
    void afterSimulation(QStackedWidget *pages); // Fonction à exécuter après la simulation
    void drawGanttBars(); // Dessiner le diagramme de GANTT
    void drawTimeAxis(); // Dessiner l'axe du temps, ainsi que le texte du diagramme
    void saveImage(); // Enregistrer le diagramme de GANTT sous forme d'image

private slots:
    void addRandomProcessClicked(); // Ajouter un processus quelconque à la liste des processus
    void addCustomProcessClicked(); // Créer un widget pour ajouter un processus personnalisé à la liste des processus
    void checkCreateCustomProcess(); // Vérifier les entrées des champs du processus personnalisé
    void createCustomProcessClicked(); // Ajouter le processus personnalisé à la liste des processus
    void simulateClicked(QStackedWidget *pages); // Simuler l'exécution des processus instanciés
    void checkCombo(); // Vérifier si la simulation peut être lancée
    void FCFS(QStackedWidget *pages); // Simulation de l'algorithme FCFS
    void SJF(QStackedWidget *pages); // Simulation de l'algorithme SJF
    void SRTF(QStackedWidget *pages); // Simulation de l'algorithme SRTF
    void PS(QStackedWidget *pages); // Simulation de l'algorithme Priority Scheduling
    void RoundRobin(QStackedWidget *pages); // Simulation de l'algorithme Round Robin
    void CFS(QStackedWidget *pages); // Simulation de l'algorithme CFS
};

void sort_process(sort_type type, vector <Process>& process_list, vector <QProgressBar*> &progressBars
                  , vector <QLabel*> &labels, vector <QLabel*> &states); // Pour trier les différents vecteurs reliés aux processus

static const int rectangleSize = 40;
static const int scale = 25;

#endif // MAINWINDOW_H
