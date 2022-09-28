#pragma once
#include "IntermediateCode.h"
namespace Env{
	namespace Settings {
		bool debug = true;
		bool time = true;
	}
	ulli register1;
	ulli register2;
#if safe == true
	vector<ulli*> registerPtrList = { &register1, &register2 };
#else
	const ulli* registerPtrList[2] = { &register1, &register2 };
#endif

	struct Stack {
		byte* stackPtr = nullptr;
		lli stackSize = 1 * kb;
		lli usedStack = 0;
		vector<lli> stackReturnPositions;

		Stack() = default;
		void Init() {
#if safe == true
			if (stackPtr != nullptr) {
				err("Memory leack");
			}
#endif
			stackPtr = new byte[stackSize];
		}

		template<typename T> T& Get (lli i) {
#if safe == true
			if (i < 0 or i >= usedStack) {
				err("Invalid indexing on the stack index -> " + to_string(i));
			}
#endif
			return $((T*)(stackPtr + i));
		}
		byte& operator[] (lli i) {
#if safe == true
			if (i < 0 or i >= usedStack) {
				err("Invalid index -> " + to_string(i));
			}
#endif
			return $((byte*)(stackPtr + i));
		}
		
		byte* alloc(lli size) {
			usedStack += size;
			return stackPtr + usedStack - size;		
		}
		void Free(lli size) {
#if safe == true
			if (size < 0) {
				err("Freeing size less than 0");
			}
#endif
			usedStack -= size;
		}

		template<typename T>void WriteValue(T var) {
			byte* ptr = alloc(sizeof(T));
			$((T*)(ptr)) = var;
		}


		//saves the value in register1 for arithmatic opirations
		void SaveState() {
			if (usedStack + sizeof(ulli) > stackSize){
#if safe == true
				print("Stack realocation to size -> ", stackSize);
#endif
				stackSize *= 2;
				delete[] stackPtr;
				stackPtr = new byte[stackSize];
			}
			$(stackPtr + usedStack) = register1;
			usedStack += sizeof(ulli);
		}
		void ReadState() {
			register1 = $(stackPtr + usedStack);
			usedStack -= sizeof(ulli);
		}

		void StackDump() {
			loop(i, 0, usedStack) {
				printn((int)$(stackPtr + i), " ");
			}
			print();
		}

		~Stack() {
#if safe == true
			if (stackPtr == nullptr) {
				err("Trying to delete the stack when it was never allocated.\n"
					"Not necessrly bad but odd.");
			}
#endif
			delete[] stackPtr;
		}
	};
	Stack stack;

	struct Variable {
		bool is_startOfFunction = false;

		string name;
		lli ptr;
		lli dataType;
		lli dataSize_bytes;
		lli dataSize_total;//if list combined size of all elements
		bool is_list;
		bool is_ptr;
		bool is_float;
		bool is_unsigned;

