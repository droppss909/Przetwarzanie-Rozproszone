#include "main.hpp"

#include "watek_glowny.hpp"

#include <queue>

void mainLoop()
{
	srandom(rank);
	int perc;
	int hotelNo;
	int threshold = color == Cleaner ? 15 : 40;

	while (stan != InFinish)
	{
		switch (stan)
		{
		case Rest:
			perc = random() % 100;
			if (perc < threshold)
			{
				hotelNo = random() % H;
				println("Ubiegam się o miejsce w hotelu %d", hotelNo);
				packet_t *pkt = new packet_t;
				pkt->hotelNo = hotelNo;
				pkt->color = color;
				ackMut.lock();
				ackCount = 0;
				ackMut.unlock();
				for (int i = 0; i <= size - 1; i++)
					sendPacket(pkt, i, REQUEST_H);
				changeState(WaitHotel);
				delete (pkt);
			}
			break;
		case WaitHotel:
		{
			std::unique_lock<std::mutex> ackLock{
				ackMut};
			ackCond.wait(ackLock, [&]()
						 { return ackCount == size; });
			ackLock.unlock();

			std::unique_lock<std::mutex> hotelLock{
				hotelMut};
			if (color!= Cleaner){
				hotelCond.wait(hotelLock, [&]()
							{
								for (unsigned long i = 0; i < hotelQueues[hotelNo].size() && i < R; i++)
									if (hotelQueues[hotelNo].at(0).color!= Cleaner && hotelQueues[hotelNo].at(i).rank == rank)
										return true;
									else if (hotelQueues[hotelNo].at(i).color!=Cleaner && hotelQueues[hotelNo].at(i).color != color)
										return false;

								return false; });
			}
			else{
				hotelCond.wait(hotelLock, [&]()
							{
								if (hotelQueues[hotelNo].at(0).rank==rank){
								return true;
								}});
			}

			if (color == Cleaner)
				changeState(Cleaning);
			else
				changeState(InHotel);
			break;
		}
		case InHotel:
		{
			// tutaj zapewne jakiś muteks albo zmienna warunkowa
			println("Jestem w hotelu %d i ubiegam sie o przewodnika", hotelNo);
			ackMut.lock();
			ackCount = 0;
			ackMut.unlock();
			packet_t *pkt = new packet_t;
			for (int i = 0; i <= size - 1; i++)
				sendPacket(pkt, i, REQUEST_G);
			changeState(WaitGuide);
			delete (pkt);

			break;
		}
		case Cleaning:
		{
			println("Sprztam hotel: %d", hotelNo);
			sleep(3);
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
			std::unique_lock<std::mutex> ackLock{
				ackMut};
			ackCond.wait(ackLock, [&]()
						 { return ackCount == size; });
			ackLock.unlock();

			std::unique_lock<std::mutex> guideLock{
				guideMut};
			hotelCond.wait(guideLock, [&]()
						   {
							   for (unsigned long i = 0; i < guideQueue.size() && i < G; i++)
								   if (guideQueue.at(i).rank == rank)
									   return true;

							   return false; });
			changeState(Trip);
		}
		case Trip:
		{
			println("Jestem na wycieczce");
			sleep(3);
			println("wychodze z sekcji krytycznej");
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