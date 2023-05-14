#include "main.hpp"
#include "util.hpp"
MPI_Datatype MPI_PAKIET_T;

/*
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan = Rest;

/* zamek wokół zmiennej współdzielonej między wątkami.
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
std::mutex stateMut;
std::mutex lamportMut;
std::mutex hotelMut;
std::mutex guideMut;
std::mutex ackMut;
std::condition_variable ackCond;
std::condition_variable hotelCond;
std::condition_variable guideCond;

struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {{"finish", FINISH}, {"potwierdzenie", ACK}, {"prośbę o sekcję krytyczną", REQUEST_H}, {"zwolnienie sekcji krytycznej", RELEASE_H}, {"prosba o przewodnika", REQUEST_G}, {"Zezwolenie na podroz", RELEASE_G}};

const char *const tag2string(int tag)
{
    for (long unsigned int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
 */
void inicjuj_typ_pakietu()
{
    int blocklengths[NITEMS] = {1, 1, 1, 1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, hotelNo);
    offsets[3] = offsetof(packet_t, color);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    bool freepkt = false;
    if (pkt == 0)
    {
        pkt = new packet_t;
        freepkt = true;
    }
    pkt->src = rank;
    lamportMut.lock();
    pkt->ts = ++lamport;
    lamportMut.unlock();
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freepkt)
        delete (pkt);
}

void changeState(state_t newState)
{
    stateMut.lock();
    if (stan == InFinish)
    {
        stateMut.unlock();
        return;
    }
    stan = newState;
    stateMut.unlock();
}