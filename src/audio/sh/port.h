#ifndef AUDIO_PORT_H
#define AUDIO_PORT_H

#include <PR/ultratypes.h>

extern OSMesg OSMesg0[1];
extern OSMesgQueue OSMesgQueue0Data;
extern OSMesgQueue *OSMesgQueue0;

extern OSMesg OSMesg1[4];
extern OSMesgQueue OSMesgQueue1Data;
extern OSMesgQueue *OSMesgQueue1;

extern OSMesg OSMesg2[1];
extern OSMesgQueue OSMesgQueue2Data;
extern OSMesgQueue *OSMesgQueue2;

extern OSMesg OSMesg3[1];
extern OSMesgQueue OSMesgQueue3Data;
extern OSMesgQueue *OSMesgQueue3;

// main thread -> sound thread dispatchers
void port_cmd_f32(u32 arg0, f32 arg1);
void port_cmd_u32(u32 arg0, u32 arg1);
void port_cmd_s8(u32 arg0, s8 arg1);

void func_802ad7a0(void);

#endif // AUDIO_PORT_H
