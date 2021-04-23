#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "mu-mips.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-MIPS Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("show\t-- print the current content of the pipeline registers\n");
	printf("f x\t -- Turn forwarding flag ON: x = 1, Turn forwarding flag OFF: x = 0");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_pipeline();
	CURRENT_STATE = NEXT_STATE;
	CYCLE_COUNT++;
}

/***************************************************************/
/* Simulate MIPS for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("# Cycles Executed\t: %u\n", CYCLE_COUNT - 1);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < MIPS_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-MIPS SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			if (buffer[1] == 'h' || buffer[1] == 'H'){
				show_pipeline();
			}else {
				runAll(); 
			}
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-MIPS! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		case 'F':
		case 'f':
			if (scanf("%d", &ENABLE_FORWARDING) != 1) 
			{	
				break;
			}
			ENABLE_FORWARDING == 0 ? printf("Forwarding OFF\n") : printf("Forwarding ON\n");
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < MIPS_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;

	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

/************************************************************/
/* maintain the pipeline                                                                                           */ 
/************************************************************/
void handle_pipeline()
{
	/*INSTRUCTION_COUNT should be incremented when instruction is done*/
	/*Since we do not have branch/jump instructions, INSTRUCTION_COUNT should be incremented in WB stage */
	
	WB();
	MEM();
	EX();
	ID();
	IF();
}

/************************************************************/
/* writeback (WB) pipeline stage:                                                                          */ 
/************************************************************/
void WB()
{
	if(CYCLE_COUNT < 5)
	{
		return;
	}
	
	if(WB_MEM.IR == 0 && WB_MEM.PC == 0 && WB_MEM.SYSCALL == 0)
	{
		printf("STALL\n");
		return;
	}

	print_instruction(WB_MEM.PC);
	uint32_t opcode = (WB_MEM.IR & 0xFC000000) >> 26;
	uint32_t function = (WB_MEM.IR & 0x3F);
	uint32_t rd = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t rt = (0x1F0000 & WB_MEM.IR) >> 16;
	
	INSTRUCTION_COUNT++;
	
	if (opcode == 0x00) {
		switch(function) {
			case 0x00://SLL
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x02://SRL
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x03://SRA
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x0C://SYSCALL
				if(WB_MEM.SYSCALL == 0xA)
				{
					RUN_FLAG = FALSE;
				} 
				break;
			case 0x10://MFHI
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x11://MTHI
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				break;
			case 0x12://MFLO
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x13://MTLO
				NEXT_STATE.LO = WB_MEM.ALUOutput;
				break;
			case 0x18://MULT
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x19://MULT Unsigned
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x1A://DIV
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x1B://DIVU
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x20://ADD
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x21://ADD Unsigned
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x22://SUB
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x23://SUB Unsigned
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x24://AND
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x25://OR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x26://XOR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x27://NOR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x2A://SLT
				if(EX_ID.A < EX_ID.B)
				{
					NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				}
				else
				{
					NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				}
				break;
		}
	}
	else {
		switch(opcode) {
			case 0x8://ADDI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0x9://ADDIU
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xC://ANDI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xE://XORI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xD://ORI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xA://SLTI
				if(EX_ID.A < EX_ID.imm)
				{
					NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				}
				else
				{
					NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				}
				break;
			case 0x20://LB
				NEXT_STATE.REGS[rt] = WB_MEM.LMD;
				break;
			case 0x21://LH
				NEXT_STATE.REGS[rt] = WB_MEM.LMD;
				break;
			case 0xF://LUI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0x23://LW
				NEXT_STATE.REGS[rt] = WB_MEM.LMD;;
				break;
		}
	}
}	

