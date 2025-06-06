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


// Flags
extern std::atomic_bool flag0;
//extern std::atomic_bool flagf 	= { false };
extern std::atomic_bool wrt;
extern std::atomic_bool skz;
extern std::atomic_bool ioc;
extern std::atomic_bool rtn;


// Registers
extern std::atomic_uint8_t carry;
extern std::atomic_uint8_t rr;
extern std::atomic_uint8_t ien;
extern std::atomic_uint8_t oen;
extern std::atomic_uint8_t outrg;
extern std::atomic_uint8_t sctrg;
extern std::atomic_uint8_t inprg;


extern std::string mema; // Memory address.
extern std::string opco; // Opcode.


int run_program(std::ifstream& asmfile);


#endif
