#ifndef MESH_TESTMODE_H
#define MESH_TESTMODE_H


typedef enum
{
    MESH_TESTMODE_CCTM,
} MESH_TESTMODE_T;

bool MESH_TestmodeStart(MESH_TESTMODE_T mode);
void MESH_TestmodeStop(void);
bool MESH_TestmodeSetAntenna(uint8_t ant);
bool MESH_TestmodeSetTxPower(uint8_t pwr);
bool MESH_TestmodeSetTrim(uint8_t trim);
bool MESH_TestmodeSetChannel(uint8_t chan);
bool MESH_TestmodeSetFrameBufferPattern(uint8_t pat);
bool MESH_TestmodeSaveSettings(void);

#endif
