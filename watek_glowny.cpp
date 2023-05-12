#include "main.hpp"
#include "watek_glowny.hpp"
#include <queue>

void mainLoop()
{
	srandom(rank);
	int perc;
	int hotelNo;

	while (stan != InFinish)
	{
		switch (stan)
		{
		case InRun:
			perc = random() % 100;
			if (perc < 80)
			{
				hotelNo = random() % H;
				println("Ubiegam się o miejsce w hotelu %d", hotelNo);
				packet_t *pkt = new packet_t;
				pkt->hotelNo = hotelNo;
				pkt->color = color;
				pthread_mutex_lock(&ackMut);
				ackCount = 0;
				pthread_mutex_unlock(&ackMut);
				for (int i = 0; i <= size - 1; i++)
					sendPacket(pkt, i, REQUEST);
				changeState(InWant);
				delete (pkt);
			}
			break;
		case InWant:
			if (ackCount == size)
			{
				std::priority_queue<q_item_t> tempQueue;
				pthread_mutex_lock(&queueMut);
				for (int i = 0; i < hotelQueues[hotelNo].size() && i < R; i++)
				{
					if (hotelQueues[hotelNo].at(i).rank == rank)
					{
						changeState(InSection);
						break;
					}
					if (hotelQueues[hotelNo].at(i).color != color)
					{
						break;
					}
				}
				pthread_mutex_unlock(&queueMut);
			}
			break;
		case InSection:
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w hotelu %d", hotelNo);
			sleep(5);
			println("Wychodzę z sekcji krytycznej");
			packet_t *pkt = new packet_t;
			pkt->hotelNo = hotelNo;
			for (int i = 0; i <= size - 1; i++)
				sendPacket(pkt, i, RELEASE);
			changeState(InRun);
			delete (pkt);
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
