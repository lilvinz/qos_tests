/*
 * network.c
 *
 *  Created on: 04.03.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "network.h"

NWK_T _network;


void NETWORK_SetType(NWK_NETWORK_TYPE_T type)
{
    _network.type = type;
}

void NETWORK_SetID(NWK_NETWORK_ID_T id)
{
    _network.id = id;
}

NWK_NETWORK_TYPE_T NETWORK_GetType(void)
{
    return _network.type;
}

NWK_NETWORK_ID_T NETWORK_GetID(void)
{
    return _network.id;
}
