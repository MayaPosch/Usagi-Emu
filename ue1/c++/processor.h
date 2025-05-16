/*
	processor.h - UE1 processor loop definition.
	
	Revision 0.
	
*/


#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <fstream>
#include <atomic>


extern std::atomic_bool stop;
extern std::atomic_bool halt;
extern std::atomic<float> speed;
extern std::atomic_uint32_t delay;


int run_program(std::ifstream& asmfile);


#endif
