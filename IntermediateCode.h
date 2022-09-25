#pragma once
namespace Iasm{
	namespace Opcode {
	#define CREATE_OPCODE(...)\
		enum {\
			__VA_ARGS__\
		};\
		string names_conjoined = #__VA_ARGS__;

		CREATE_OPCODE(
			CREATE_FUNCTION, CREATE_VARIABLE,
			CREATE_EXP, POP_EXP,
			OPENSCOPE, CLOSESCOPE,

			MOV, MOV2, READ,
			ADD, SUB, DIV, MUL, MOD,	

			PUSH_ARG, WRITE_ARG,

			JMP,CALL,RET,

			CJMP, SETE, SETNE, SETG, SETL,
			LABEL,

			PRINT_STRING,
			PRINT_NUMBER,
			INPUT
		);

		vector<string> names;
	#undef CREATE_OPCODE
		
		//takes the names_conjoined and splits them into names
		void Init() {
			names_conjoined = strReplace(names_conjoined, " ", "");
			strSplit(names_conjoined, ',', &names);
		}
	}
	namespace Oprand{
		enum {
			TYPE,
			CONST,
			NAME,
			REG,
			LABEL_INDEX,
			GAP
		};
	}
	//registers
	enum {
		REG1, REG2
	};
	//vector<string> regNameList = { "REG1", "REG2" };
	vector<string> regNameList = { "EAX", "EBX" };
	struct Instruction {
		//oprand type

		lli opcode;
		vector<string> oprand_string;
		vector<lli> oprand_int;
		vector<long double> oprand_float;//TODO not implimented
		vector<lli> oprand_type;

		//for Env 
		bool is_breakpoint = false;

		Instruction() = default;
		Instruction(lli opcode) {
			self.opcode = opcode;
		}
		Instruction(lli opcode, lli oprand, lli oprand_type) {
			self.opcode = opcode;
			oprand_int.push_back(oprand);
			self.oprand_type.push_back(oprand_type);
			oprand_string.push_back("");
		}
		Instruction(lli opcode,vector<string> oprand_string, vector<lli> oprand_type) {
			self.opcode = opcode;
			self.oprand_string = oprand_string;
			self.oprand_type = oprand_type;
		}
		Instruction(lli opcode, vector<lli> oprand_int, vector<lli> oprand_type) {
			self.opcode = opcode;
			self.oprand_int = oprand_int;
			self.oprand_type = oprand_type;
		}

		void AppendOprand(lli value, lli oprand_type) {
			self.oprand_type.push_back(oprand_type);
			oprand_int.push_back(value);
			oprand_string.push_back("");			
		}
		void AppendOprand(string value, lli oprand_type) {
			self.oprand_type.push_back(oprand_type);
			oprand_string.push_back(value);
			oprand_int.push_back((lli)sbitMax64);			
		}
		string StringOprand(lli idx) {
			string out;
#if safe == true
			if (idx >= oprand_type.size()) {
				err("Invalid index");
			}
#endif
			switch (oprand_type[idx]) {
			case Oprand::TYPE:
				out = typeNameList[oprand_int[idx]];
				break;
			case Oprand::CONST:
				out = to_string(oprand_int[idx]);
				break;
			case Oprand::NAME:
				out = oprand_string[idx];
				break;
			case Oprand::REG:
				out = regNameList[oprand_int[idx]];
				break;
			case Oprand::LABEL_INDEX:
				out = "LABEL_INDEX_" + to_string(oprand_int[idx]);
				break;
			case Oprand::GAP:
				out = "GAP";
				break;	
			default:
				err("Invalid oprand type: "+to_string(oprand_type[idx]));
				break;
			}
			return out;
		}
		string PrintInstruction(const bool ret = false) {
			string out = "";
			out += Opcode::names[opcode] + " ";			
			loop(i, 0, oprand_type.size()) {
				out += StringOprand(i);
				if (i != oprand_type.size() - 1) {
					out += ", ";
				}
			}
			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
	};
	

	string PrintInstructionList(vector<Iasm::Instruction> instList, const bool ret = false) {
		string out;

		lli lineCount = 0;

		out += "--Instruction dump--\n";
		string tabList;
		fin(i, instList) {
			if (i.opcode == Opcode::OPENSCOPE)tabList += '\t';	
			out += to_string(lineCount) + "| ";
			out += tabList;
			out += i.PrintInstruction(true) + "\n";
			if (i.opcode == Opcode::CLOSESCOPE)tabList.pop_back();
			lineCount++;
		}
		out += "----\n";

		if (ret == false) {
			print(out);
			return "";
		}
		return out;
	}
}