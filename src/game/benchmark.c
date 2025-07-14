#include <ultra64.h>

#include "cfg/benchmark.h"

#include "game_init.h"
#include "n64-stdio.h"

#ifdef CFG_BENCHMARK

#define REPLAY_FILE "data/benchmark_replay.m64"
#define REPLAY_HEADER_SIZE 0x400
#define REPLAY_FRAME_SIZE sizeof(ReplayContPad)

typedef struct {
	u16     button;
	s8      stickX;
	s8      stickY;
} ReplayContPad;

static alignas(32) u8 replayData[] = {
    #embed REPLAY_FILE
};

static u8 *replayDataPtr = &replayData[0] + REPLAY_HEADER_SIZE;
static u8 *replayDataEnd = &replayData[0] + (sizeof(replayData));
static u32 benchmarkFrame = 0;

void replay_contpad(OSContPad *pad) {
    ReplayContPad *input = (ReplayContPad *) replayDataPtr;

    if (replayDataPtr >= replayDataEnd) {
        return;
    }

    pad->button =  input->button;
    pad->stick_x = input->stickX;
    pad->stick_y = input->stickY;

    replayDataPtr += REPLAY_FRAME_SIZE;
}

void replay_print_stats() {
    OSTime finalTime = osGetTime();
    n64_printf("BENCHMARK END\n");
    n64_printf("Time to complete: %llu us\n", OS_CYCLES_TO_USEC(finalTime));
    n64_printf("Average FPS: %.2f", (f32)gGlobalTimer * 1000000.0f / OS_CYCLES_TO_USEC(finalTime));
}
#endif
