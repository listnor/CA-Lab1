#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>

using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
#define MemSize 65536 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

class RF {

public:

	bitset<32> ReadData1, ReadData2;
	vector<bitset<32> > Registers;

	RF()
	{
		Registers.resize(32);
		Registers[0] = bitset<32>(0);
		for(unsigned i=0;i<32;++i)
			Registers[i] = bitset<32>(1);
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
			ALUresult = oprand1.to_ulong() + oprand2.to_ulong();break;
		case SUBU:
			ALUresult = oprand1.to_ulong() - oprand2.to_ulong();break;
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

		if (writemem.to_ulong()) 
			for (unsigned i = 0; i < 4; ++i) 
                        	for (unsigned j = 0; j < 8; ++j)
				DMem[Address.to_ulong() + i][j] = WriteData[8 * i + j];

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
	bitset < 32 > addressExample = 0; bitset < 32 > insExample = 0;
	unsigned int opcode = 0;
	while (1)
	{
		// Fetch
		insExample = myInsMem.ReadMemory(addressExample);
		addressExample = addressExample.to_ulong() + 4;
		// If current insturciton is "11111111111111111111111111111111", then break;
		if (insExample.to_ulong() == 4294967295) break;
		// decode(Read RF)
		opcode = insExample.to_ulong() & 0xFC000000;
		cout<<"instrcution is "<<insExample<<endl<<"opcode is "<<opcode<<endl;

		switch (opcode){
		case 0:
		{
			bitset<3> func = insExample.to_ulong() & 0x7;
			bitset<5> rs = (insExample.to_ulong() & 0x03E00000)>>21;
			bitset<5> rt = (insExample.to_ulong() & 0x001F0000)>>16;
			cout<<myRF.Registers[rs.to_ulong()]<<"     "<<myRF.Registers[rt.to_ulong()]<<endl;
			myALU.ALUOperation(func, myRF.Registers[rs.to_ulong()], myRF.Registers[rt.to_ulong()]);
			cout<<"ALUresult is "<<myALU.ALUresult<<endl<<"function code is "<<func.to_ulong()
			    <<" ,rs is "<<rs.to_ulong()<<" ,rt is "<<rt.to_ulong();
			break;
		}

		default: break;
		}
		// Execute

		// Read/Write Mem

		// Write back to RF
		bitset<5> rd = (insExample.to_ulong() & 0x0000F800)>>11;
		cout<<" ,rd is "<< rd.to_ulong()<<endl;
		myRF.Registers[rd.to_ulong()] = myALU.ALUresult;
		cout<<"Result in register "<<rd.to_ulong()<<" is "<<myALU.ALUresult.to_ulong()<<endl;
		myRF.OutputRF(); // dump RF;    
	}
	myDataMem.OutputDataMem(); // dump data mem

	return 0;

}
