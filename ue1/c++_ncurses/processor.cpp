/*
	processor.cpp - UE1 processor loop implementation.
	
	Revision 0.
*/


#include "processor.h"

#include <chrono>
#include <thread>
#include <iostream>


std::atomic_bool stop 	= { false };
std::atomic_bool halt 	= { false };
std::atomic<float> speed	= { 10.0 };
std::atomic_uint32_t delay 	= { 0 };


// Flags
std::atomic_bool flag0 	= { false };
std::atomic_bool flagf 	= { false };
std::atomic_bool wrt 	= { false };
std::atomic_bool skz 	= { false };
std::atomic_bool ioc 	= { false };
std::atomic_bool rtn 	= { false };


// Registers
std::atomic_uint8_t carry	= { 0 };
std::atomic_uint8_t rr		= { 0 };
std::atomic_uint8_t ien		= { 0 };
std::atomic_uint8_t oen		= { 0 };
std::atomic_uint8_t outrg	= { 0 };
std::atomic_uint8_t sctrg	= { 0 };
std::atomic_uint8_t inprg	= { 0 };


// Temporary holding cells.
std::atomic_uint8_t tprr	= { 0 };
std::atomic_uint8_t tpdb	= { 0 };
std::atomic_uint8_t tpreg	= { 0 };
std::atomic_uint8_t trgt1	= { 0 };
std::atomic_uint8_t trgt2	= { 0 };
std::atomic_uint8_t tpbit	= { 0 };
std::atomic_uint8_t inloc	= { 13 };
std::atomic_uint8_t inbit	= { 0 };
std::atomic_uint8_t tsbit	= { 0 };


std::string mema; // Memory address.
std::string opco; // Opcode.


// --- END CYCLE ---
// Called at the end of a CPU cycle. Inserts appropriate delay for the target clock speed.
void end_cycle() {
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}


void update_display();
void do_beep();


