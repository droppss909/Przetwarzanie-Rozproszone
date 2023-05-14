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
		case Rest:
			perc = random() % 100;
			if (color == Cleaner)
				sleep(5);
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
					sendPacket(pkt, i, REQUEST_H);
				changeState(WaitHotel);
				delete (pkt);
			}
			break;
		case WaitHotel:
			pthread_mutex_lock(&ackMut);
			if (ackCount == size)
			{
				pthread_mutex_lock(&hotelMut);
				for (unsigned long i = 0; i < hotelQueues[hotelNo].size() && i < R; i++)
				{
					if (hotelQueues[hotelNo].at(i).rank == rank)
					{
						if (color == Cleaner)
							changeState(Cleaning);
						else
							changeState(InHotel);
						break;
					}
					if (hotelQueues[hotelNo].at(i).color != color)
					{
						break;
					}
				}
				pthread_mutex_unlock(&hotelMut);
			}
			pthread_mutex_unlock(&ackMut);
			break;
		case InHotel:
		{
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w hotelu %d i ubiegam sie o przewodnika", hotelNo);
			pthread_mutex_lock(&ackMut);
			ackCount = 0;
			pthread_mutex_unlock(&ackMut);
			packet_t *pkt = new packet_t;
			for (int i = 0; i <= size - 1; i++)
			{
				sendPacket(pkt, i, REQUEST_G);
			}
			changeState(WaitGuide);
			delete (pkt);

			break;
		}
		case Cleaning:
		{
			println("Sprztam hotel: %d", hotelNo);
			sleep(1);
			println("Wychodzę z sekcji krytycznej");
			packet_t *pkt = new packet_t;
			pkt->hotelNo = hotelNo;
			for (int i = 0; i <= size - 1; i++)
				sendPacket(pkt, i, RELEASE_H);
			changeState(Rest);
			delete (pkt);
			break;
		}
		case WaitGuide:
		{
			pthread_mutex_lock(&ackMut);
			if (ackCount == size)
			{
				pthread_mutex_lock(&guideMut);
				for (unsigned long i = 0; i < guideQueue.size() && i < G; i++)
				{
					if (guideQueue.at(i).rank == rank)
					{
						changeState(Trip);
						break;
					}
				}
				pthread_mutex_unlock(&guideMut);
			}
			pthread_mutex_unlock(&ackMut);
		}
		case Trip:
		{
			println("Jestem na wycieczce");
			sleep(2);
			println("wychodze z sekcji rytycznej");
			packet_t *pkt = new packet_t;
			pkt->hotelNo = hotelNo;
			for (int i = 0; i <= size - 1; i++)
				sendPacket(pkt, i, RELEASE_G);

			for (int i = 0; i <= size - 1; i++)
				sendPacket(pkt, i, RELEASE_H);
			changeState(Rest);
			delete (pkt);
			break;
		}
		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
