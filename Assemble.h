#pragma once
namespace Assemble {
	vector<AST::Function> functionList;

	namespace Settings {
		bool os = false;
		bool is_nasm = true;
		bool optimize = false;
	}

	namespace Register {
		enum {
			EAX, EBX
		};
	}

	lli currentStackUsed = 0;
	vector<lli> stackUsedSize;
	struct Variable {
		string name;
		lli dataSize_total;
		lli stackPtr = -1;
		lli stackFrame = -1;//scope level the variable is located at

		Variable(string name, lli dataSize_total) {
			self.name = name;
			self.dataSize_total = dataSize_total;
		}

		//returns varaible as x86 asm
		string AsString() {
#if safe == true
			if (stackPtr == -1) {
				err("stackPtr has not been initilized");
			}
			if (stackFrame == -1) {
				err("stackFrame has not been initilized");
			}
#endif
			string dataType;
			switch (dataSize_total) {
				case 4: dataType = "dword"; break;
				case 2: dataType = "word"; break;
				case 1: dataType = "byte"; break;
#if safe == true
				default: err("Did not match any casses");
#endif
			}
			if (Settings::is_nasm == false) {
				dataType += " ptr ";
			}
			lli currentStackFrame = stackUsedSize.size() - 1;
			lli rewindCount = currentStackFrame - stackFrame;
			lli stackPos = 0;
			loop(i, 0, rewindCount) {
				stackPos += 4;//acount for ebp
				stackPos += stackUsedSize[stackUsedSize.size() - 2 - i];				
			}
			stackPos -= stackPtr;
			//lli stackPos = stackPtr + (stackUsedSize.size() - stackFrame) * 4;//32 bit ebp
			return dataType + " [ ebp + " + to_string(stackPos) + "]";
		}
	};
	vector<vector<Variable>> variableList;

	struct FindVariable_Return {
		lli scopeIndex;
		lli inScopeIndex;
	};
	//returns -1 if failed
	FindVariable_Return FindVaraiable(string variableName) {
		FindVariable_Return ret;
		loop(s, 0, variableList.size()) {
			loop(i, 0, variableList[s].size()) {
				if (variableList[s][i].name == variableName) {
					ret.scopeIndex = s;
					ret.inScopeIndex = i;
					return ret;
				}
			}
		}
		//failed to find varaible
		err("Failed to find variable");
		ret.scopeIndex = -1;
		ret.inScopeIndex = -1;
		return ret;
	}

	namespace Register {

		struct RegStored {
			enum StoreType {
				NONE,
				CONST,
				VARIABLE
			};
			lli storeType;
			lli instructionIndex;

			ulli constant;
			lli variableScopeIndex;
			lli variableInScopeIndex;

			RegStored() {
				storeType = NONE;
			}

			void set(Iasm::Instruction& inst, lli argIndex, lli instructionIndex) {
#if safe == true
				if (inst.oprand_type.size() <= argIndex) {
					err("Arg Index out of range");
				}
#endif
				storeType = NONE;
				switch (inst.oprand_type[argIndex]) {
				case Iasm::Oprand::CONST:
					storeType = CONST;
					constant = inst.oprand_int[argIndex];
					break;
				case Iasm::Oprand::NAME:
					storeType = VARIABLE;
					{
						FindVariable_Return ret = FindVaraiable(inst.oprand_string[argIndex]);
						variableScopeIndex = ret.scopeIndex;
						variableInScopeIndex = ret.inScopeIndex;
					}
					break;
#if safe == true
				default: err("I dont know");
#endif
				}
				self.instructionIndex = instructionIndex;
			}
		};
		RegStored eaxStore;
		RegStored ebxStore;
	}