/************************************************************/
/* memory access (MEM) pipeline stage:                                                          */ 
/************************************************************/
void MEM()
{
	if(CYCLE_COUNT < 4 || WB_MEM.SYSCALL == 0xA)
	{
		return;
	}
	
	WB_MEM.IR = MEM_EX.IR;
	WB_MEM.PC = MEM_EX.PC;
	WB_MEM.SYSCALL = MEM_EX.SYSCALL;

	uint32_t opcode = (MEM_EX.IR & 0xFC000000) >> 26;
	uint32_t WB_RD = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t MEM_RD = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t EX_RS = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_RT = (0x1F0000 & EX_ID.IR) >> 16;

	if(ENABLE_FORWARDING)
	{
		if(opcode == 0x29 || opcode == 0x2B || opcode == 0x28)
		{
			
		}
		else
		{
			if((WB_RD != 0) && !((MEM_RD != 0) && (MEM_RD == EX_RS)) && (WB_RD == EX_RS))
			{
				ForwardA = 01;
			}
			
			if((WB_RD != 0) && !((MEM_RD != 0) && (MEM_RD == EX_RT)) && (WB_RD == EX_RT))
			{
				ForwardB = 01;
			}
		}
	}
	if (opcode == 0x00) {
		WB_MEM.ALUOutput = MEM_EX.ALUOutput;
		WB_MEM.ALUOutput2 = MEM_EX.ALUOutput2;
	}
	else {
		switch(opcode) {
			case 0x8:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0x9:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xC:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xE:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xD:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xA:
				if(EX_ID.A < EX_ID.imm)
				{
					WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				}
				else
				{
					WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				}
				break;
			case 0x20:
				WB_MEM.LMD = mem_read_32(MEM_EX.ALUOutput) >> 24;
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0x21:
				WB_MEM.LMD = mem_read_32(MEM_EX.ALUOutput) >> 16;
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0xF:
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0x23:
				WB_MEM.LMD = mem_read_32(MEM_EX.ALUOutput);
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0x29:
				mem_write_32(MEM_EX.ALUOutput, ((MEM_EX.B & 0xFFFF) << 16) + (0xFFFF & mem_read_32(MEM_EX.ALUOutput)));
				break;
			case 0x28:
				mem_write_32(MEM_EX.ALUOutput, ((MEM_EX.B & 0xFF) << 24) + (0xFFFFFF & mem_read_32(MEM_EX.ALUOutput)));
				break;
			case 0x2B:
				mem_write_32(MEM_EX.ALUOutput, MEM_EX.B);
				break;
		}
	}
}

