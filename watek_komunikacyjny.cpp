#include "main.hpp"
#include "watek_komunikacyjny.hpp"
#include "util.hpp"
#include <algorithm>

void *startKomWatek(void *ptr)
{
    MPI_Status status;
    packet_t pakiet;
    while (stan != InFinish)
    {
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&lamportMut);
        if (pakiet.ts > lamport)
            lamport = pakiet.ts;
        lamport++;
        pthread_mutex_unlock(&lamportMut);

        switch (status.MPI_TAG)
        {
        case REQUEST:
            // println("dostałem REQUEST[%d] od %d", pakiet.hotelNo, pakiet.src);
            pthread_mutex_lock(&queueMut);
            hotelQueues[pakiet.hotelNo].push_back(q_item_t{pakiet.ts, pakiet.color, pakiet.src});
            std::sort(hotelQueues[pakiet.hotelNo].begin(), hotelQueues[pakiet.hotelNo].end(), cmp);
            pthread_mutex_unlock(&queueMut);
            sendPacket(0, status.MPI_SOURCE, ACK);
            break;
        case ACK:
            pthread_mutex_lock(&ackMut);
            ackCount++;
            // println("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            pthread_mutex_unlock(&ackMut);
            break;
        case RELEASE:
        {
            // println("Dostałem RELEASE[%d] od [%d]", pakiet.hotelNo, pakiet.src);
            std::priority_queue<q_item_t> tempQueue;
            pthread_mutex_lock(&queueMut);
            for (std::deque<q_item_t>::iterator it = hotelQueues[pakiet.hotelNo].begin(); it != hotelQueues[pakiet.hotelNo].end(); ++it)
            {
                if (it->rank == pakiet.src)
                {
                    hotelQueues[pakiet.hotelNo].erase(it);
                    break;
                }
            }
            pthread_mutex_unlock(&queueMut);
            break;
        }
        case REQUEST_G:
        {
            pthread_mutex_lock(&guideMut);
            guideQueue.push_back(q_item_t{pakiet.ts,pakiet.color, pakiet.src});
            std::sort(guideQueue.begin(), guideQueue.end());
            pthread_mutex_unlock(&guideMut);
            sendPacket(0,status.MPI_SOURCE,ACK_G);
            break;
        }
        case ACK_G:
        {
            pthread_mutex_lock(&ackMut_g);
            ackCount_g++;
            pthread_mutex_unlock(&ackMut_g);
            break;
        }
        case RELEASE_G:
        {
            pthread_mutex_lock(&guideMut);
            for(std::deque<q_item_t>::iterator it = guideQueue.begin(); it != guideQueue.end(); ++it){
                if(it->rank == pakiet.src){
                    guideQueue.erase(it);
                    break;
                }
            }
            pthread_mutex_unlock(&guideMut);
        }

        default:
            break;
        }
    }
}
