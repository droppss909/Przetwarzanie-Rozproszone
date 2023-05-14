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
        lamportMut.lock();
        if (pakiet.ts > lamport)
            lamport = pakiet.ts;
        lamport++;
        lamportMut.unlock();

        switch (status.MPI_TAG)
        {
        case REQUEST_H:
            // println("dostałem REQUEST_H[%d] od %d", pakiet.hotelNo, pakiet.src);
            hotelMut.lock();
            hotelQueues[pakiet.hotelNo].push_back(q_item_t{pakiet.ts, pakiet.color, pakiet.src});
            std::sort(hotelQueues[pakiet.hotelNo].begin(), hotelQueues[pakiet.hotelNo].end());
            hotelMut.unlock();
            sendPacket(0, status.MPI_SOURCE, ACK);
            break;
        case ACK:
            ackMut.lock();
            ackCount++;
            // println("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackMut.unlock();
            ackCond.notify_all();
            break;
        case RELEASE_H:
        {
            // println("Dostałem RELEASE_H[%d] od [%d]", pakiet.hotelNo, pakiet.src);
            hotelMut.lock();
            for (std::deque<q_item_t>::iterator it = hotelQueues[pakiet.hotelNo].begin(); it != hotelQueues[pakiet.hotelNo].end(); ++it)
            {
                if (it->rank == pakiet.src)
                {
                    hotelQueues[pakiet.hotelNo].erase(it);
                    break;
                }
            }
            hotelMut.unlock();
            hotelCond.notify_one();
            break;
        }
        case REQUEST_G:
        {
            // println("Dostałem REQUEST_G od [%d]", pakiet.src);
            guideMut.lock();
            guideQueue.push_back(q_item_t{pakiet.ts, pakiet.color, pakiet.src});
            std::sort(guideQueue.begin(), guideQueue.end());
            guideMut.unlock();
            sendPacket(0, status.MPI_SOURCE, ACK);
            break;
        }
        case RELEASE_G:
        {
            // println("Dostałem RELEASE_G od [%d]", pakiet.src);
            guideMut.lock();
            for (std::deque<q_item_t>::iterator it = guideQueue.begin(); it != guideQueue.end(); ++it)
            {
                if (it->rank == pakiet.src)
                {
                    guideQueue.erase(it);
                    break;
                }
            }
            guideMut.unlock();
            guideCond.notify_one();
        }

        default:
            break;
        }
    }
    pthread_exit(NULL);
}