/************************************************************/
/* execution (EX) pipeline stage:                                                                          */ 
/************************************************************/
void EX()
{
	
	if(CYCLE_COUNT < 3 || MEM_EX.SYSCALL == 0xA)
	{
		return;
	}
	MEM_EX.IR = EX_ID.IR;
	MEM_EX.PC = EX_ID.PC;
	MEM_EX.SYSCALL = EX_ID.SYSCALL;

	if(EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYSCALL == 0)
	{
		//printf("Skipping EX()\n");
		return;
	}

	
	uint32_t opcode = (EX_ID.IR & 0xFC000000) >> 26;
	uint32_t function = (EX_ID.IR & 0x3F);
	uint32_t shamt = (0x7C0 & EX_ID.IR) >> 6;
	uint32_t MEM_RD = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t EX_RS = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_RT = (0x1F0000 & EX_ID.IR) >> 16;

	uint64_t product;

	if(ENABLE_FORWARDING)
	{
		
		if (ForwardA == 10)
		{
			
			EX_ID.A = MEM_EX.ALUOutput;
			ForwardA = 0;
		}
		else if (ForwardA == 01)
		{
			EX_ID.A = WB_MEM.ALUOutput;
			ForwardA = 0;
		}
		if (ForwardB == 10)
		{
			EX_ID.B = MEM_EX.ALUOutput;
			ForwardB = 0;
		}
		else if (ForwardB == 01)
		{
			EX_ID.B = WB_MEM.ALUOutput;
			ForwardB = 0;
		}
	}

	if(ENABLE_FORWARDING)
	{
		EX_ID.IR = ID_IF.IR;
		EX_ID.PC = ID_IF.PC;
		EX_ID.SYSCALL = ID_IF.SYSCALL;
		MEM_RD = (0xF800 & MEM_EX.IR) >> 11;
		EX_RS = (0x3E00000 & EX_ID.IR) >> 21;
		EX_RT = (0x1F0000 & EX_ID.IR) >> 16;
		if(opcode == 0x29 || opcode == 0x2B || opcode == 0x28)
		{
			
		}
		else
		{
			if(opcode != 0x00)
			{
				MEM_RD = (0x1F0000 & MEM_EX.IR) >> 16;

			}

			if((MEM_RD != 0) && (MEM_RD == EX_RS))
			{
				ForwardA = 10;
			}

			if((MEM_RD != 0) && (MEM_RD == EX_RT))
			{
				ForwardB = 10;
			}
		}
	}

	if (opcode == 0x00) {
		switch(function) {
			case 0x00:
				MEM_EX.ALUOutput = EX_ID.B << shamt;
				break;
			case 0x02:
				MEM_EX.ALUOutput = EX_ID.B >> shamt;
				break;
			case 0x03:
				MEM_EX.ALUOutput = EX_ID.B >> shamt;
				break;
			case 0x0C:
				if(EX_ID.SYSCALL == 0xA)
				{
					MEM_EX.ALUOutput = 0xA;
				} 
				break;
			case 0x10:
				MEM_EX.ALUOutput = EX_ID.HI;
				break;
			case 0x11:
				MEM_EX.ALUOutput = EX_ID.A;
				break;
			case 0x12:
				MEM_EX.ALUOutput = EX_ID.LO;
				break;
			case 0x13:
				MEM_EX.ALUOutput = EX_ID.A;
				break;
			case 0x18:
				product = EX_ID.A * EX_ID.B;
				MEM_EX.ALUOutput = product >> 32;
				MEM_EX.ALUOutput2 = product & 0xFFFFFFFF;
				break;
			case 0x19:
				product = EX_ID.A * EX_ID.B;
				MEM_EX.ALUOutput = product >> 32;
				MEM_EX.ALUOutput2 = product & 0xFFFFFFFF;
				break;
			case 0x1A:
				MEM_EX.ALUOutput = EX_ID.A / EX_ID.B;
				MEM_EX.ALUOutput2 = EX_ID.A % EX_ID.B;
				break;
			case 0x1B:
				MEM_EX.ALUOutput = EX_ID.A / EX_ID.B;
				MEM_EX.ALUOutput2 = EX_ID.A % EX_ID.B;
				break;
			case 0x20:
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.B;
				break;
			case 0x21:
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.B;
				break;
			case 0x22:
				MEM_EX.ALUOutput = EX_ID.A - EX_ID.B;
				break;
			case 0x23:
				MEM_EX.ALUOutput = EX_ID.A - EX_ID.B;
				break;
			case 0x24:
				MEM_EX.ALUOutput = EX_ID.A & EX_ID.B;
				break;
			case 0x25:
				MEM_EX.ALUOutput = EX_ID.A | EX_ID.B;
				break;
			case 0x26:
				MEM_EX.ALUOutput = EX_ID.A ^ EX_ID.B;
				break;
			case 0x27:
				MEM_EX.ALUOutput = ~(EX_ID.A | EX_ID.B);
				break;
			case 0x2A:
				if(EX_ID.A < EX_ID.B)
				{
					MEM_EX.ALUOutput = 0x00000001;
				}
				else
				{
					MEM_EX.ALUOutput = 0x00000000;
				}
				break;
		}
	} 
	else {
		switch(opcode) {
			case 0x8:
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.imm;
				break;
			case 0x9:
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.imm;
				break;
			case 0xC:
				MEM_EX.ALUOutput = EX_ID.imm & EX_ID.A & 0xFFFF;
				break;
			case 0xE:
				MEM_EX.ALUOutput = EX_ID.A ^ EX_ID.imm;
				break;
			case 0xD:
				MEM_EX.ALUOutput = EX_ID.A | EX_ID.imm;
				break;
			case 0xA:
				if(EX_ID.A < EX_ID.imm)
				{
					MEM_EX.ALUOutput = 0x00000001;
				}
				else
				{
					MEM_EX.ALUOutput = 0x00000000;
				}
				break;
			case 0x20:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x21:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0xF:
				MEM_EX.ALUOutput = (EX_ID.imm << 16);
				break;
			case 0x23:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x29:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x28:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x2B:
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
		}
	}
}