int run_program(std::ifstream& asmfile) {
	// Parse the file one line at a time and run the main processing loop.
	std::string line;
	while (!stop) {
		if (halt || flagf == 1) {
			// We're in halted state. Idle here until halted state is cancelled.
			halt = true;
			end_cycle();
			update_display();
			continue;
		}
		
		// Skip this instruction [cycle?] if last instruction was SKZ & RR = 0.
		if (skz && (rr == 0)) {
			skz = false;
			end_cycle();
			continue;
		}
		
		// Skip this instruction [cycle?] if last instruction was RTN.
		if (rtn) {
			rtn = false;
			end_cycle();
			continue;
		}
		
		// Fetch next line with opcode & memory address.
		while (1) {
			// if we're at EOF, return the cursor to the beginning.
			if (asmfile.eof()) {
				asmfile.seekg(0);
			}
			
			std::getline(asmfile, line);
			
			// Split line into opcode (4 characters) & memory address (3 chars).
			// Both are separated by two spaces.
			// Skip if starting with ';'.
			if (line[0] == ';' || line.size() < 7) { continue; }
			opco = line.substr(0, 4);
			mema = line.substr(5, 3);
			
			// Trim trailing whitespace on mem address.
			while (mema[mema.size() - 1] == ' ') { mema.erase(mema.end() - 1); }
			
			break;
		}
		
		// Reset flags.
		flag0 = false;
		flagf = false;
		wrt = false;
		ioc = false;
		trgt1 = false;
		trgt2 = false;
		
		// Set up the target memory location.
		// trgt1 => the specific bit we want to focus on.
		// trgt2 => the register want to work with (1 = scratch, 2 = output).
		if (mema == "SR0") {
			trgt1 = 1;
			trgt2 = 1;
			tpbit = (sctrg & 1);
		}
		else if (mema == "SR1") {
			trgt1 = 2;
			trgt2 = 1;
			tpbit = (sctrg & 2) / 2;
		}
		else if (mema == "SR2") {
			trgt1 = 4;
			trgt2 = 1;
			tpbit = (sctrg & 4) / 4;
		}
		else if (mema == "SR3") {
			trgt1 = 8;
			trgt2 = 1;
			tpbit = (sctrg & 8) / 8;
		}
		else if (mema == "SR4") {
			trgt1 = 16;
			trgt2 = 1;
			tpbit = (sctrg & 16) / 16;
		}
		else if (mema == "SR5") {
			trgt1 = 32;
			trgt2 = 1;
			tpbit = (sctrg & 32) / 32;
		}
		else if (mema == "SR6") {
			trgt1 = 64;
			trgt2 = 1;
			tpbit = (sctrg & 64) / 64;
		}
		else if (mema == "SR7") {
			trgt1 = 128;
			trgt2 = 1;
			tpbit = (sctrg & 128) / 128;
		}
		else if (mema == "OR0") {
			trgt1 = 1;
			trgt2 = 2;
			tpbit = (outrg & 1);
		}
		else if (mema == "OR1") {
			trgt1 = 2;
			trgt2 = 2;
			tpbit = (outrg & 2) / 2;
		}
		else if (mema == "OR2") {
			trgt1 = 4;
			trgt2 = 2;
			tpbit = (outrg & 4) / 4;
		}
		else if (mema == "OR3") {
			trgt1 = 8;
			trgt2 = 2;
			tpbit = (outrg & 8) / 8;
		}
		else if (mema == "OR4") {
			trgt1 = 16;
			trgt2 = 2;
			tpbit = (outrg & 16) / 16;
		}
		else if (mema == "OR5") {
			trgt1 = 32;
			trgt2 = 2;
			tpbit = (outrg & 32) / 32;
		}
		else if (mema == "OR6") {
			trgt1 = 64;
			trgt2 = 2;
			tpbit = (outrg & 64) / 64;
		}
		else if (mema == "OR7") {
			trgt1 = 128;
			trgt2 = 2;
			tpbit = (outrg & 128) / 128;
		}
		else if (mema == "RR") { tpbit = rr.load(); }
		else if (mema == "IR1") { tpbit = (inprg & 2) / 2; }
		else if (mema == "IR2") { tpbit = (inprg & 4) / 4; }
		else if (mema == "IR3") { tpbit = (inprg & 8) / 8; }
		else if (mema == "IR4") { tpbit = (inprg & 16) / 16; }
		else if (mema == "IR5") { tpbit = (inprg & 32) / 32; }
		else if (mema == "IR6") { tpbit = (inprg & 64) / 64; }
		else if (mema == "IR7") { tpbit = (inprg & 128) / 128; }
		else {
			std::cerr << "Unknown memory address ('" << mema << "'), stop execution." << std::endl;
			stop = true;
			continue;
		}
		
		// Execute the operation.
		if (opco == "NOP0") {
			flag0 = false;
		}
		else if (opco == "LD  ") {
			if (ien) { rr = tpbit.load(); }
		}
		else if (opco == "ADD ") {
			if (ien) {
				tprr = rr + carry + tpbit;
				if 		(tprr == 0)	{ rr = 0; carry = 0; }
				else if (tprr == 1) { rr = 1; carry = 0; }
				else if (tprr == 2)	{ rr = 0; carry = 1; }
				else if (tprr == 3)	{ rr = 1; carry = 1; }
			}
		}
		else if (opco == "SUB ") {
			if (ien == 1) {
				if (tpbit == 1) { tpdb = 0; }
				else 			{ tpdb = 1; }
				
				tprr = (rr + carry + tpdb);
				
				if 		(tprr == 1) { rr = 0; carry = 0; }
				else if (tprr == 1) { rr = 1; carry = 0; }
				else if (tprr == 2) { rr = 0; carry = 1; }
				else if (tprr == 3) { rr = 1; carry = 1; }
			}
		}
		else if (opco == "ONE ") {
			rr = 1;
			carry = 0;
		}
		else if (opco == "NAND") {
			if (ien == 1) {
				tprr = rr & tpbit;
				if 		(tprr == 1) { rr = 0; }
				else if (rr == 0) { rr = 1; }
			}
		}
		else if (opco == "OR  ") { if (ien == 1) { rr = rr || tpbit; } }
		else if (opco == "XOR ") { if (ien == 1) { rr = rr || tpbit; } }
		else if (opco == "STO ") { if (oen == 1) { wrt = true; } }
		else if (opco == "STOC") { if (oen == 1) { wrt = true; } }
		else if (opco == "IEN ") { ien = tpbit.load(); }
		else if (opco == "OEN ") { oen = tpbit.load(); }
		else if (opco == "IOC ") { ioc = 1; do_beep(); }
		else if (opco == "RTN ") { rtn = 1; }
		else if (opco == "SKZ ") { skz = true; }
		else if (opco == "NOPF") { flagf = 1; }
		else if (opco == "HLT ") { flagf = 1; }
		else {
			std::cerr << "Unknown opcode ('" << opco << "'). Aborting." << std::endl;
			stop = 1;
			continue;
		}
		
		// Write the result to scratch or output (if STO/STOC).
		if (wrt == true) {
			tprr = rr.load();
			if (opco == "STOC") {
				if 		(rr == 1) { tprr = 0; }
				else if (rr == 0) { tprr = 1; }
			}
				
			if (trgt2 == 1) {
				if (tpbit == 1 && tprr == 0) { sctrg = sctrg - trgt1; }
				if (tpbit == 0 && tprr == 1) { sctrg = sctrg + trgt1; }
			}
			else if (trgt2 == 2) {
				if (tpbit == 1 && tprr == 0) { outrg = outrg - trgt1; }
				if (tpbit == 0 && tprr == 1) { outrg = outrg + trgt1; }
			}
		}
		
		update_display();
		
		end_cycle();
	}
	
	// Shutdown.
	std::cout << "Shutting down processor task..." << std::endl;	
	
	asmfile.close();
	return 0;
}