		void Init(string name, lli typeinfo, lli listSize = 1,
			bool is_list = false, bool is_ptr = false, bool is_float = false, bool is_unsigned = false) {
			self.name = name;
			self.is_list = is_list;
			self.is_float = is_float;
			self.is_ptr = is_ptr;
			self.is_unsigned = is_unsigned;
			dataType = typeinfo;
#define c(type) if(dataType == TypeIdList::type){\
					dataSize_bytes = typeSizeList[TypeIdList::type];\
					dataSize_total = dataSize_bytes * listSize;\
					goto foundType;\
				}
			c(INT);
			c(FLOAT);
			c(VOID);
#undef c
			foundType:			
			ptr = stack.usedStack;	
			stack.usedStack += dataSize_total;
		}
	};
	vector<vector<Variable>> scopeVarList;

	vector<lli> functionStackFrame;

	//struct Exp {
	//	ulli register1_savedState;
	//};
	//vector<Exp> expList;

	struct RunTimeEnvData {
		lli currentInstruction;
		lli currentFunction;
		vector<AST::Function>* functionListPtr;
	};

	vector<vector<bool>> breakPointList;

	void Init() {
		stack.Init();
	}


	namespace IO {
		struct Console {
			string context;

			void writeNumber(ulli arg) {
				context += to_string(arg);
				context += "\n";
			}
			void writeString(ulli ptr) {
				//TODO error checking
				while (stack[ptr] != 0) {
					context += stack[ptr];
					ptr++;
				}
				context += "\n";
			}
			
			void PrintConsole() {
				print(context);
			}
		};
		Console console;
	}
	namespace Debug {
		string PrintRegisters(const bool ret = false) {
			string out;
			out += "EAX = " + to_string(register1) + "\n";
			out += "EBX = " + to_string(register2) + "\n";
			out += "ESP = " + to_string(stack.usedStack) + "\n";
			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
		//returns all varaibles
		string DumpVaraibles() {
			string out;
			loop(s, 0, scopeVarList.size()) {
				loop(i, 0, scopeVarList[s].size()) {
					ulli value = 0;
					memcpy(&value, scopeVarList[s][i].ptr + stack.stackPtr, scopeVarList[s][i].dataSize_bytes);
					out += scopeVarList[s][i].name + " = " + to_string(value);
					out += "\n";
				}
			}
			return out;
		}
	}

	lli FindFunctionIndex(string functionName, vector<AST::Function>& functionList) {
		lli index = -1;
		loop(f, 0, functionList.size()) {
			if (functionList[f].name == functionName) {
				if (index == -1) {
					index = f;
				}
				else {
					err("Multiple " + functionName + " decliration");
				}
			}
		}
		if (index == -1) {
			err("Could not find " + functionName + " function");
		}
		return index;
	}
	//returns scope level followed by index
	struct FindVarWithName_return {
		lli scopeLevel = -1;
		lli index = -1;
	};
	FindVarWithName_return FindVarWithName(string name) {//std::tuple<lli,lli>
		FindVarWithName_return r;
		loopR(s, 0, scopeVarList.size()) {
			r.index = indexVec_lambda<string, Variable>(name, scopeVarList[s], [](string& a, Variable& b) -> bool {
				if (a == b.name) {
					return true;
				}
				return false;
				}, -1);
			if (r.index != -1) {
				r.scopeLevel = s;
				break;
			}
		}
		if (r.scopeLevel == -1 or r.index == -1) {
			string nameList = "[";
			loop(i, 0, scopeVarList.size()) {
				nameList += "[";
				loop(j, 0, scopeVarList[i].size()) {
					nameList += scopeVarList[i][j].name + ", ";
				}				
				nameList += "],";
			}
			nameList += "]";
			err("Could not find variable, indexs are -1, looking for ->" + name + " In List\n " + nameList);
		}
		return r;//std::tuple<lli, lli>(scopeLevel, index)
	}


	//Map labels to their instruction address
	vector<AST::Function> MapLabels(vector<AST::Function>& functionList_argument) {
		vector<AST::Function> functionList = functionList_argument;
		struct Name_Address {
			lli labelName;
			lli labelAddress;
		};
		vector<Name_Address> labelList;
		lli labelAddress = 0;
		loop(f, 0, functionList.size()) {
			loop(i, 0, functionList[f].instructionList.size()) {
				Iasm::Instruction& currentInst = functionList[f].instructionList[i];
				if (currentInst.opcode == Iasm::Opcode::LABEL) {
					Name_Address na;
					na.labelName = currentInst.oprand_int[0];
					//na.labelAddress = labelAddress-1;
					na.labelAddress = i;//labelAddress
					//TODO could have two labels with the same index resulting in error
					labelList.push_back(na);
					//functionList[f].instructionList.erase(functionList[f].instructionList.begin() + i);
					//i--;
				}
				{//else
					labelAddress++;
				}				
			}
		}
		loop(f, 0, functionList.size()) {
			loop(i, 0, functionList[f].instructionList.size()) {
				Iasm::Instruction& currentInst = functionList[f].instructionList[i];
				loop(a, 0, currentInst.oprand_type.size()) {
					if (currentInst.oprand_type[a] == Iasm::Oprand::LABEL_INDEX) {
						lli labelIndex = indexVec_lambda<lli, Name_Address>(
							currentInst.oprand_int[a], labelList,
							[](lli& a, Name_Address& b) -> bool {	return (a == b.labelName); });
#if safe == true
						if (labelIndex == -1) {
							err("Failed to find label");
						}
#endif
						currentInst.oprand_int[a] = labelList[labelIndex].labelAddress;
					}
				}				
			}
		}
		return functionList;
	}
	void CopyStringsToStack() {
		lli usedSize = 0;
		fin(s, Lexer::charArrayList) {
#if safe == true
			if (stack.stackPtr == nullptr) {
				err("here");
			}
#endif
			memcpy(stack.stackPtr + (ulli)s.ptr, s.content.c_str(), s.content.size() + 1);
			usedSize += s.content.size() + 1;
		}
		stack.usedStack += usedSize;
	}
	fnPtr(void, executeFunctionPtr, vector<AST::Function>& functionList, lli functionIndex);
	lli ExecuteInstruction(lli inst_idx, vector<Iasm::Instruction>& instructionList,
		vector<AST::Function>& functionList, RunTimeEnvData* rted)
	{
		Iasm::Instruction& instruction = instructionList[inst_idx];
		lli newInstructionIdx = inst_idx;
		using namespace Iasm::Opcode;

		auto getArgumentValue = [&](lli oprand_type, lli oprand_int, string& oprand_string) -> ulli{
			using namespace Iasm::Oprand;
			ulli return_value = 0;
			switch (oprand_type) {
				case NAME: {
					//loopR(s, 0, scopeVarList.size()) {

//						if (index == -1) { continue; }/
//						Variable* var = scopeVarList[s][index];
//						return var->value;
//					}	
					//std::tuple<lli, lli> index_tup = FindVarWithName(oprand_string);
					//lli scopeLevel = std::get<0>(index_tup);
					//lli index = std::get<1>(index_tup);
					//if (scopeLevel != -1 and index != -1) {
					//	Variable* var = scopeVarList[scopeLevel][index];
					//	return var->value;
					//}
					FindVarWithName_return r = FindVarWithName(oprand_string);
					if (r.scopeLevel != -1 and r.index != -1) {
						Variable& var = scopeVarList[r.scopeLevel][r.index];
						memcpy(&return_value, stack.stackPtr + var.ptr, var.dataSize_bytes);//TODO list dont work
						return return_value;
					}

					print("-------ERROR------");
					print("Failed to find variable of name: (" + oprand_string + ")");
					print("In list: ");
					loopR(s, 0, scopeVarList.size()) {
						string slist = "[";
						loop(i, 0, scopeVarList[s].size()) {
							slist += scopeVarList[s][i].name;
							if (i != scopeVarList[s].size() - 1) {
								slist += ", ";
							}
						}
						slist += "]";
						print(slist);
					}
					exit(0);

				}
				case CONST: {
					return (ulli)oprand_int;
				}
				case REG: {
					return $(registerPtrList[oprand_int]);
				}
#if safe == true
				default:{
					err("Oprand_type does not match any oprand types");
				}
#endif
			}
			err("Failed");
			return 0;
		};

		//parse to ide
		if (Settings::debug) {
#if safe == true
			if (IDE_Handler::handler.EnvDebugger == nullptr) {
				err("Trying to call a nullptr.");
			}
#endif
			IDE_Handler::handler.EnvDebugger(rted);
			/*
			if (instruction.is_breakpoint == true) {
				print("Breakpoint Found.");
				print("Stack dump.");
				stack.StackDump();
				print("\nPausing press enter to continue");
				system("pause");
			}
			*/
			string instName;
			if (instruction.opcode >= 0 and instruction.opcode < names.size()) {
				instName = names[instruction.opcode];
			}
			else {
				instName = "(Failed to get instruction name)";
			}
			printn("Instruction being interprited: " + instruction.PrintInstruction(true));
			if (instruction.oprand_int.size() != instruction.oprand_type.size() or instruction.oprand_string.size() != instruction.oprand_type.size()) {
				printn("\t- oprand_int and oprand_string are differnt sizes ERROR!!!");
			}
			print();
		}	
		//----------instruction block---------------
		switch (instruction.opcode) {
		case OPENSCOPE: {
			scopeVarList.emplace_back(vector<Variable>());
			stack.stackReturnPositions.emplace_back(stack.usedStack);
			break;
		}
		case CLOSESCOPE: {
			stack.usedStack = stack.stackReturnPositions.back();
			stack.stackReturnPositions.pop_back();
			scopeVarList.pop_back();		
			break;
		}
		case CREATE_VARIABLE: {
			//Variable* varPtr = new Variable();
			//variableList.push_back(varPtr);
			//variableList.back()->name = instruction.oprand_string[1];
			//variableList.back()->type = instruction.oprand_int[0];
			//variableList.back()->Init();
			//scopeVarList.back().push_back(varPtr);
			scopeVarList.back().emplace_back(Variable());
			Variable& var = scopeVarList.back().back();
			var.Init(instruction.oprand_string[1], instruction.oprand_int[0]);			
			break;
		}
		case CREATE_FUNCTION: {
			functionStackFrame.push_back(stack.usedStack);
			break;
		}
		//Moving memory
		case WRITE_ARG: {
			lli argIndex = instruction.oprand_int[0];
			ulli arg = getArgumentValue(instruction.oprand_type[2], instruction.oprand_int[2], instruction.oprand_string[2]);
			lli beginFunctionPtr = functionStackFrame.back();
			stack.Get<ui32>(beginFunctionPtr - (argIndex * 4) - 4 - 4) = (ui32)arg;
			//                                    ^ hard coded BAD
			break;
		}
		case MOV:{
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 = arg;
			break;
		}
		case MOV2: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register2 = arg;
			break;
		}
		case READ: {
			string& varName = instruction.oprand_string[0];
			FindVarWithName_return r = FindVarWithName(varName);
#if safe == true
			if (scopeVarList[r.scopeLevel][r.index].dataSize_bytes != scopeVarList[r.scopeLevel][r.index].dataSize_total) {
				err("Not going to work.");
			}
#endif
			memcpy(stack.stackPtr + scopeVarList[r.scopeLevel][r.index].ptr,
				&register1, scopeVarList[r.scopeLevel][r.index].dataSize_bytes);
			break;
		}
		//Expressions
		//case CREATE_EXP: {
			//expList.push_back(Exp());
		//	expList.emplace_back(Exp());
		//	expList.back().register1_savedState = register1;
		//	break;
		//}
		//case POP_EXP: {
		//	register2 = register1;
		//	register1 = expList.back().register1_savedState;
		//	expList.pop_back();
		//	break;
		//}
		//Maths
		case ADD: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 += arg;
			break;
		}
		case MUL: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 *= arg;
			break;
		}
		case MOD: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 = register1 % arg;
			break;
		}
		case SUB: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 = register1 - arg;
			break;
		}
		case DIV: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			register1 = register1 / arg;
			break;
		}
