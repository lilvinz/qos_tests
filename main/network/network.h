#ifndef NETWORK_H
#define NETWORK_H


typedef enum
{
    NWK_TYPE_NONE,
    NWK_TYPE_MESH,
} NWK_NETWORK_TYPE_T;

typedef uint16_t NWK_NETWORK_ID_T;

typedef struct
{
    NWK_NETWORK_TYPE_T  type;
    NWK_NETWORK_ID_T    id;
} NWK_T;

extern NWK_T _network;


void NETWORK_SetType(NWK_NETWORK_TYPE_T type);
void NETWORK_SetID(NWK_NETWORK_ID_T id);
NWK_NETWORK_TYPE_T NETWORK_GetType(void);
NWK_NETWORK_ID_T NETWORK_GetID(void);


#endif