	//resets label names in functions so no two matching labels
	void SetFunctionLabels() {
		lli labelCount = 0;
		fin(f, functionList) {
			fin(i,f.instructionList){
				if (i.opcode == Iasm::Opcode::LABEL) {
					lli oldName = i.oprand_int[0];
					i.oprand_int[0] = labelCount;
					//finds all instance of label and changes them
					fin(i2, f.instructionList) {
						loop(a, 0, i2.oprand_type.size()) {
							if (i2.oprand_type[a] == Iasm::Oprand::LABEL_INDEX) {
								if (i2.oprand_int[a] == oldName) {
									i2.oprand_int[a] = labelCount;
								}
							}
						}
					}					
					labelCount++;
				}
			}
			//add entry label to call the function
			f.label_index = labelCount;
			f.instructionList.insert(f.instructionList.begin(), Iasm::Instruction(Iasm::Opcode::LABEL));
			f.instructionList[0].AppendOprand(labelCount, Iasm::Oprand::LABEL_INDEX);
			labelCount++;
		}
	}
	// dont know what this does
	/*
	void MapLabelsToAddress() {
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
					na.labelAddress = na.labelName;//labelAddress;
					//TODO could have two labels with the same index resulting in error
					labelList.push_back(na);
					//maintian labels for asm conversion
					//functionList[f].instructionList.erase(functionList[f].instructionList.begin() + i);
					//i--;
				}
				else {
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
	}
	*/
	vector<Iasm::Instruction> ConcaternateInstructions() {
		vector<Iasm::Instruction> instructionList;
		fin(f, functionList) {
			instructionList = vecConcaternate(instructionList, f.instructionList);
		}
		return instructionList;
	}
	lli GetOprandSize(Iasm::Instruction inst, lli argIndex) {
		lli type = inst.oprand_type[argIndex];
		lli i = inst.oprand_int[argIndex];
		string s = inst.oprand_string[argIndex];
		switch (type) {
		case Iasm::Oprand::CONST:
			return 4; // all const are 32 bit constants
			break;
		case Iasm::Oprand::REG: return 4;
		case Iasm::Oprand::NAME: 
			FindVariable_Return ret = FindVaraiable(s);
			return variableList[ret.scopeIndex][ret.inScopeIndex].dataSize_total;
			break;
		}
		err("Invalid Oprand passed to GetOprandSize oprand=" + to_string(type));
		return sbitMax64;
	}
	string inline GetRegisterNameFromArg(Iasm::Instruction inst, lli argIndex, const lli registerClass) {
		lli oprandSize = GetOprandSize(inst, argIndex);
		switch (registerClass) {
			case Register::EAX: {
				switch (oprandSize) {
					case 4: {
						return "eax";
						break;
					}
					case 2: {
						return "ax";
						break;
					}
					case 1: {
						return "al";
						break;
					}
#if safe == true
					default: err("Failed to find correct size"); break;
#endif
				}
				break;
			}
			case Register::EBX: {
				switch (oprandSize) {
				case 4: {
					return "ebx";
					break;
				}
				case 2: {
					return "bx";
					break;
				}
				case 1: {
					return "bl";
					break;
				}
#if safe == true
				default: err("Failed to find correct size"); break;
#endif
				}
				break;
			}
#if safe == true
			default: err("Not valid register id"); break;
#endif
		}
	}
	string ReturnOprand(Iasm::Instruction inst, lli argIndex) {
		lli type = inst.oprand_type[argIndex];
		lli i = inst.oprand_int[argIndex];
		string s = inst.oprand_string[argIndex];
		string out;
		switch (type) {
		case Iasm::Oprand::CONST:
			return to_string(i);
			break;
		case Iasm::Oprand::REG:
			if (i == Iasm::REG1) {
				return "eax";
			}
			else {
#if safe == true
				if (i != Iasm::REG2) {
					err("Does not equal any registers");
				}
#endif
				return "ebx";
			}
			break;
		case Iasm::Oprand::NAME: {
			FindVariable_Return ret = FindVaraiable(s);
#if safe == true
			if (ret.inScopeIndex == -1 or ret.scopeIndex == -1) {
				err("Failed to find variable with name: " + s);
			}
#endif
			return variableList[ret.scopeIndex][ret.inScopeIndex].AsString();
			break;
		}			
#if safe == true
		default:
			err("Did not match any cases in switch case");
#endif
		}
		return "";
	}
	string CompileByteCode(vector<Iasm::Instruction>& instructionList) {
		//if masm return OFFSET label if nasm return label
		auto GetLabelOffset = [](string s) -> string {
			if (Settings::is_nasm) {
				return s;
			}
			else {
				return "OFFSET " + s;
			}
		};
		string printfName = "_printf";
		if (!Settings::is_nasm) {
			printfName = "printf";
		}
		string out;
		vector<string> instList;
		variableList.clear();
		variableList.emplace_back(vector<Variable>());
		stackUsedSize.clear();
		//stackUsedSize.push_back(0);//global scope
		string tabs;
		fin(i, instructionList) {
			using namespace Iasm::Opcode;
			string instruction_string;
			//comment each instruction with byte code
			instruction_string += tabs + ";" + i.PrintInstruction(true) + "\n";
			switch (i.opcode) {
			case CREATE_VARIABLE: {
				string variableName = i.oprand_string[1];
				lli dataSize_total = typeSizeList[i.oprand_int[0]];
				variableList.back().push_back(Variable(variableName, dataSize_total));
				variableList.back().back().stackPtr = currentStackUsed - stackUsedSize.back() + dataSize_total;
				variableList.back().back().stackFrame = stackUsedSize.size() - 1;
				currentStackUsed += dataSize_total;
				instruction_string += tabs + "sub esp, " + to_string(dataSize_total) + "\n";
				break;
			}
			case OPENSCOPE: {
				tabs += '\t';
				instruction_string += ""
					+ tabs + "push ebp\n"
					+ tabs + "mov ebp, esp\n";				
				if(stackUsedSize.size() > 0)stackUsedSize.back() = currentStackUsed;
				stackUsedSize.push_back(0);
				currentStackUsed = 0;
				break;
			}
			case CLOSESCOPE: {
				instruction_string += ""
					"" + tabs + "mov esp, ebp\n"
					"" + tabs + "pop ebp\n"
					// "" + tabs + "add esp, 4\n"
					"";				
				tabs.pop_back();
				stackUsedSize.pop_back();
				if (stackUsedSize.size() > 0)currentStackUsed = stackUsedSize.back();
				break;
			}
			case PRINT_STRING: {
				instruction_string += ""
					"" + tabs + "mov ebx, " + ReturnOprand(i, 0) + "\n"
					"" + tabs + "add ebx, " + GetLabelOffset("DATA_LABEL") + "\n";
				if (Settings::os == true) {
					instruction_string += tabs + "call PrintString\n";
				}
				else {
					//ebx is current ptr to string
					instruction_string += ""
						+ tabs + "push ebx\n"//char*
						+ tabs + "call " + printfName + "\n"
						+ tabs + "add esp, 4\n"; // restor the stack
					//print newline
					/*
					instruction_string += ""
						//+ tabs + "push 10\n"//pushes 16 bit
						+ tabs + "mov dl, 0\n"
						+ tabs + "mov dh, 10\n"						
						+ tabs + "push dx\n"
						+ tabs + "mov edi, esp\n"
						+ tabs + "add edi, 2\n"
						+ tabs + "push edi\n"
						+ tabs + "call _printf\n"
						+ tabs + "add esp, 6\n";
					*/
				}
				break;
			}
			case PRINT_NUMBER: {
				if (Settings::os == true) {
					instruction_string += ""
						"" + tabs + "xor ebx, ebx\n"
						"" + tabs + "mov " + GetRegisterNameFromArg(i, 0, Register::EBX) + ", " + ReturnOprand(i, 0) + "\n"
						"" + tabs + "call PrintNumber\n";
				}
				else {
					instruction_string += ""
						+ tabs + "push eax\n"
						+ tabs + "push " + GetLabelOffset("PRINT_NUMBER") + "\n"
						+ tabs + "call " + printfName + "\n"
						+ tabs + "add esp, 8\n";
				}
				break;
			}
			case RET: {
				instruction_string += "ret\n";
				break;
			}
			case READ: {
				instruction_string += tabs + "mov "
					+ ReturnOprand(i, 0) 
					+ ", " + GetRegisterNameFromArg(i, 0, Register::EAX) + "\n";
				break;
			}
			case MOV: {
				string reg = GetRegisterNameFromArg(i, 0, Register::EAX);
				string movInst = "mov";
				if (reg != "eax")movInst = "movzx";
				instruction_string += 
					tabs + movInst + " "
					+ reg
					+ ", " + ReturnOprand(i, 0) + "\n";
				Register::eaxStore.set(i, 0, instList.size());
				break;
			}
			case MOV2: {
				string reg = GetRegisterNameFromArg(i, 0, Register::EBX);
				string movInst = "mov";
				if (reg != "ebx")movInst = "movzx";
				instruction_string += 
					tabs + movInst + " "
					+ GetRegisterNameFromArg(i, 0, Register::EBX)
					+ ", " + ReturnOprand(i, 0) + "\n";
				Register::ebxStore.set(i, 0, instList.size());
				break;
			}
			case ADD: {
				string oprand = ReturnOprand(i, 0); 
				if (oprand == "ebx" and Register::ebxStore.storeType == Register::RegStored::CONST) {					
						instList[Register::ebxStore.instructionIndex] = string(""); 
						string eax = GetRegisterNameFromArg(i, 0, Register::EAX);
						//if (Register::eaxStore.storeType == Register::RegStored::VARIABLE) {
						//	eax = variableList[Register::eaxStore.variableScopeIndex][Register::eaxStore.variableInScopeIndex].AsString();
						//	instList[Register::eaxStore.instructionIndex] = string("");
						//}
						if (Register::ebxStore.constant == 1) {							
							instruction_string += tabs + "inc" + " "
								+ eax + "\n"; 
						}
						else {							
							instruction_string += tabs + "add" + " "
								+ eax + ", " 
								+ to_string(Register::ebxStore.constant) + "\n"; 
						}
				}
				else {					
					instruction_string += tabs + "add" + " "
						+ GetRegisterNameFromArg(i, 0, Register::EAX) + ", " 
						+ ReturnOprand(i, 0) + "\n"; 
				}
				break; 
			}
			case SUB: {
				string oprand = ReturnOprand(i, 0);
				if (oprand == "ebx" and Register::ebxStore.storeType == Register::RegStored::CONST) {
					instList[Register::ebxStore.instructionIndex] = string("");
					string eax = GetRegisterNameFromArg(i, 0, Register::EAX);
					//if (Register::eaxStore.storeType == Register::RegStored::VARIABLE) {
					//	eax = variableList[Register::eaxStore.variableScopeIndex][Register::eaxStore.variableInScopeIndex].AsString();
					//	instList[Register::eaxStore.instructionIndex] = string("");
					//}
					if (Register::ebxStore.constant == 1) {
						instruction_string += tabs + "dec" + " "
							+ eax + "\n";
					}
					else {
						instruction_string += tabs + "sub" + " "
							+ eax + ", "
							+ to_string(Register::ebxStore.constant) + "\n";
					}
				}
				else {
					instruction_string += tabs + "sub" + " "
						+ GetRegisterNameFromArg(i, 0, Register::EAX) + ", "
						+ ReturnOprand(i, 0) + "\n";
				}
				break;
			}
			//WARNING DOES NOT XOR EBX, EBX
			case MUL: {
				string oprand = ReturnOprand(i, 0);
				if (oprand == "ebx") {
					instruction_string += tabs + "xor edx, edx\n"
						+ tabs + "mul ebx\n";
				}
				else {
					instruction_string += tabs + "xor edx, edx\n"
						+ tabs + "push ebx\n"
						+ tabs + "mov " + GetRegisterNameFromArg(i, 0, Register::EBX) + ", " + oprand + "\n"
						+ tabs + "mul ebx\n"
						+ tabs + "pop ebx\n";
				}
				break;
			}
			case DIV: {
				string oprand = ReturnOprand(i, 0);
				if (oprand == "ebx") {
					instruction_string += tabs + "xor edx, edx\n"
						+ tabs + "div ebx\n";
				}
				else {
					instruction_string += tabs + "xor edx, edx\n"
						+ tabs + "push ebx\n"
						+ tabs + "mov " + GetRegisterNameFromArg(i, 0, Register::EBX) + ", " + oprand + "\n"
						+ tabs + "div ebx\n"
						+ tabs + "pop ebx\n";
				}
				break;
			}
			case MOD: {
				string oprand = ReturnOprand(i, 0);
				if (oprand == "ebx" and Register::ebxStore.storeType == Register::RegStored::CONST
					and Register::ebxStore.constant == 2) {
					instList[Register::ebxStore.instructionIndex] = string("");
					string eax = GetRegisterNameFromArg(i, 0, Register::EAX);					
					instruction_string += ""
						+ tabs + "and " + eax + ", 1\n";					
				}
				else {
					if (oprand == "ebx") {
						//may overwrite ebx carefull with future optimizations because of this
						instruction_string += tabs + "xor edx, edx\n"
							+ tabs + "div ebx\n"
							+ tabs + "mov eax, edx\n";
					}
					else {
						instruction_string += tabs + "xor edx, edx\n"
							+ tabs + "push ebx\n"
							+ tabs + "mov " + GetRegisterNameFromArg(i, 0, Register::EBX) + ", " + oprand + "\n"
							+ tabs + "div ebx\n"
							+ tabs + "mov eax, edx\n"
							+ tabs + "pop ebx\n";
					}
				}
				break;
			}
#define c(name, inst)\
			case name: {\
				instruction_string += tabs + "cmp eax, " + ReturnOprand(i,0) + "\n"\
					+ tabs + #inst" al\n"\
					+ tabs + "movzx eax, al\n";\
				break;\
			}pass
			c(SETE, sete);
			c(SETNE, setne);
			c(SETL, setl);
			c(SETG, setg);
#undef c
			case LABEL: {
				instruction_string += tabs + ".LABEL_" + to_string(i.oprand_int[0]) + ":\n";
				break;
			}
			case JMP: {
				instruction_string += tabs + "jmp .LABEL_" + to_string(i.oprand_int[0]) + "\n";
				break;
			}
			case CJMP: {
				if (i.oprand_int[0] == Iasm::REG1) {
					instruction_string += tabs + "cmp eax, 0\n"
						+ tabs + "je LABEL_" + to_string(i.oprand_int[1]) + "\n";
				}
				else {
#if safe == true
					if (i.oprand_int[0] != Iasm::REG2) {
						err("Register does not match any registers");
					}
#endif
					instruction_string += tabs + "cmp ebx, 0\n"
						+ tabs + "je LABEL_" + to_string(i.oprand_int[1]) + "\n";
				}
				break;
			}
			case CALL: {
				lli functionIndex = indexVec_lambda<string, AST::Function>(i.oprand_string[0], functionList,
					[](string& s, AST::Function& func)->bool {
						return s == func.name;
					});
#if safe == true
				//call names should be checked before being assembled
				if (functionIndex == -1) {
					err("Failed to find function: " + i.oprand_string[0]);
				}
#endif
				//instruction_string += tabs + "call LABEL_" + to_string(functionList[functionIndex].label_index) + "\n";
				instruction_string += tabs + "call " + functionList[functionIndex].name + "\n";
				break;
			}
#if safe == true
			default:
				//err("Failed to compile Instruciton");
				print("Failed to compile Instruction: " + i.PrintInstruction());
				break;
#endif
			}
			print(instruction_string);
			instList.push_back(instruction_string);
		}
		fin(h, instList) {
			out += h;
		}
		return out;
	}
	string WriteStringToMemory() {
		string out;
		if (Settings::is_nasm)out = "DATA_LABEL:\n";
		else out = "DATA_LABEL ";
		bool first = true;//need to add comma after string
		fin(s, Lexer::charArrayList) {
			string t = s.content;
			if (first or Settings::is_nasm)out += "db ";
			loop(c, 0, t.size()) {
				if (t[c] == '\n') {
					out += "10";
				}
				else {
					out += string("'") + t[c] + string("'");
				}
				if (c != t.size() - 1) {
					out += ", ";
				}
			}
			out += ", 0,\n";
			first = false;
		}
		if (Lexer::charArrayList.size() > 0) {
			out.pop_back();
			out.pop_back();
		}
		else {
			out = "";
		}
		out += "\n\n\n\n";
		return out;
	}
	string AssembleCode(vector<AST::Function>& functionList_arg) {
		functionList = functionList_arg;//copy for public use and for data changes
		//SetFunctionLabels();
		//MapLabelsToAddress();
		//SetFunctionLabels();
		
		//vector<Iasm::Instruction> instructionList = ConcaternateInstructions();
		//Iasm::PrintInstructionList(instructionList);

		string code;
		if (Settings::os == true) {
			code += "[org 0x7e00]\n[BITS 16]\n";
			code += "call START_LABEL\n";
			code += "call EndTimer\n";
			code += "jmp $\n";
			code += "%include \"stdLib.asm\"\n";
			code += WriteStringToMemory();
			code += "START_LABEL:\n";
			code += "call StartTimer\n";
		}
		if (Settings::os == false) {//windows
			if (Settings::is_nasm) {
				code += "[bits 32]\n";
				code += "global _main\n";
				code += "extern  _printf\n";
				code += "section .text\n";
				code += "PRINTF_NUMBER: db \"%i\", 10, 0\n";
				code += WriteStringToMemory();
				code += "_main:\n";
			}
			else {	
				//code += ".486\n";
				code += ".model flat, C\n";
				code += "includelib legacy_stdio_definitions.lib\n";
				code += "EXTERN printf :PROC ; declare printf\n";
				code += ".data\n";
				code += "PRINT_NUMBER db \"%i\", 10, 0\n";
				code += WriteStringToMemory();
				code += ".code\n";
				//code += "includelib legacy_stdio_definitions.lib\n";
				//code += "EXTERN printf : PROC\n";11
			}
		}

		loop(f, 0, functionList.size()) {
			if (Settings::is_nasm) {
				code += functionList[f].name + ":\n";
			}
			else {
				code += functionList[f].name + " proc\n";
			}			
			code += CompileByteCode(functionList[f].instructionList);
			if (Settings::is_nasm == false) {
				code += functionList[f].name + " endp\n";
			}
		}
		//code += CompileByteCode(instructionList);

		print("\n\n\n\n\n\n\n\n\n\n\n");
		print("----------OUTPUT----------");
		print(code);
		
		if (Settings::os == true) {
			code += "times 512*4-($-$$) db 0\n";
		}
		if (Settings::is_nasm == false) {
			code += "end\n";
		}
		return code;
	}
}