#define c(instructionName,opp) 	case instructionName: {\
				ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);\
				register1 = (register1 opp arg);\
				break;\
			}
		c(SETE, == );
		c(SETG, > );
		c(SETL, < );
		c(SETNE, != );
#undef c
		//Jumps
		case CJMP: {
			if (register1 == 0) {
				newInstructionIdx = instruction.oprand_int[1];
			}
			break;                           
		}
		case CALL:{
			stack.WriteValue<ui32>(inst_idx);//<-technicly not neede only kept for asm compilation debugging
			string& functionName = instruction.oprand_string[0];
			lli functionIndex = FindFunctionIndex(functionName, functionList);
			executeFunctionPtr(functionList, functionIndex);
			break;
		}
		case JMP: {
			newInstructionIdx = instruction.oprand_int[0];
			break;
		}	
#if safe == true
		case LABEL: {
			//err("Should not be labels in code that is to be interprited. Labels might not have been mapped to "
			//	"instruction indexes");
			break;
		}
#endif
		//IO
		case PRINT_STRING: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			IO::console.writeString(arg);
			break;
		}
		case PRINT_NUMBER: {
			ulli arg = getArgumentValue(instruction.oprand_type[0], instruction.oprand_int[0], instruction.oprand_string[0]);
			IO::console.writeNumber(arg);
			break;
		}
		case RET: {
			functionStackFrame.pop_back();
			//free return address 32 bit!!!!
			if (stack.usedStack != 0) {
				//dont free if exiting main because stack empty
				stack.Free(sizeof(ui32));
			}			
			break;
		}
