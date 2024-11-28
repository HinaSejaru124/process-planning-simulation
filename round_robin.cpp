#include "mainwindow.h"

void MainWindow::RoundRobin(QStackedWidget *pages)
{
    static int time_executed = 0;
    if (nb_processed != process_list.size())
    {
        enAttente();
        if (process_queue.size() != 0)
        {
            process_queue[0].process_execution(0.1);
            time_executed++;
            processQueueProgressBars.at(0)->setValue((process_queue[0].burst_time - process_queue[0].remaining_time)/process_queue[0].burst_time * 100);
            processQueueRemainingTime.at(0)->setText("Remaining time: " + QString::number(process_queue[0].remaining_time) + "s");
            processQueueStateLabel.at(0)->setText("État: En cours de traitement");
            if (process_queue[0].complete)
            {
                nb_processed++;
                processQueueStateLabel.at(0)->setText("État: Terminé");
                completed.push_back(process_queue.at(0));
                process_queue.erase(process_queue.begin());
                processQueueProgressBars.erase(processQueueProgressBars.begin());
                processQueueRemainingTime.erase(processQueueRemainingTime.begin());
                processQueueStateLabel.erase(processQueueStateLabel.begin());
                time_executed = 0;
            }
            if (time_executed == quantumBox->value()*10)
            {
                processQueueStateLabel.at(0)->setText("État: En attente");
                process_queue.push_back(process_queue[0]);
                process_queue.erase(process_queue.begin());
                processQueueProgressBars.push_back(processQueueProgressBars[0]);
                processQueueProgressBars.erase(processQueueProgressBars.begin());
                processQueueRemainingTime.push_back(processQueueRemainingTime[0]);
                processQueueRemainingTime.erase(processQueueRemainingTime.begin());
                processQueueStateLabel.push_back(processQueueStateLabel.at(0));
                processQueueStateLabel.erase(processQueueStateLabel.begin());
                time_executed = 0;
            }
        }
        time += 0.1;
        time = round(time * 10)/10;
    }
    else
    {
        timer->stop();
        afterSimulation(pages);
    }
}
