#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
#include <math.h>

using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

	
long to_long32(bitset<32> &input){
	long output = - input[31] *pow(2,32) + input.to_ulong();
	return output;
}

long to_long16(bitset<16> &input){
	long output = - input[15] *pow(2,16) + input.to_ulong();
	return output;
}		

class RF {

public:

	bitset<32> ReadData1, ReadData2;
	vector<bitset<32> > Registers;

	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
	}

	void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
	{
	     // implement the funciton by you.
		ReadData1 = Registers[RdReg1.to_ulong()];
		ReadData2 = Registers[RdReg2.to_ulong()];
		if (WrtEnable.to_ulong()) Registers[WrtReg.to_ulong()] = WrtData;
	}

	void OutputRF()
	{
		ofstream rfout;
		rfout.open("RFresult.txt", std::ios_base::app);
		if (rfout.is_open())
		{
			rfout << "A state of RF:" << endl;
			for (int j = 0; j<32; j++)
			{
				rfout << Registers[j] << endl;
			}

		}
		else cout << "Unable to open file";
		rfout.close();
	}

};

class ALU
{

public:

	bitset<32> ALUresult;

	bitset<32> ALUOperation(bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2) {

		// implement the ALU operations by you.				
		switch (ALUOP.to_ulong()) {

		case ADDU:
			ALUresult = -oprand1[31]*pow(2,32)+oprand1.to_ulong() +
					-oprand2[31]*pow(2,32) + oprand2.to_ulong();break;
		case SUBU:
			-oprand1[31]*pow(2,32)+oprand1.to_ulong() -
					(-oprand2[31]*pow(2,32) + oprand2.to_ulong());break;
		case AND:
			ALUresult = oprand1 & oprand2;break;

		case OR:
			ALUresult = oprand1 | oprand2;break;

		case NOR:
			ALUresult = oprand1 ^ oprand2;break;

		default:
			break;
		}

		return ALUresult;
	}
};

class INSMem
{
public:

	bitset<32> Instruction;

