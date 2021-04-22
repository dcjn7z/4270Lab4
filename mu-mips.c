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
	
	if(WB_MEM.IR == 0 && WB_MEM.PC == 0 && WB_MEM.SYS == 0)
	{
		printf("BUBBLE\n");
		return;
	}

	print_instruction(WB_MEM.PC);
	uint32_t op_code = (WB_MEM.IR & 0xFC000000) >> 26;
	uint32_t function = (WB_MEM.IR & 0x3F);
	uint32_t rd = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t rt = (0x1F0000 & WB_MEM.IR) >> 16;
	
	INSTRUCTION_COUNT++;
/*	
	printf("\n=================WB==============\n");
	print_instruction(WB_MEM.PC);
	printf("WB.SYS = %X\n", WB_MEM.SYS);
	printf("WB op_code = %X\n", op_code);
	printf("====================================");
*/	
	
	if (op_code == 0x00) {
		switch(function) {
			case 0x00:		//SLL
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x02:		//SRL
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x03:		//SRA
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x0C:		//SYSCALL
				if(WB_MEM.SYS == 0xA)
				{
					RUN_FLAG = FALSE;
				} 
				break;
			case 0x10:		//MFHI
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x11:		//MTHI
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				break;
			case 0x12:		//MFLO
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x13:		//MTLO
				NEXT_STATE.LO = WB_MEM.ALUOutput;
				break;
			case 0x18:		//MULT
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x19:		//MULT Unsigned
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x1A:		//DIV
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x1B:		//DIVU
				NEXT_STATE.HI = WB_MEM.ALUOutput;
				NEXT_STATE.LO = WB_MEM.ALUOutput2;
				break;
			case 0x20:		//ADD
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x21:		//ADD Unsigned
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x22:		//SUB
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x23:		//SUB Unsigned
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x24:		//AND
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x25:		//OR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x26:		//XOR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x27:		//NOR
				NEXT_STATE.REGS[rd] = WB_MEM.ALUOutput;
				break;
			case 0x2A:		//SLT
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
	// Regular opcode (I-Type)
	else {
		switch(op_code) {
			case 0x8:		//ADDI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0x9:		//ADDIU
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xC:		//ANDI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xE:		//XORI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xD:		//ORI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0xA:		//SLTI
				if(EX_ID.A < EX_ID.imm)
				{
					NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				}
				else
				{
					NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				}
				break;
			case 0x20:		//LB
				// :(
				NEXT_STATE.REGS[rt] = WB_MEM.LMD;
				break;
			case 0x21:		//LH
				NEXT_STATE.REGS[rt] = WB_MEM.LMD;
				break;
			case 0xF:		//LUI
				NEXT_STATE.REGS[rt] = WB_MEM.ALUOutput;
				break;
			case 0x23:		//LW
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
	if(CYCLE_COUNT < 4 || WB_MEM.SYS == 0xA)
	{
		return;
	}

	//print_instruction(WB_MEM.PC);

	WB_MEM.IR = MEM_EX.IR;
	WB_MEM.PC = MEM_EX.PC;
	WB_MEM.SYS = MEM_EX.SYS;


	uint32_t op_code = (MEM_EX.IR & 0xFC000000) >> 26;
	uint32_t WB_rd = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t MEM_rd = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t EX_rs = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_rt = (0x1F0000 & EX_ID.IR) >> 16;

	if(ENABLE_FORWARDING)
	{
		if(op_code == 0x29 || op_code == 0x2B || op_code == 0x28)
		{
			
		}
		else
		{
			if((WB_rd != 0) && !((MEM_rd != 0) && (MEM_rd == EX_rs)) && (WB_rd == EX_rs))
			{
				//print_instruction(WB_MEM.PC);
				ForwardA = 01;
			}
			
			if((WB_rd != 0) && !((MEM_rd != 0) && (MEM_rd == EX_rt)) && (WB_rd == EX_rt))
			{
				//print_instruction(WB_MEM.PC);
				ForwardB = 01;
			}
		}
	}
	
/*	
	printf("\n===================MEM=================\n");
	print_instruction(WB_MEM.PC);
	printf("MEM.SYS = %X\n", WB_MEM.SYS);
	printf("MEM op_code = %X\n", op_code);
	printf("====================================\n");
*/
	if (op_code == 0x00) {
		WB_MEM.ALUOutput = MEM_EX.ALUOutput;
		WB_MEM.ALUOutput2 = MEM_EX.ALUOutput2;
	}
	// Regular opcode (I-Type)
	else {
		switch(op_code) {
			case 0x8:		//ADDI
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0x9:		//ADDIU
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xC:		//ANDI
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xE:		//XORI
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xD:		//ORI
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0xA:		//SLTI
				if(EX_ID.A < EX_ID.imm)
				{
					WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				}
				else
				{
					WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				}
				break;
			case 0x20:		//LB
				// :(
				WB_MEM.LMD = (int32_t)((int16_t)(mem_read_32(MEM_EX.ALUOutput) >> 24));
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0x21:		//LH
				WB_MEM.LMD = (int32_t)((int16_t)(mem_read_32(MEM_EX.ALUOutput) >> 16));
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0xF:		//LUI
				WB_MEM.ALUOutput = MEM_EX.ALUOutput;
				break;
			case 0x23:		//LW
				WB_MEM.LMD = mem_read_32(MEM_EX.ALUOutput);
				MEM_EX.ALUOutput = WB_MEM.LMD;
				break;
			case 0x29:		//SH
				mem_write_32(MEM_EX.ALUOutput, (((int32_t)((int16_t)(MEM_EX.B & 0xFFFF))) << 16) + (0xFFFF & mem_read_32(MEM_EX.ALUOutput)));
				break;
			case 0x28:		//SB
				mem_write_32(MEM_EX.ALUOutput, (((int32_t)((int8_t)(MEM_EX.B & 0xFF))) << 24) + (0xFFFFFF & mem_read_32(MEM_EX.ALUOutput)));
				break;
			case 0x2B:		//SW
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
	if(CYCLE_COUNT < 3 || MEM_EX.SYS == 0xA)
	{
		return;
	}

	//print_instruction(MEM_EX.PC);

	MEM_EX.IR = EX_ID.IR;
	MEM_EX.PC = EX_ID.PC;
	MEM_EX.SYS = EX_ID.SYS;

	if(EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYS == 0)
	{
		//printf("EX STALL\n");
		return;
	}

	
	uint32_t op_code = (EX_ID.IR & 0xFC000000) >> 26;
	uint32_t function = (EX_ID.IR & 0x3F);
	uint32_t shamt = (0x7C0 & EX_ID.IR) >> 6;
	uint32_t sixteen_bit_mask = 0xFFFF;
	uint32_t MEM_rd = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t EX_rs = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_rt = (0x1F0000 & EX_ID.IR) >> 16;

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
		EX_ID.SYS = ID_IF.SYS;
		MEM_rd = (0xF800 & MEM_EX.IR) >> 11;
		EX_rs = (0x3E00000 & EX_ID.IR) >> 21;
		EX_rt = (0x1F0000 & EX_ID.IR) >> 16;
		if(op_code == 0x29 || op_code == 0x2B || op_code == 0x28)
		{
			
		}
		else
		{

			if(op_code != 0x00) 									//check for i-type
			{
				// Set rd to be equivalent to the rt logic
				MEM_rd = (0x1F0000 & MEM_EX.IR) >> 16;

			}

			if((MEM_rd != 0) && (MEM_rd == EX_rs))
			{
				//print_instruction(MEM_EX.PC);
				ForwardA = 10;
				/*if(op_code == 0x20 || op_code == 0x21 || op_code == 0x23)
				{
					ForwardA = 11;
				}*/
			}

			if((MEM_rd != 0) && (MEM_rd == EX_rt))
			{
				//print_instruction(MEM_EX.PC);
				ForwardB = 10;
			}
		}
	}


/*
	printf("\n================EX===================\n");
	print_instruction(MEM_EX.PC);
	printf("EX_ID.SYS = %X\n", EX_ID.SYS);
	printf("EX op_code = %X\n", op_code);
	printf("====================================\n");
*/
	if (op_code == 0x00) {
		switch(function) {
			case 0x00:		//SLL
				MEM_EX.ALUOutput = EX_ID.B << shamt;
				break;
			case 0x02:		//SRL
				MEM_EX.ALUOutput = EX_ID.B >> shamt;
				break;
			case 0x03:		//SRA
				MEM_EX.ALUOutput = EX_ID.B >> shamt;
				break;
			case 0x0C:		//SYSCALL
				if(EX_ID.SYS == 0xA)
				{
					MEM_EX.ALUOutput = 0xA;
				} 
				break;
			case 0x10:		//MFHI
				MEM_EX.ALUOutput = EX_ID.HI;
				break;
			case 0x11:		//MTHI
				MEM_EX.ALUOutput = EX_ID.A;
				break;
			case 0x12:		//MFLO
				MEM_EX.ALUOutput = EX_ID.LO;
				break;
			case 0x13:		//MTLO
				MEM_EX.ALUOutput = EX_ID.A;
				break;
			case 0x18:		//MULT
				product = EX_ID.A * EX_ID.B;
				MEM_EX.ALUOutput = product >> 32;
				MEM_EX.ALUOutput2 = product & 0xFFFFFFFF;
				break;
			case 0x19:		//MULT Unsigned
				product = EX_ID.A * EX_ID.B;
				MEM_EX.ALUOutput = product >> 32;
				MEM_EX.ALUOutput2 = product & 0xFFFFFFFF;
				break;
			case 0x1A:		//DIV
				MEM_EX.ALUOutput = EX_ID.A / EX_ID.B;
				MEM_EX.ALUOutput2 = EX_ID.A % EX_ID.B;
				break;
			case 0x1B:		//DIVU
				MEM_EX.ALUOutput = EX_ID.A / EX_ID.B;
				MEM_EX.ALUOutput2 = EX_ID.A % EX_ID.B;
				break;
			case 0x20:		//ADD
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.B;
				break;
			case 0x21:		//ADD Unsigned
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.B;
				break;
			case 0x22:		//SUB
				MEM_EX.ALUOutput = EX_ID.A - EX_ID.B;
				break;
			case 0x23:		//SUB Unsigned
				MEM_EX.ALUOutput = EX_ID.A - EX_ID.B;
				break;
			case 0x24:		//AND
				MEM_EX.ALUOutput = EX_ID.A & EX_ID.B;
				break;
			case 0x25:		//OR
				MEM_EX.ALUOutput = EX_ID.A | EX_ID.B;
				break;
			case 0x26:		//XOR
				MEM_EX.ALUOutput = EX_ID.A ^ EX_ID.B;
				break;
			case 0x27:		//NOR
				MEM_EX.ALUOutput = ~(EX_ID.A | EX_ID.B);
				break;
			case 0x2A:		//SLT
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
	// Regular opcode (I-Type)
	else {
		switch(op_code) {
			case 0x8:		//ADDI
				// make immediate sign extended
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.imm;
				break;
			case 0x9:		//ADDIU
				// should be unsigned?
				MEM_EX.ALUOutput = EX_ID.A + EX_ID.imm;
				break;
			case 0xC:		//ANDI
				//NEXT_STATE.REGS[rt] = (0000000000000000 | (immediate & (EX_ID.A & sixteen_bit_mask)));
				MEM_EX.ALUOutput = EX_ID.imm & EX_ID.A & sixteen_bit_mask;
				break;
			case 0xE:		//XORI
				MEM_EX.ALUOutput = EX_ID.A ^ EX_ID.imm;
				break;
			case 0xD:		//ORI
				MEM_EX.ALUOutput = EX_ID.A | EX_ID.imm;
				break;
			case 0xA:		//SLTI
				if(EX_ID.A < EX_ID.imm)
				{
					MEM_EX.ALUOutput = 0x00000001;
				}
				else
				{
					MEM_EX.ALUOutput = 0x00000000;
				}
				break;
			case 0x20:		//LB
				// :(
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x21:		//LH
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0xF:		//LUI
				MEM_EX.ALUOutput = (EX_ID.imm << 16); // ?????????????
				break;
			case 0x23:		//LW
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x29:		//SH
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x28:		//SB
				MEM_EX.ALUOutput = EX_ID.imm + EX_ID.A;
				MEM_EX.B = EX_ID.B;
				break;
			case 0x2B:		//SW
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
	if(CYCLE_COUNT < 2 || EX_ID.SYS == 0xA)
	{
		return;
	}

	int stallFlag = 0;
	if(EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYS == 0)
	{
		stallFlag = 1;
	}

	EX_ID.IR = ID_IF.IR;
	EX_ID.PC = ID_IF.PC;
	EX_ID.SYS = ID_IF.SYS;
	uint32_t rs = (0x3E00000 & ID_IF.IR) >> 21;
	uint32_t rt = (0x1F0000 & ID_IF.IR) >> 16;
	uint32_t immediate = (0xFFFF & ID_IF.IR);
	EX_ID.A = CURRENT_STATE.REGS[rs];
	EX_ID.B = CURRENT_STATE.REGS[rt];
	EX_ID.HI = CURRENT_STATE.HI;
	EX_ID.LO = CURRENT_STATE.LO;
	EX_ID.imm = (uint32_t)((int16_t)immediate);
	uint32_t op_code = (EX_ID.IR & 0xFC000000) >> 26;

	if (stallFlag == 1 || (ENABLE_FORWARDING && (op_code == 0x29 || op_code == 0x2B || op_code == 0x28)))
	{
		EX_ID.A = NEXT_STATE.REGS[rs];
		EX_ID.B = NEXT_STATE.REGS[rt];
	}

	uint32_t MEM_rd = (0xF800 & MEM_EX.IR) >> 11;
	uint32_t WB_rd = (0xF800 & WB_MEM.IR) >> 11;
	uint32_t EX_rs = (0x3E00000 & EX_ID.IR) >> 21;
	uint32_t EX_rt = (0x1F0000 & EX_ID.IR) >> 16;
	uint32_t function = (EX_ID.IR & 0x3F);
	uint32_t MEM_op_code = (MEM_EX.IR & 0xFC000000) >> 26;
	uint32_t WB_op_code = (WB_MEM.IR & 0xFC000000) >> 26;
	

	if((!ENABLE_FORWARDING) && MEM_op_code == 0x00)
	{
		if(!(MEM_op_code == 0x29 || MEM_op_code == 0x2B || MEM_op_code == 0x28))
		{
			if((MEM_rd != 0) && (MEM_rd == EX_rs))
			{
				//print_instruction(EX_ID.PC);
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYS = 0;
			}
			if((MEM_rd != 0) && (MEM_rd == EX_rt))
			{
				//print_instruction(EX_ID.PC);
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYS = 0;
			}
		}
	}
	else if((!ENABLE_FORWARDING) || (MEM_op_code == 0x20 || MEM_op_code == 0x21 || MEM_op_code == 0x23))
	{
		MEM_rd = (0x1F0000 & MEM_EX.IR) >> 16;
		switch(MEM_op_code) {
			case 0x8:		//ADDI
			case 0x9:		//ADDIU
			case 0xC:		//ANDI
			case 0xE:		//XORI
			case 0xD:		//ORI
			case 0xA:		//SLTI
			case 0x20:		//LB
			case 0x21:		//LH
			case 0xF:		//LUI
			case 0x23:		//LW
				if(!(MEM_op_code == 0x29 || MEM_op_code == 0x2B || MEM_op_code == 0x28))
				{
					if((MEM_rd != 0) && (MEM_rd == EX_rs))
					{
						//print_instruction(EX_ID.PC);
						EX_ID.IR = 0;
						EX_ID.PC = 0;
						EX_ID.SYS = 0;
					}
				}
				break;
			case 0x29:		//SH
			case 0x28:		//SB
			case 0x2B:		//SW
				if((MEM_rd != 0) && (MEM_rd == EX_rs))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}
				if((MEM_rd != 0) && (MEM_rd == EX_rt))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}
				break;
		}
	}

	if(((!ENABLE_FORWARDING) || (WB_op_code == 0xF)) && WB_op_code == 0x00) 
	{
		if(!(WB_op_code == 0x29 || WB_op_code == 0x2B || WB_op_code == 0x28))
		{
			if((WB_rd != 0) && (WB_rd == EX_rs))
			{
				//print_instruction(EX_ID.PC);
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYS = 0;
			}

			if((WB_rd != 0) && (WB_rd == EX_rt))
			{
				//print_instruction(EX_ID.PC);
				EX_ID.IR = 0;
				EX_ID.PC = 0;
				EX_ID.SYS = 0;
			}
		}
	}
	else if((!ENABLE_FORWARDING) || (WB_op_code == 0xF))
	{
		WB_rd = (0x1F0000 & WB_MEM.IR) >> 16;
		switch(WB_op_code) {
			case 0x8:		//ADDI
			case 0x9:		//ADDIU
			case 0xC:		//ANDI
			case 0xE:		//XORI
			case 0xD:		//ORI
			case 0xA:		//SLTI
			case 0x20:		//LB
			case 0x21:		//LH
			case 0xF:		//LUI
			case 0x23:		//LW
				if(!(WB_op_code == 0x29 || WB_op_code == 0x2B || WB_op_code == 0x28))
				{
					if((WB_rd != 0) && (WB_rd == EX_rs))
					{
						//print_instruction(EX_ID.PC);
						EX_ID.IR = 0;
						EX_ID.PC = 0;
						EX_ID.SYS = 0;
					}
				}
				break;
			case 0x29:		//SH
			case 0x28:		//SB
			case 0x2B:		//SW
				if((WB_rd != 0) && (WB_rd == EX_rs))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}

				if((WB_rd != 0) && (WB_rd == EX_rt))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}
				break;
		}
	}

	if(!ENABLE_FORWARDING)
	{
		MEM_rd = (0x1F0000 & MEM_EX.IR) >> 16;
		WB_rd = (0x1F0000 & WB_MEM.IR) >> 16;
		switch (op_code)
		{
			case 0x29:		//SH
			case 0x28:		//SB
			case 0x2B:		//SW
				if((MEM_rd != 0) && (MEM_rd == EX_rs))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}
				if((MEM_rd != 0) && (MEM_rd == EX_rt))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}	

				if((WB_rd != 0) && (WB_rd == EX_rs))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}

				if((WB_rd != 0) && (WB_rd == EX_rt))
				{
					//print_instruction(EX_ID.PC);
					EX_ID.IR = 0;
					EX_ID.PC = 0;
					EX_ID.SYS = 0;
				}		
				break;
		}
	}

	if (op_code == 0x00 && function == 0x0C)
		EX_ID.SYS = CURRENT_STATE.REGS[2];
}

/************************************************************/
/* instruction fetch (IF) pipeline stage:                                                              */ 
/************************************************************/
void IF()
{
	if(CYCLE_COUNT < 1 || ID_IF.SYS == 0xA || ((EX_ID.IR == 0 && EX_ID.PC == 0 && EX_ID.SYS == 0) && CYCLE_COUNT > 4))
	{
		return;
	}
	
	

	ID_IF.IR = mem_read_32(CURRENT_STATE.PC);
	ID_IF.PC = CURRENT_STATE.PC;
	NEXT_STATE.PC = CURRENT_STATE.PC + 4;
	

	uint32_t op_code = (ID_IF.IR & 0xFC000000) >> 26;
	uint32_t function = (ID_IF.IR & 0x3F);
	
	if (op_code == 0x00 && function == 0x0C)
		ID_IF.SYS = 0xA;
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
	/*IMPLEMENT THIS*/
	int i;
	uint32_t addr;
	
	for(i=0; i<PROGRAM_SIZE; i++){
		addr = MEM_TEXT_BEGIN + (i*4);
		printf("[0x%x]\t", addr);
		print_instruction(addr);
	}
}

void print_instruction(uint32_t addr){
	/*IMPLEMENT THIS*/
	uint32_t current_instruction_decimal = mem_read_32(addr);
	uint32_t op_code_mask = 0xFC000000;
	uint32_t op_code = (current_instruction_decimal & op_code_mask) >> 26;

	// Other masks
	uint32_t rs_mask = 0x3E00000, rt_mask = 0x1F0000, immediate_mask = 0xFFFF, rd_mask = 0xF800, shamt_mask = 0x7C0, funct_mask = 0x3F, offset_mask = 0x3FFFFFF;
	uint32_t rs = (rs_mask & current_instruction_decimal) >> 21;
	uint32_t rt = (rt_mask & current_instruction_decimal) >> 16;
	uint32_t immediate = (immediate_mask & current_instruction_decimal);
	uint32_t rd = (rd_mask & current_instruction_decimal) >> 11;
	uint32_t shamt = (shamt_mask & current_instruction_decimal) >> 6;
	uint32_t function = (funct_mask & current_instruction_decimal);
	uint32_t offset = (offset_mask & current_instruction_decimal);

	
	// Function opcode (R-Type)
	if (op_code == 0x00) {
		switch(function) {
			case 0x00:		//SLL
				printf("SLL $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x02:		//SRL
				printf("SRL $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x03:		//SRA
				printf("SRA $%d, $%d, 0x%x\n", rd, rt, shamt);
				break;
			case 0x08:		//JR
				printf("JR $%d\n", rs);
				break;
			case 0x09:		//JALR
				printf("JALR $%d, $%d\n", rs, rd);
				break;
			case 0x0C:		//SYSCALL
				printf("SYSCALL\n");
				break;
			case 0x10:		//MFHI
				printf("MFHI $%d\n", rd);
				break;
			case 0x11:		//MTHI
				printf("MTHI $%d\n", rs);
				break;
			case 0x12:		//MFLO
				printf("MFLO $%d\n", rd);
				break;
			case 0x13:		//MTLO
				printf("MTLO $%d\n", rs);
				break;
			case 0x18:		//MULT
				printf("MULT $%d, $%d\n", rs, rt);
				break;
			case 0x19:		//MULT Unsigned
				printf("MULTU $%d, $%d\n", rs, rt);
				break;
			case 0x1A:		//DIV
				printf("DIV $%d, $%d\n", rs, rt);
				break;
			case 0x1B:		//DIVU
				printf("DIVU $%d, $%d\n", rs, rt);
				break;
			case 0x20:		//ADD
				printf("ADD $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x21:		//ADD Unsigned
				printf("ADDU $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x22:		//SUB
				printf("SUB $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x23:		//SUB Unsigned
				printf("SUBU $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x24:		//AND
				printf("AND $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x25:		//OR
				printf("OR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x26:		//XOR
				printf("XOR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x27:		//NOR
				printf("NOR $%d, $%d, $%d\n", rd, rs, rt);
				break;
			case 0x2A:		//SLT
				printf("SLT $%d, $%d, $%d\n", rd, rs, rt);
				break;
		}
	} 
	// Regular opcode (I-Type)
	else {
		switch(op_code) {
			case 0x8:		//ADDI
				// make immediate sign extended
				printf("ADDI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0x9:		//ADDIU
				// should be unsigned?
				printf("ADDIU $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0xC:		//ANDI
				//NEXT_STATE.REGS[rt] = (0000000000000000 | (immediate & (CURRENT_STATE.REGS[rs] & sixteen_bit_mask)));
				printf("ANDI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0xE:		//XORI
				printf("XORI $%d, $%d, 0x%x\n", rt, rs, immediate);	
				break;
			case 0xD:		//ORI
				printf("ORI $%d, $%d, 0x%x\n", rt, rs, immediate);	
				break;
			case 0xA:		//SLTI
				printf("SLTI $%d, $%d, 0x%x\n", rt, rs, immediate);
				break;
			case 0x4:		//BEQ
				// Immediate is used in the branch instructions since its a 16 bit masked value
				printf("BEQ $%d, $%d, 0x%x\n", rs, rt, (uint32_t)((int16_t)(immediate * 4)));
				break;
			case 0x1:		//BGEZ, BLTZ
				if (rt == 1) {
					// BGEZ
					printf("BGEZ $%d, 0x%x\n", rs, (uint32_t)((int16_t)(immediate * 4)));
				} else if (rt == 0) {
					// BLTZ
					printf("BLTZ $%d, 0x%x\n", rs, (uint32_t)((int16_t)(immediate * 4)));
				}
				break;
			case 0x7:		//BGTZ
				printf("BGTZ $%d, 0x%x\n", rs, (uint32_t)((int16_t)(immediate * 4)));
				break;
			case 0x6:		//BLEZ
				printf("BLEZ $%d, 0x%x\n", rs, (uint32_t)((int16_t)(immediate * 4)));
				break;
			case 0x5:		//BNE
				printf("BNE $%d, $%d, %d\n", rs, rt, (uint32_t)((int16_t)(immediate * 4)));
				break;
			case 0x2:		//J
				printf("J 0x%x\n", offset);
				break;
			case 0x3:		//JAL
				printf("JAL 0x%x\n", offset);
				break;
			case 0x20:		//LB
				printf("LB $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x21:		//LH
				printf("LH $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0xF:		//LUI
				printf("LUI $%d, 0x%x\n", rt, immediate);
				break;
			case 0x23:		//LW
				printf("LW $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x29:		//SH
				printf("SH $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x28:		//SB
				printf("SB $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
			case 0x2B:		//SW
				printf("SW $%d, 0x%x($%d)\n", rt, immediate, rs);
				break;
		}
	}
}

/************************************************************/
/* Print the current pipeline                                                                                    */ 
/************************************************************/
void show_pipeline(){
	printf("Current PC: 		%X\n", CURRENT_STATE.PC);
	printf("IF/ID.IR			%X  ", ID_IF.IR);
	print_instruction(ID_IF.PC);
	printf("IF/ID.PC			%X\n", ID_IF.PC);
	printf("\n");

	printf("ID/EX.IR			%X  ", EX_ID.IR);
	print_instruction(EX_ID.PC);
	printf("ID/EX.A				%X\n", EX_ID.A);
	printf("ID/EX.B				%X\n", EX_ID.B);
	printf("ID/EX.imm			%X\n", EX_ID.imm);
	printf("\n");

	printf("EX/MEM.IR			%X  ", MEM_EX.IR);
	print_instruction(MEM_EX.PC);
	printf("EX/MEM.A			%X\n", MEM_EX.A);
	printf("EX/MEM.B			%X\n", MEM_EX.B);
	printf("EX/MEM.ALUOutput	%X\n", MEM_EX.ALUOutput);
	printf("EX/MEM.ALUOutput2	%X\n", MEM_EX.ALUOutput2);
	printf("\n");

	printf("MEM/WEB.IR			%X  ", WB_MEM.IR);
	print_instruction(WB_MEM.PC);
	printf("MEM/WEB.A			%X\n", WB_MEM.IR);
	printf("MEM/WEB.LMD			%X\n", WB_MEM.IR);
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
