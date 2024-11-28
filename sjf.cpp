#include "mainwindow.h"

void MainWindow::SJF(QStackedWidget *pages)
{
    bool sE = true; // Si c'est la 1e exécution
    if (nb_processed != process_list.size())
    {
        enAttente();
        if (process_queue.size() != 0)
        {
            while (sE)
                for (size_t i = 0; i < process_queue.size() - 1; i++)
                    if (process_queue[i].arrival_time != process_queue[i+1].arrival_time)
                    {
                        sE = false;
                        break;
                    }
            if (sE)
                sort_process(BURST_TIME, process_queue, processQueueProgressBars, processQueueRemainingTime, processQueueStateLabel);

            process_queue[0].process_execution(0.1);
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
                sort_process(BURST_TIME, process_queue, processQueueProgressBars, processQueueRemainingTime, processQueueStateLabel);
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
