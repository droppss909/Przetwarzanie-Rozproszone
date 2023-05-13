#ifndef UTILH
#define UTILH
#include "main.hpp"

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
            return ts > rhs.ts;
        else
            return rank > rhs.rank;
    }
};

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;
    int hotelNo; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    color_t color;
    
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 5

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK 1
#define REQUEST 2
#define RELEASE 3
#define ACK_G 4
#define REQUEST_G 5
#define RELEASE_G 6
#define APP_PKT 7
#define FINISH 8

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    InRun,
    InMonitor,
    InWant,
    InSection,
    InFinish,
    Rest,
    WaitHotel,
    WaitGuide,
    Trip,
    Cleaning
} state_t;

extern state_t stan;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t lamportMut;
extern pthread_mutex_t queueMut;
extern pthread_mutex_t guideMut;
extern pthread_mutex_t ackMut;
extern pthread_mutex_t ackMut_g;

void changeState(state_t);
int cmp(q_item_t const &lhs, q_item_t const &rhs);
#endif
