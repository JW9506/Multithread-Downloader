#pragma once

typedef void (*RunFunc)(void*);
#define RUNNABLE(run_func, arg)                                                \
    &(Runnable) { .run = (RunFunc)run_func, .argument = arg }

// Like a tuple
typedef struct Runnable {
    RunFunc run;
    void* argument;
} Runnable;

void InitThreadPool();
void DestroyThreadPool();
void ThreadPoolExecute(Runnable* runnable);
