#ifndef UTILH
#define UTILH
#include "main.hpp"
#include <mutex>
#include <condition_variable>

// Parametry programu
#define H 1
#define R 2
#define G 2

// Kolory Kosmitów
typedef enum
{
    Blue,
    Purple,
    Cleaner
} color_t;

// Element kolejki
struct q_item_t
{
    int ts;
    color_t color;
    int rank;
    bool operator<(const q_item_t &rhs) const
    {
        if (ts != rhs.ts)
            return ts < rhs.ts;
        else
            return rank > rhs.rank;
    }
};

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;
    int hotelNo; /* Numer hotelu, do którego wchodzi proces*/
    color_t color;

} packet_t;
#define NITEMS 4

/* TYPY PAKIETÓW */
#define ACK 1
#define REQUEST_H 2
#define RELEASE_H 3
#define REQUEST_G 4
#define RELEASE_G 5
#define FINISH 6

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    Rest,
    WaitHotel,
    InHotel,
    InFinish,
    WaitGuide,
    Trip,
    Cleaning
} state_t;

extern state_t stan;

extern std::mutex stateMut;
extern std::mutex lamportMut;
extern std::mutex hotelMut;
extern std::mutex guideMut;
extern std::mutex ackMut;
extern std::condition_variable ackCond;
extern std::condition_variable hotelCond;
extern std::condition_variable guideCond;

void changeState(state_t);
int cmp(q_item_t const &lhs, q_item_t const &rhs);
#endif