	INSMem()
	{
		IMem.resize(MemSize);
		ifstream imem;
		string line;
		int i = 0;
		imem.open("imem.txt");
		if (imem.is_open()) {
			while (getline(imem, line)) {
				IMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		imem.close();
	}

	bitset<32> ReadMemory(bitset<32> ReadAddress) {
		// implement by you. (Read the byte at the ReadAddress and the following three byte).
		Instruction = 0;
		for (unsigned i = 0; i < 4; ++i) {
			Instruction <<= 8;
			Instruction =Instruction.to_ulong() + IMem[ReadAddress.to_ulong() + i].to_ulong();
		}
		return Instruction;
	}

private:

	vector<bitset<8> > IMem;

};

class DataMem
{
public:
	bitset<32> readdata;

	DataMem()
	{
		DMem.resize(MemSize);
		ifstream dmem;
		string line;
		int i = 0;
		dmem.open("dmem.txt");
		if (dmem.is_open())
		{
			while (getline(dmem, line))
			{
				DMem[i] = bitset<8>(line);
				i++;
			}
		}
		else cout << "Unable to open file";
		dmem.close();
	}

	bitset<32> MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
	{
		// implement by you.
		if (readmem.to_ulong()) {
			readdata = 0;
			for (unsigned i = 0; i < 4; ++i) {
				readdata <<= 8;
				readdata = readdata.to_ulong() + DMem[Address.to_ulong() + i].to_ulong();
			}
		}

		if (writemem.to_ulong()) {
			for (unsigned i = 0; i < 4; ++i) 
                        	for (unsigned j = 0; j < 8; ++j)
				DMem[Address.to_ulong() + i][7 - j] = WriteData[31-(8 * i + j)];
		}

		return readdata;
	}

	void OutputDataMem()
	{
		ofstream dmemout;
		dmemout.open("dmemresult.txt");
		if (dmemout.is_open())
		{
			for (int j = 0; j< 1000; j++)
				dmemout << DMem[j] << endl;
		}
		else cout << "Unable to open file";
		dmemout.close();
	}

private:

	vector<bitset<8> > DMem;

};

int main()
{
	RF myRF;
	ALU myALU;
	INSMem myInsMem;
	DataMem myDataMem;

	bitset < 32 > addressExample = 0;//The address of Instruction
    bitset < 32 > insExample = 0;	//The instruction
	unsigned pass=0; //The number of circles
	bitset<6> opcode = 0; //Type code
	while (1)
	{
		// Fetch
		insExample = myInsMem.ReadMemory(addressExample);
		addressExample = addressExample.to_ulong() + 4;
		cout<<endl<<"The "<<pass++<<" instruction"<<endl;
		cout<<"The Instruction Address is "<<addressExample.to_ulong()<<endl;

		// decode(Read RF)
		opcode = (insExample.to_ulong() & 0xFC000000)>>26;
		cout<<"instrcution is "<<insExample<<endl<<"opcode is "<<opcode<<endl;

		switch (opcode.to_ulong()){ //Judge operation type by opcode
		case 0: // R type
		{
			bitset<3> func = insExample.to_ulong() & 0x7;
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			bitset<5> rd = (insExample.to_ulong() & 0x0000F800)>>11;
			myALU.ALUOperation(func, myRF.Registers[rs.to_ulong()], myRF.Registers[rt.to_ulong()]);
			cout<<"It's R type and function code is "<<func.to_ulong()<<endl
			    <<"rs is Register["<<rs.to_ulong()<<"]"
                <<",rt is Register["<<rt.to_ulong()<<"]"
			    <<",rd is Register["<< rd.to_ulong()<<"]"<<endl;

			myRF.Registers[rd.to_ulong()] = myALU.ALUresult;

			cout<<"Calculation Result in Register["<<rd.to_ulong()<<"] is "<<myALU.ALUresult.to_ulong()<<endl;
			break;
		}
		case 9://Addiu
		{
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			bitset<16> imm = insExample.to_ulong() & 0x0000FFFF;
			
			cout<<"It's addiu operation"
			    <<" ,rs is "<<rs.to_ulong()
                <<" ,rt is "<<rt.to_ulong()
			    <<" ,im is "<< imm.to_ulong()<<endl;

			myRF.Registers[rt.to_ulong()] = to_long32(myRF.Registers[rs.to_ulong()]) + to_long16(imm);
			cout<<"Result is Register["<<rt.to_ulong()<<"]="<<myRF.Registers[rt.to_ulong()]<<endl;
			break;
		}
		case 4://beq
		{
			cout<<"It's beq operation"<<endl;
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			bitset<18> imm = (insExample.to_ulong() & 0x0000FFFF)<<2;

			if(rs==rt){
				addressExample = addressExample.to_ulong() - pow(2,18) * imm[17] + imm.to_ulong();
				cout<<"rs==rt so beq to"<<endl<<addressExample.to_ulong();
			}
			else
				cout<<"No beq"<<endl;

			break;
		}
		case 2://j
		{
			cout<<"It's j operation"<<endl;
			bitset<28> imm = insExample.to_ulong() & 0x03FFFFFF;
			bitset<32> exImm = (addressExample.to_ulong() + 4) & 0xF0000000;		
			           exImm = exImm.to_ulong() + (imm<<2).to_ulong();
			addressExample = exImm.to_ulong() - 4;	
			cout<<"Jump to "<<addressExample<<endl;	
			break;
		}	
		case 35://lw
		{
			cout<<"It's lw operation"<<endl;
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			bitset<16> imm = insExample.to_ulong() & 0x0000FFFF;

			myRF.Registers[rt.to_ulong()] = myDataMem.MemoryAccess(to_long32(myRF.Registers[rs.to_ulong()]) + to_long16(imm), 0, 1, 0);

			cout<<"load Memory["<<to_long32(myRF.Registers[rs.to_ulong()]) + to_long16(imm)<<"] to "
				<<"Register["<<rt.to_ulong()<<"]"<<endl;
			break;
		}
		case 43://sw
		{
			cout<<"It's sw operation"<<endl;
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			bitset<16> imm = insExample.to_ulong() & 0x0000FFFF;

			myDataMem.MemoryAccess(to_long32(myRF.Registers[rs.to_ulong()]) + to_long16(imm), 
								   to_long32(myRF.Registers[rt.to_ulong()]), 0, 1) ;

			cout<<"save Register["<<rt.to_ulong()<<"] to "
				<<"Memory["<<to_long32(myRF.Registers[rs.to_ulong()]) + to_long16(imm)<<"]"<<endl;
			break;
		}		

		default: cout<<"Calculation Haut"<<endl; return 0;
		
		}

		myRF.OutputRF(); // dump RF; 
		cout << endl;   
	}
	myDataMem.OutputDataMem(); // dump data mem

	return 0;

}