/************************************************************/
/* instruction decode (ID) pipeline stage:                                                         */ 
/************************************************************/
void ID()
{
	if(CYCLE_COUNT < 2 || EX_ID.SYSCALL == 0xA)
	{
		return;
	}

	int stallFlag = 0;
	if(EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYSCALL == 0)
	{
		stallFlag = 1;
	}

	EX_ID.IR = ID_IF.IR;
	EX_ID.PC = ID_IF.PC;
	EX_ID.SYSCALL = ID_IF.SYSCALL;
	uint32_t rs = (0x3E00000 & ID_IF.IR) >> 21;
	uint32_t rt = (0x1F0000 & ID_IF.IR) >> 16;
	uint32_t immediate = (0xFFFF & ID_IF.IR);
	EX_ID.A = CURRENT_STATE.REGS[rs];
	EX_ID.B = CURRENT_STATE.REGS[rt];
	EX_ID.HI = CURRENT_STATE.HI;
	EX_ID.LO = CURRENT_STATE.LO;
	EX_ID.imm = (uint32_t)((int16_t)immediate);
	uint32_t opcode = (EX_ID.IR & 0xFC000000) >> 26;


	if (stallFlag == 1 || (ENABLE_FORWARDING && (opcode == 0x28 || opcode == 0x29 || opcode == 0x2B)))
	{
		EX_ID.A = NEXT_STATE.REGS[rs];
		EX_ID.B = NEXT_STATE.REGS[rt];
	}


	uint32_t MEM_opcode = (MEM_EX.IR & 0xFC000000) >> 26;
	uint32_t WB_opcode = (WB_MEM.IR & 0xFC000000) >> 26;
	uint32_t MEM_RD = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t WB_RD = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t EX_RS = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_RT = (0x1F0000 & EX_ID.IR) >> 16;
	uint32_t function = (EX_ID.IR & 0x3F);
	
	

	if((!ENABLE_FORWARDING) && MEM_opcode == 0x00)
	{
		if(!(MEM_opcode == 0x28 || MEM_opcode == 0x29 || MEM_opcode == 0x2B))
		{
			if((MEM_RD != 0) && (MEM_RD == EX_RS))
			{
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYSCALL = 0;
			}
			if((MEM_RD != 0) && (MEM_RD == EX_RT))
			{
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYSCALL = 0;
			}
		}
	}
	else if((!ENABLE_FORWARDING) || (MEM_opcode == 0x20 || MEM_opcode == 0x21 || MEM_opcode == 0x23))
	{
		MEM_RD = (0x1F0000 & MEM_EX.IR) >> 16;
		switch(MEM_opcode) {
			case 0x8:
			case 0x9:
			case 0xC:
			case 0xE:
			case 0xD:	
			case 0xA:
			case 0x20:
			case 0x21:
			case 0xF:
			case 0x23:
				if(!(MEM_opcode == 0x28 || MEM_opcode == 0x29 || MEM_opcode == 0x2B))
				{
					if((MEM_RD != 0) && (MEM_RD == EX_RS))
					{
						EX_ID.IR = 0;
						EX_ID.PC = 0;
						EX_ID.SYSCALL = 0;
					}
				}
				break;
			case 0x29:
			case 0x28:
			case 0x2B:
				if((MEM_RD != 0) && (MEM_RD == EX_RS))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}
				if((MEM_RD != 0) && (MEM_RD == EX_RT))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}
				break;
		}
	}

	if(((!ENABLE_FORWARDING) || (WB_opcode == 0xF)) && WB_opcode == 0x00) 
	{
		if(!(WB_opcode == 0x28 || WB_opcode == 0x29 || WB_opcode == 0x2B))
		{
			if((WB_RD != 0) && (WB_RD == EX_RS))
			{
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYSCALL = 0;
			}

			if((WB_RD != 0) && (WB_RD == EX_RT))
			{
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYSCALL = 0;
			}
		}
	}
	else if((!ENABLE_FORWARDING) || (WB_opcode == 0xF))
	{
		WB_RD = (0x1F0000 & WB_MEM.IR) >> 16;
		switch(WB_opcode) {
			case 0x8:
			case 0x9:
			case 0xC:
			case 0xE:
			case 0xD:
			case 0xA:
			case 0x20:
			case 0x21:
			case 0xF:
			case 0x23:
				if(!(WB_opcode == 0x28 || WB_opcode == 0x29 || WB_opcode == 0x2B))
				{
					if((WB_RD != 0) && (WB_RD == EX_RS))
					{
						EX_ID.IR = 0;
						EX_ID.PC = 0;
						EX_ID.SYSCALL = 0;
					}
				}
				break;
			case 0x29:
			case 0x28:
			case 0x2B:
				if((WB_RD == EX_RS)&&(WB_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}

				if((WB_RD == EX_RT)&&(WB_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}
				break;
		}
	}

	if(!ENABLE_FORWARDING)
	{
		MEM_RD = (0x1F0000 & MEM_EX.IR) >> 16;
		WB_RD = (0x1F0000 & WB_MEM.IR) >> 16;
		switch (opcode)
		{
			case 0x29:
			case 0x28:
			case 0x2B:
				if((MEM_RD == EX_RS)&&(MEM_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}
				if((MEM_RD == EX_RT)&&(MEM_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}	

				if((WB_RD == EX_RS)&&(WB_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}

				if((WB_RD == EX_RT)&&(WB_RD != 0))
				{
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYSCALL = 0;
				}		
				break;
		}
	}

	if (function == 0x0C && opcode == 0x00)
	{
		EX_ID.SYSCALL = CURRENT_STATE.REGS[2];
	}
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	if(CYCLE_COUNT < 1 || ID_IF.SYSCALL == 0xA || ((EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYSCALL == 0) && CYCLE_COUNT > 4))
	{
		return;
	}
	
	uint32_t opcode = (ID_IF.IR & 0xFC000000) >> 26;
	uint32_t function = (ID_IF.IR & 0x3F);
	
	ID_IF.IR = mem_read_32(CURRENT_STATE.PC);
	ID_IF.PC = CURRENT_STATE.PC;
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	
	
	if (opcode == 0x00 && function == 0x0C)
		ID_IF.SYSCALL = 0xA;
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in MIPS assembly format)    */ 
/************************************************************/
void print_program(){
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

void print_instruction(uint32_t addr){
	uint32_t instruction = mem_read_32(addr);
	uint32_t opcode = (instruction & 0xFC000000) >> 26;
	uint32_t rs = (0x3E00000 & instruction) >> 21;
	uint32_t rt = (0x1F0000 & instruction) >> 16;
	uint32_t immediate = (0xFFFF & instruction);
	uint32_t rd = (0xF800 & instruction) >> 11;
	uint32_t shamt = (0x7C & instruction) >> 6;
	uint32_t function = (0x3F & instruction);
	uint32_t offset = (0x3FFFFFF & instruction);

	if (opcode == 0x00) {
		switch(function) {
			case 0x00:
				printf("SLL $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x02:
				printf("SRL $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x03:
				printf("SRA $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x08:
				printf("JR $%d\n", rs);
				break;
			case 0x09:
				printf("JALR $%d, $%d\n", rs, rd);
				break;
			case 0x0C:
				printf("SYSCALL\n");
				break;
			case 0x10:
				printf("MFHI $%d\n", rd);
				break;
			case 0x11:
				printf("MTHI $%d\n", rs);
				break;
			case 0x12:
				printf("MFLO $%d\n", rd);
				break;
			case 0x13:
				printf("MTLO $%d\n", rs);
				break;
			case 0x18:
				printf("MULT $%d, $%d\n", rs, rt);
				break;
			case 0x19:
				printf("MULTU $%d, $%d\n", rs, rt);
				break;
			case 0x1A:
				printf("DIV $%d, $%d\n", rs, rt);
				break;
			case 0x1B:
				printf("DIVU $%d, $%d\n", rs, rt);
				break;
			case 0x20:
				printf("ADD $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x21:
				printf("ADDU $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x22:
				printf("SUB $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x23:
				printf("SUBU $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x24:
				printf("AND $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x25:
				printf("OR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x26:
				printf("XOR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x27:
				printf("NOR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x2A:
				printf("SLT $%d, $%d, $%d\n", rd, rs, rt);
				break;
		}
	} 
	else {
		switch(opcode) {
			case 0x8:
				printf("ADDI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0x9:
				printf("ADDIU $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0xC:
				printf("ANDI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0xE:
				printf("XORI $%d, $%d, 0x%x\n", rt, rs, immediate);	
				break;
			case 0xD:
				printf("ORI $%d, $%d, 0x%x\n", rt, rs, immediate);	
				break;
			case 0xA:
				printf("SLTI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0x4:
				printf("BEQ $%d, $%d, 0x%x\n", rs, rt, (uint32_t)(immediate * 4));
				break;
			case 0x1:
				if (rt == 1) 
				{
					printf("BGEZ $%d, 0x%x\n", rs, (uint32_t)(immediate * 4));
				} 
				else if (rt == 0) 
				{
					printf("BLTZ $%d, 0x%x\n", rs, (uint32_t)(immediate * 4));
				}
				break;
			case 0x7:
				printf("BGTZ $%d, 0x%x\n", rs, (uint32_t)(immediate * 4));
				break;
			case 0x6:
				printf("BLEZ $%d, 0x%x\n", rs, (uint32_t)(immediate * 4));
				break;
			case 0x5:
				printf("BNE $%d, $%d, %d\n", rs, rt, (uint32_t)(immediate * 4));
				break;
			case 0x2:	
				printf("J 0x%x\n", offset);
				break;
			case 0x3:
				printf("JAL 0x%x\n", offset);
				break;
			case 0x20:
				printf("LB $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x21:
				printf("LH $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0xF:
				printf("LUI $%d, 0x%x\n", rt, immediate);
				break;
			case 0x23:
				printf("LW $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x29:
				printf("SH $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x28:
				printf("SB $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x2B:
				printf("SW $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
		}
	}
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	printf("Current PC:\t\t%X\n", CURRENT_STATE.PC);
	printf("IF/ID.IR\t\t%X  ", ID_IF.IR);
	print_instruction(ID_IF.PC);
	printf("IF/ID.PC\t\t%X\n", ID_IF.PC);
	printf("\n");
	printf("ID/EX.IR\t\t%X  ", EX_ID.IR);
	print_instruction(EX_ID.PC);
	printf("ID/EX.A\t\t%X\n", EX_ID.A);
	printf("ID/EX.B\t\t%X\n", EX_ID.B);
	printf("ID/EX.imm\t\t%X\n", EX_ID.imm);
	printf("\n");
	printf("EX/MEM.IR\t\t%X ", MEM_EX.IR);
	print_instruction(MEM_EX.PC);
	printf("EX/MEM.A\t\t%X\n", MEM_EX.A);
	printf("EX/MEM.B\t\t%X\n", MEM_EX.B);
	printf("EX/MEM.ALUOutput\t\t%X\n", MEM_EX.ALUOutput);
	printf("EX/MEM.ALUOutput2\t\t%X\n", MEM_EX.ALUOutput2);
	printf("\n");
	printf("MEM/WEB.IR\t\t%X", WB_MEM.IR);
	print_instruction(WB_MEM.PC);
	printf("MEM/WEB.A\t\t%X\n", WB_MEM.IR);
	printf("MEM/WEB.LMD\t\t%X\n", WB_MEM.IR);
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-MIPS SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