#if safe == true
		default: {
			string instructionName;
			if (instruction.opcode >= 0 and instruction.opcode < names.size()) {
				instructionName = names[instruction.opcode];
			}			
			//err("Instruction did not match any instructions opcode: "+to_string(instruction.opcode));
			print("!!!!!!!FAILED TO COMPILE INSTRUCTION!!!!!: "
				+ instructionName+"  Instruction id: " + to_string(instruction.opcode));
			break;
		}
#endif
		}
		return newInstructionIdx;
	}
	void ExecuteFunction(vector<AST::Function>& functionList, lli functionIndex) {
		RunTimeEnvData rted;
		rted.functionListPtr = &functionList;
		rted.currentFunction = functionIndex;
		scopeVarList.emplace_back(vector<Variable>());
		loop(inst_idx, 0, functionList[functionIndex].instructionList.size()) {
			rted.currentInstruction = inst_idx;
			//IDE_Handler::handler.EnvDebugger(&rted);
			if (Settings::debug) {
				printn(inst_idx, ", ");
				if (breakPointList[functionIndex][inst_idx] == true) {
					print("Hit break Point.");
					stack.StackDump();
					Debug::DumpVaraibles();
					Debug::PrintRegisters();
					print("Instruction Name -> ", functionList[functionIndex].instructionList[inst_idx].PrintInstruction(true));
					print("Instruction Index -> ", inst_idx);
					print("Enter to continue");
					std::cin.get();
				}
			}		
			inst_idx = ExecuteInstruction(inst_idx, functionList[functionIndex].instructionList, functionList, &rted);
			if (Settings::debug) {
				Debug::PrintRegisters();
				print(Debug::DumpVaraibles());
				print("\n");
			}
		}
		//stack.StackDump();
	}
	void Execute(vector<AST::Function> functionList) {
		executeFunctionPtr = &ExecuteFunction;
		breakPointList = IDE_Handler::handler.ByteCodeSetBreakPoints(&functionList);
		functionList = MapLabels(functionList);
		//std::cin.get();
		lli mainFunctionIndex = FindFunctionIndex("main",functionList);
		CopyStringsToStack();
		auto start = std::chrono::steady_clock::now();
		ExecuteFunction(functionList, mainFunctionIndex);
		print("------Console dump--------");
		IO::console.PrintConsole();
		if (Settings::time) {
			auto end = std::chrono::steady_clock::now();
			cout << "Elapsed time in seconds: "
				<< (double)std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() * 0.000001
				<< "\n";
		}
	}
}