// tracer-bakes playground: the one change to the engine's execution. Every
// place the engine sized its worker pool by std::thread::hardware_concurrency()
// now asks tracer_worker_count(), which returns TRACER_THREADS when that is set
// to a positive number and the hardware count otherwise. Nothing else about
// the engine's scheduling changes.
#pragma once
#include <cstdlib>
#include <thread>

inline unsigned tracer_worker_count(){
    const unsigned hardware=std::thread::hardware_concurrency();
    const char* text=std::getenv("TRACER_THREADS");
    if(text==nullptr)return hardware;
    const long wanted=std::strtol(text,nullptr,10);
    return wanted>0?static_cast<unsigned>(wanted):hardware;}
