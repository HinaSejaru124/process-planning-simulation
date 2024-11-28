#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <cmath>
#include <QProgressBar>
#include <QLabel>

using namespace std;

#define PRIORITY_RANGE 9
#define BURST_TIME_RANGE 10
#define ARRIVAL_TIME_RANGE 10

static int process_number = 0; // process_number of process initializations
enum sort_type {ARRIVAL_TIME, REMAINING_TIME, BURST_TIME, PRIORITY, PROCESSOR_TIME};
class Process
{
public:
    int PID;
    int arrival_time;
    int priority;
    float burst_time;
    float remaining_time;
    bool complete;

    // Constructor
    Process()
    {
        PID = ++process_number;
        arrival_time = rand() % (ARRIVAL_TIME_RANGE + 1);
        burst_time = rand() % BURST_TIME_RANGE + 1;
        remaining_time = burst_time;
        priority = rand() % PRIORITY_RANGE  + 1;
        complete = false;
    }

    Process(int at, int bt, int p): arrival_time(at), priority(p), burst_time(bt)
    {
        PID = ++process_number;
        remaining_time = burst_time;
        complete = false;
    }

    void process_execution(float time)
    {
        if (remaining_time <= time) // S'il lui est accordé plus de temps CPU qu'il ne lui reste à exécuter
        {
            remaining_time = 0; // Considère le processus comme terminé
            complete = true;
        }
        else
        {
            remaining_time -= time; // Sinon soustrais lui le temps CPU accordé
            remaining_time = round(remaining_time * 10)/10; // À 0.1s près
        }
    }

    bool operator == (Process process)
    {
        return this->PID == process.PID;
    }
};
#endif // PROCESS_H
