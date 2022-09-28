#pragma once
#include "Lexer.h"
#include "IntermediateCode.h"
//namespace syntax anylisis
namespace AST {//was SA
	//arglist

	//return is arglist followed by the size of the argList
	std::tuple<bool,lli> CheckIfParameterList(vector<Lexer::Token> tokenList, lli idx) {
		if (tokenList[idx].id == Lexer::TokenId::OPENBRACKET) {
			//look			
			enum {
				TYPE_LOOK,
				NAME_LOOK,
				COMMA_LOOK
			};
			lli lookType = TYPE_LOOK;
			//
			lli count = 0;
			while (1 == 1) {
				//error checking
				if (idx + count >= tokenList.size()) {
					err("arglist not closed befor end of tokenList");
				}
				Lexer::Token& currentToken = tokenList[idx + count];

				if (currentToken.id == Lexer::TokenId::CLOSEBRACKET) {
					if (lookType == NAME_LOOK) { 
						return std::tuple<bool, lli>(false, null);
					}
					return std::tuple<bool, lli>(true, count+1);
				}
				if (currentToken.id == Lexer::TokenId::TYPE) {
					if (lookType != TYPE_LOOK) {
						return std::tuple<bool, lli>(false, null);						
					}
					lookType = NAME_LOOK;
				}
				if (currentToken.id == Lexer::TokenId::NAME) {
					if (lookType != NAME_LOOK) {
						return std::tuple<bool, lli>(false, null);
					}
					lookType = COMMA_LOOK;
				}
				if (currentToken.id == Lexer::TokenId::COMMA) {
					if (lookType != COMMA_LOOK) {
						return std::tuple<bool, lli>(false, null);
					}
					lookType = TYPE_LOOK;
				}
				count++;
			}
		}
		return std::tuple<bool, lli>(false, null);
	}
	struct ParameterList {
		vector<lli> typeIdList;
		vector<string> variableNames;
		//TODO preset values
		string PrintParameterList(const bool ret = false) {
			string out;
			out += "(";

			loop(i, 0, typeIdList.size()) {
				out += typeNameList[typeIdList[i]] + ":";
				out += variableNames[i];
				out += ", ";
			}
			if (typeIdList.size() > 0) {
				out.pop_back(); out.pop_back();
			}

			out += ")";
			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
	};
	ParameterList ReadInParameterList(vector<Lexer::Token> tokenList, lli idx) {
		ParameterList argList;
		if (tokenList[idx].id != Lexer::TokenId::OPENBRACKET) {
			err("Invalid arg list");
		}
		idx++;
		enum {
			LOOK_TYPE,
			LOOK_NAME,
			LOOK_COMMA
		};
		lli lookType = LOOK_TYPE;
		while (1 == 1) {
#if safe == true
			if (idx >= tokenList.size()) {
				err("idx overrun tokenList");
			}
#endif
			if (tokenList[idx].id == Lexer::TokenId::CLOSEBRACKET) {
				break;
			}

			if (lookType == LOOK_TYPE) {
				if (tokenList[idx].id != Lexer::TokenId::TYPE) {
					err("Invalid thing");
				}
				argList.typeIdList.push_back(tokenList[idx].type);
			}
			if (lookType == LOOK_NAME) {
				if (tokenList[idx].id != Lexer::TokenId::NAME) {
					err("Invalid argList thing");
				}
				argList.variableNames.push_back(tokenList[idx].variableName);
			}
			if (lookType == LOOK_COMMA) {
				if (tokenList[idx].id != Lexer::TokenId::COMMA) {
					IDE_Handler::Exception e("Expected a comma", __LINE__, __FILE__,
						Lexer::ReturnTokenLineForException(tokenList, idx), null, idx);
					IDE_Handler::handler.CreateException(e);
					err("Expected a comma");
				}
			}

			if (lookType == LOOK_TYPE) { lookType = LOOK_NAME; }
			else if (lookType == LOOK_NAME) { lookType = LOOK_COMMA; }
			else if (lookType == LOOK_COMMA) { lookType = LOOK_TYPE; }
			idx++;
		}
		if (argList.typeIdList.size() != argList.variableNames.size()) {
			err("unequal sizes");
		}
		return argList;
	}
	//Functions
	struct Function {
		string name;
		vector<lli> returnTypeList;
		ParameterList argList;

		vector<Lexer::Token> tokenList;
		vector<Iasm::Instruction> instructionList;

		//the label to call when entering the function
		//only for compiling to asm
		lli label_index = sbitMax64;

		string PrintInstructionList(const bool ret = false) {
			string out = "";
			out += Iasm::PrintInstructionList(instructionList, true);
			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
		string PrintFunction(const bool ret = false) {
			string out;
			fin(i, returnTypeList) {
				out += typeNameList[i] + ", ";
			}
			out.pop_back(); out.pop_back();
			out += " " + name + " ";
			out += argList.PrintParameterList(true);
			out += "{";
			fin(i, tokenList) {
				out += i.printToken(true);
				out += " ";
				if (i.is_newLine) {
					out += "\n";
				}
			}
			out += "}";

			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
	};
	vector<Function> functionList;

	struct Exp {
		vector<Lexer::Token> tokenList;
		vector<Iasm::Instruction> storedInstList;//dont need
		lli writeRegister;//dont need
		vector<Exp> expList;
		lli resultantType = 1;//set to int

		lli ReadInExp(vector<Lexer::Token>& tokenList, lli idx,
			vector<lli> exitTokenList = { Lexer::TokenId::SEMI_COLON }) {
#if safe == true
			if (self.tokenList.size() != 0) {
				err("Weird behavour");
			}
#endif
			lli sizeCount = 0;
			while (1 == 1) {
				if (idx >= tokenList.size()) {
					err("idx bigger than tokenList");
				}
				//if (tokenList[idx].id == Lexer::TokenId::SEMI_COLON) {
				if(in<lli>(tokenList[idx].id, exitTokenList)) {
					break;
				}
				self.tokenList.push_back(tokenList[idx]);
				idx++;
				sizeCount++;
			}
			return sizeCount;
		}
		//move argument into register
		void moveIntoRegister(vector<Exp>& expList, vector<Iasm::Instruction>& instList, Lexer::Token& token, int regId) {
#if safe == true
			if (regId != Iasm::REG1 and regId != Iasm::REG2) {
				err("Invalid regId");
			}
#endif
			lli movType = Iasm::Opcode::MOV;
			if (regId == Iasm::REG2) { movType = Iasm::Opcode::MOV2; }
			switch (token.id) {
			case Lexer::TokenId::EXP:

				//vecConcaternate(instList, expList[token.expIndex].Compile(false));
			{
				vector<Iasm::Instruction> expInstList;
				if (expList[token.expIndex].tokenList.size() == 3) {
					expInstList = expList[token.expIndex].SimpleCompile(expList, regId == Iasm::REG2);
				}
				else {
					expInstList = expList[token.expIndex].Compile(regId == Iasm::REG2);//save state if target is register 2
				}
				instList.insert(instList.end(), expInstList.begin(), expInstList.end());
			}

			break;
			case Lexer::TokenId::CONST: {
				instList.push_back(Iasm::Instruction(movType));
				instList.back().AppendOprand(token.constant, Iasm::Oprand::CONST);
				break;
			}			
			case Lexer::TokenId::NAME:
				instList.push_back(Iasm::Instruction(movType));
				instList.back().AppendOprand(token.variableName, Iasm::Oprand::NAME);
				break;
			case Lexer::TokenId::FUNCTION_CALL: {
				//call function
				string functionName = token.variableName;
				lli index = indexVec_lambda<string, Function>(functionName, functionList,
					[](string& a, Function& b) -> bool {
						return a == b.name;
				});
//#if safe == true
				if (index == -1) {
					err("Failed to find function");
				}
//#endif
				Function& func = functionList[index];
				//allocate return memory
				loop(i, 0, func.returnTypeList.size()) {
					instList.push_back(Iasm::Instruction(Iasm::Opcode::CREATE_TEMP_VAR));
					instList.back().AppendOprand(func.returnTypeList[i], Iasm::Oprand::TYPE);
					instList.back().AppendOprand(i, Iasm::Oprand::CONST);
				}
				//calls the function
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CALL));
				instList.back().AppendOprand(func.name, Iasm::Oprand::NAME);
				//move return into return register
				//ALWAYS TAKE FIRST RETURN VALUE FROM FUNCTION
				instList.push_back(Iasm::Instruction(movType));				
				instList.back().AppendOprand(0,Iasm::Oprand::TEMP_VAR_INDEX);
				//delete allocated memory
				loop(i, 0, func.returnTypeList.size()) {
					instList.push_back(Iasm::Instruction(Iasm::Opcode::DELETE_TEMP_VAR));
					instList.back().AppendOprand(func.returnTypeList[i], Iasm::Oprand::TYPE);
				}
				break;
			}
			default:
				err("Invalid token id");
			}
		}
		//check if single 10 or func()
		bool isSingle() {
			if (tokenList.size() == 1) {
				return true;
			}
			if (tokenList[0].id == Lexer::TokenId::FUNCTION_CALL) {
				lli count = 0;
				lli bracketCount = 0;
				loop(i, 0, tokenList.size()){
					count++;
					if (tokenList[i].id == Lexer::TokenId::CLOSEBRACKET) {
						bracketCount--;
						if (bracketCount == -1) {
							break;
						}
					}
					if (tokenList[i].id == Lexer::TokenId::OPENBRACKET) {
						bracketCount++;
					}
				}
				if (tokenList.size() == count) {
					return true;
				}
				return false;
			}
			return false;
		}
		//compile (10) expressions
		vector<Iasm::Instruction> SingleCompile(vector<Exp>& expList, const bool saveState) {
			vector<Iasm::Instruction> instList;
			if (saveState) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CREATE_EXP));
				//instList.back().AppendOprand(expList.size() - 1, Iasm::Oprand::CONST);
			}
			moveIntoRegister(expList, instList, tokenList[0], false);//dont save state
			if (saveState) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::POP_EXP));
			}
			return instList;
		}
		//compile 10 + 10 expression
		vector<Iasm::Instruction> SimpleCompile(vector<Exp>& expList, const bool saveState) {
			vector<Iasm::Instruction> instList;
			if (saveState) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CREATE_EXP));
			}
#if safe == true
			if (tokenList.size() != 3) {
				err("Cannot compile simple expresiont to Iasm");
			}
#endif
			moveIntoRegister(expList, instList, tokenList[0], Iasm::REG1);
			moveIntoRegister(expList, instList, tokenList[2], Iasm::REG2);
			lli operation = Operators::bidmasList[tokenList[1].operatorBidmasLevel][tokenList[1].operatorIndex].opcode;
			//instList.push_back(Iasm::Instruction(operation, { Iasm::REG1,Iasm::REG2 }, { Iasm::Oprand::REG,Iasm::Oprand::REG }));
			instList.push_back(Iasm::Instruction(operation));
			//instList.back().AppendOprand(Iasm::REG1, Iasm::Oprand::REG);
			instList.back().AppendOprand(Iasm::REG2, Iasm::Oprand::REG);

			if (saveState)instList.push_back(Iasm::Instruction(Iasm::Opcode::POP_EXP));

			fin(i, instList) {
				i.PrintInstruction();
			}
			return instList;
		}
		vector<Iasm::Instruction> Compile(const bool saveState = false) {
			vector<Iasm::Instruction> instList;
			if (saveState == true) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CREATE_EXP));
			}
			//BIDMAS
#if safe == true
			if (expList.size() != 0) {
				err("Invalid expLIST SIZE");
			}
#endif
			//brackets
			print("------NEW EXP------");
			Lexer::PrintTokenList(true, false, &tokenList);
			loop(t, 0, tokenList.size()) {
				if (tokenList[t].id == Lexer::TokenId::OPENBRACKET) {
					//read till next close bracket
					vector<Lexer::Token> newExpTokenList;
					lli newTokenListSize = 1;//skip over OPENBRACKET
					lli bracketCount = 0;
					while (1 == 1) {
						if (tokenList[t + newTokenListSize].id == Lexer::TokenId::CLOSEBRACKET) {
							bracketCount--;
							if (bracketCount == -1) {
								//newExpTokenList.push_back(tokenList[t + newTokenListSize]);
								newTokenListSize++;
								break;
							}
						}
						if (tokenList[t + newTokenListSize].id == Lexer::TokenId::OPENBRACKET) {
							bracketCount++;
						}
						newExpTokenList.push_back(tokenList[t + newTokenListSize]);
						newTokenListSize++;
					}
					expList.push_back(Exp());
					expList.back().tokenList = newExpTokenList;
					//expList.back().Compile(true); doesnt need to compile here
					loop(c, t, t + newTokenListSize) {
						tokenList.erase(tokenList.begin() + t);
					}
					vecInsert(tokenList, t, Lexer::Token(Lexer::TokenId::EXP, expList.size() - 1));
					//gives line number to expresions
#if safe == true
					if (0 > t - 1) {
						err("Error index by less than zero");
					}
#endif
					tokenList[t].lineNumber = tokenList[t - 1].lineNumber;
					Lexer::PrintTokenList(true, false, &tokenList);
				}
			}
			//check for single
			if (isSingle()) {
				print("-----Single Compile-------");
				instList = vecConcaternate(instList, SingleCompile(expList, false));
				//Iasm::PrintInstructionList(instList);
			}
			else {
				print("----BIDMAS---- not a single");
				//BIDMAS on operators
				loop(bidmas_layer_index, 0, Operators::bidmasList.size()) {
					vector<Operators::Operator>& bidmas_layer = Operators::bidmasList[bidmas_layer_index];
					loop(op_index, 0, bidmas_layer.size()) {
						Operators::Operator& op = bidmas_layer[op_index];

						bool is_lookingForOp = false;
						loop(token_index, 0, tokenList.size()) {
							Lexer::Token& token = tokenList[token_index];
							if (is_lookingForOp and token.id != Lexer::TokenId::OPP) {
								print("Token Index: ", token_index);
								Lexer::PrintTokenList(true, false, &tokenList);
								//error
								IDE_Handler::Exception e("Expecting opp did not get one, token:"
									+ token.printToken(true), __LINE__, __FILE__,
									Lexer::ReturnTokenLineForException(tokenList, token_index)
									, token_index, tokenList[token_index].lineNumber);
								IDE_Handler::handler.CreateException(e);
							}
							if (is_lookingForOp == false and token.id == Lexer::TokenId::OPP) {
								err("Not expecting opp got one");
							}
							if (token.id == Lexer::TokenId::OPP) {
								if (token.operatorBidmasLevel == bidmas_layer_index and token.operatorIndex == op_index) {
									Lexer::PrintTokenList(true, false, &tokenList);
									//is op
									expList.push_back(Exp());
									expList.back().tokenList.push_back(tokenList[token_index - 1]);
									expList.back().tokenList.push_back(tokenList[token_index + 0]);
									expList.back().tokenList.push_back(tokenList[token_index + 1]);
									vector<Iasm::Instruction> expInstList = expList.back().SimpleCompile(expList, false);
									instList.insert(instList.end(), expInstList.begin(), expInstList.end());
									//Lexer::PrintTokenList(true, false, &tokenList);
									//remove 3 tokens replace with exp
									loop(i, 0, 3) {
										tokenList.erase(tokenList.begin() + token_index - 1);
									}
									if (tokenList.size() == 0) {
										goto CompiledTokens_exit;
									}
									//Lexer::PrintTokenList(true, false, &tokenList);
									Lexer::Token expToken_toInsert = Lexer::Token(Lexer::TokenId::EXP, expList.size() - 1);
									if (token_index - 1 == tokenList.size()) {
										tokenList.push_back(expToken_toInsert);
									}
									else {
										tokenList.insert(tokenList.begin() + token_index - 1, expToken_toInsert);
									}
									Lexer::PrintTokenList(true, false, &tokenList);
									//is_lookingForOp = false;//gets reverses so means it is still looking for operator
									token_index = -1;
									is_lookingForOp = false;
									continue;
								}
							}
							is_lookingForOp = !is_lookingForOp;
						}
					}
				}
			}
		CompiledTokens_exit:
			print("-----EXP END------");

			if (saveState)instList.push_back(Iasm::Instruction(Iasm::Opcode::POP_EXP));
			return instList;
		}
	};

	struct Variable {
		bool dud = false;

		string name;

		Variable(bool is_dud) {
			dud = is_dud;
		}
		Variable(string name) {
			self.name = name;
		}
	};

	struct ArgumentList {
		vector<Exp> expList;

		//idx ... index of argument to be moved to register
		void MoveRegister(lli idx, lli reg, vector<Iasm::Instruction>& instList) {
			lli movInstruction = Iasm::Opcode::MOV;
			if (reg == Iasm::REG2) {
				movInstruction = Iasm::Opcode::MOV2;
			}
#if safe == true
			if (idx >= expList.size() or idx < 0) {
				err("Invalid idx");
			}
#endif
			Lexer::Token token(Lexer::TokenId::EXP, idx);
			//expList[idx].moveIntoRegister(expList[idx].expList, instList, token, reg);
			vector<Iasm::Instruction> expInstList = expList[idx].Compile(reg == Iasm::REG2);
			instList = vecConcaternate(instList, expInstList);
		}
	};
	ArgumentList ReadInArgumentList(vector<Lexer::Token>& tokenList, lli idx, lli* argumentSize) {
		ArgumentList argList;
		lli bracketCount = 0;
		enum {
			OPENBRACKET,
			EXP,
			COMMA
		};
		lli argSize = 0;
		lli lookType = OPENBRACKET;
		while (1 == 1) {
			if (tokenList[idx].id == Lexer::TokenId::CLOSEBRACKET) {
				argSize++;
				break;
			}
			if (tokenList[idx].id == Lexer::TokenId::OPENBRACKET) {
				if (lookType != OPENBRACKET) {
					err("Failed got OPENBRACKET did not expect it");
				}
				lookType = EXP;
				goto jmp_next;
			}
			if (lookType == EXP) {
				//read till comma and create an exp
				lli newIdx = idx;
				vector<Lexer::Token> expTokenList;
				while (1 == 1) {
#if safe == true
					if (newIdx >= tokenList.size()) {
						err("newIdx is greater than tokenList");
					}
#endif
					if (tokenList[newIdx].id == Lexer::TokenId::CLOSEBRACKET) {
						argSize++;
						break;
					}
					if (tokenList[newIdx].id == Lexer::TokenId::COMMA) {
						argSize++;
						break;
					}
					expTokenList.push_back(tokenList[newIdx]);
					newIdx++;
					argSize++;
				}
				argList.expList.push_back(Exp());
				argList.expList.back().tokenList = expTokenList;
				idx = newIdx;
				if (tokenList[idx].id == Lexer::TokenId::CLOSEBRACKET) {
					goto return_;
				}
			}
			jmp_next:
			argSize++;
			idx++;
		}
		return_:
		$(argumentSize) = argSize;//was argSize + 1
		return argList;
	}


	vector<Lexer::Token> ReadInScope(vector<Lexer::Token> tokenList, lli idx) {
		lli scopeCount = -1;//assumed it is starting on open scope
		vector<Lexer::Token> scopeList;
		loop(t, idx, tokenList.size()) {
			scopeList.push_back(tokenList[t]);
			if (tokenList[t].id == Lexer::TokenId::OPENSCOPE) {
				scopeCount++;
			}
			if (tokenList[t].id == Lexer::TokenId::CLOSESCOPE) {
				scopeCount--;
				if (scopeCount <= -1) {
					goto foundEnd;
				}
#if safe == true
				if (scopeCount < -1) {
					err("Possible caused by reading scope that does not start on an open scope token");
				}
#endif
			}
		}
		Lexer::PrintTokenList(true, false, &tokenList);
		err("failed to Find end of scope before end of tokenList");
	foundEnd:
		return scopeList;
	}
	void FindFunctions(vector<Lexer::Token> tokenList) {
		lli idx = 0;
		auto CheckIdx_TokenListBounds = [&](lli addition=0) {
			if (idx + addition >= tokenList.size()) {
				err("idx out of range");
			}
		};
		while (idx < tokenList.size()) {
			if (tokenList[idx].id == Lexer::TokenId::FN) {
				idx++;
				//get return type list
				vector<lli> returnTypeList;
				while (1 == 1) {//include first argument
					returnTypeList.push_back(tokenList[idx].type);
					idx++;
					CheckIdx_TokenListBounds();
					if (tokenList[idx].id != Lexer::TokenId::COMMA and tokenList[idx].id != Lexer::TokenId::FUNCTION_CALL) {// string [,] string functionName check
						IDE_Handler::Exception e("Expected a comma", __LINE__, __FILE__,
							Lexer::ReturnTokenLineForException(tokenList, idx), idx, tokenList[idx].lineNumber);
						IDE_Handler::handler.CreateException(e);
					}
					if (tokenList[idx].id == Lexer::TokenId::FUNCTION_CALL) {// string, string functionName exit
						break;
					}
					idx++;
					CheckIdx_TokenListBounds();
				}
				//get argument list
				if (tokenList[idx].id == Lexer::TokenId::FUNCTION_CALL) {// idx -> functionName (argList){
					CheckIdx_TokenListBounds(1);
					std::tuple<bool, lli> isParameterList_tuple = CheckIfParameterList(tokenList, idx + 1);
					bool is_argList = std::get<0>(isParameterList_tuple);
					lli argList_size = std::get<1>(isParameterList_tuple);
					if (is_argList == false) {
						err("Failed to find funciton in FN functionName: "+tokenList[idx].variableName);
					}
					functionList.push_back(Function());
					functionList[functionList.size() - 1].name = tokenList[idx].variableName;
					functionList[functionList.size() - 1].argList = ReadInParameterList(tokenList, idx + 1);
					functionList[functionList.size() - 1].returnTypeList = returnTypeList;
					idx += argList_size + 2 - 1;
					functionList[functionList.size() - 1].tokenList = ReadInScope(tokenList, idx);
					idx += functionList[functionList.size() - 1].tokenList.size();
					continue;

#if safe == true
					if (argList_size != null and is_argList == true) { err("I dont know"); }
#endif
				}
			}
			else {
				err("Not function code that is not in a scope token print: "+tokenList[idx].printToken(true));
			}
			idx++;
		}
	}

	//for a function call
	struct FunctionCall {
		string name;
		lli index;
		ArgumentList argList;
		lli tokenCount;
		bool init_bool = false;

		void Init(vector<Lexer::Token>& tokenList, lli idx) {
			if (tokenList[idx].id != Lexer::TokenId::FUNCTION_CALL) {
				IDE_Handler::handler.CreateException(IDE_Handler::Exception(
					"Creating function call struct but token was not function call",
					__LINE__, __FILE__,
					"", // also dont know what to put here
					null, //dont know what to put here
					tokenList[idx].lineNumber
				));
			}
			name = tokenList[idx].variableName;
			index = indexVec_lambda<string, Function>(name, functionList,
				[](string& n, Function& f) -> bool {
					return n == f.name;
				});
			lli sizeOfFunctionCall;
			argList = ReadInArgumentList(tokenList, idx + 1, &sizeOfFunctionCall);
			sizeOfFunctionCall++;//name of function
			tokenCount = sizeOfFunctionCall;
			init_bool = true;
		}
		vector<Iasm::Instruction> AllocateArgList() {
#if safe == true
			if (init_bool == false) {
				err("not init");
			}
#endif
			vector<Iasm::Instruction> instList;
			Function& func = functionList[index];
			loop(i, 0, argList.expList.size()) {
				//might overwrite value in reg2 ????????
				argList.MoveRegister(i, Iasm::REG2, instList);
				if (argList.expList[i].resultantType != func.argList.typeIdList[i]) {
					print("!!!!!Wrong type being passed to function!!!!!!!");
				}
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CREATE_TEMP_VAR));
				instList.back().AppendOprand(argList.expList[i].resultantType, Iasm::Oprand::TYPE);
			}	
			return instList;
		}		
	};
	
	vector<Iasm::Instruction> TokensToIasm(vector<Lexer::Token> tokenList, Function& currentFunction) {
		lli labelCount = 0;
		vector<Iasm::Instruction> instList;
		vector<vector<Iasm::Instruction>> closeScopeAdditionalInstructions;
		lli idx = 0;

		auto is_scope = [&](lli idx) -> lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::OPENSCOPE) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::OPENSCOPE));
				jump = 1;
			}
			if (tokenList[idx].id == Lexer::TokenId::CLOSESCOPE) {
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CLOSESCOPE));
				if (closeScopeAdditionalInstructions.size() > 0) {
					instList = vecConcaternate(instList, closeScopeAdditionalInstructions.back());
					closeScopeAdditionalInstructions.pop_back();
				}
				jump = 1;
			}
			return jump;
		};
		auto is_if = [&](lli idx) -> lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::IF) {
				Exp exp;
				exp.ReadInExp(tokenList, idx+1, { Lexer::TokenId::OPENSCOPE });
				jump += 1 + exp.tokenList.size();//add 1 for IF token
				vector<Iasm::Instruction> expInstList = exp.Compile();
				instList = vecConcaternate(instList, expInstList);
				//instList.push_back(Iasm::Instruction(Iasm::Opcode::SETE));
				//instList.back().AppendOprand(0, Iasm::Oprand::CONST);
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CJMP));
				instList.back().AppendOprand(Iasm::REG1, Iasm::Oprand::REG);
				instList.back().AppendOprand(labelCount, Iasm::Oprand::LABEL_INDEX);
				closeScopeAdditionalInstructions.push_back({
					Iasm::Instruction(Iasm::Opcode::LABEL,labelCount,Iasm::Oprand::CONST)
					});
				labelCount++;
			}
			return jump;
		};
		auto is_while = [&](lli idx) -> lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::WHILE) {
				Exp exp;
				exp.ReadInExp(tokenList, idx + 1, { Lexer::TokenId::OPENSCOPE });
				jump += 1 + exp.tokenList.size();//add 1 for WHILE token
				vector<Iasm::Instruction> expInstList = exp.Compile();

				instList.push_back(Iasm::Instruction(Iasm::Opcode::LABEL));
				instList.back().AppendOprand(labelCount, Iasm::Oprand::CONST);
				labelCount++;

				instList = vecConcaternate(instList, expInstList);
				//instList.push_back(Iasm::Instruction(Iasm::Opcode::SETE));
				//instList.back().AppendOprand(0, Iasm::Oprand::CONST);
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CJMP));
				instList.back().AppendOprand(Iasm::REG1, Iasm::Oprand::REG);
				instList.back().AppendOprand(labelCount, Iasm::Oprand::LABEL_INDEX);
				closeScopeAdditionalInstructions.push_back({
					Iasm::Instruction(Iasm::Opcode::JMP,labelCount-1,Iasm::Oprand::LABEL_INDEX),
					Iasm::Instruction(Iasm::Opcode::LABEL,labelCount,Iasm::Oprand::CONST)
					});
				labelCount++;
			}
			return jump;
		};
		auto is_IOMacro = [&](lli idx) -> lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::PRINT_STRING
				or tokenList[idx].id == Lexer::TokenId::PRINT_NUMBER) {
				//instList.push_back(Iasm::Instruction(Iasm::Opcode::PRINT));
				//instList.back().AppendOprand(tokenList[idx + 2].variableName, Iasm::Oprand::NAME);
				//jump += 4;
				lli argumentListSize;
				ArgumentList argList = ReadInArgumentList(tokenList, idx + 1,&argumentListSize);
				if (argList.expList.size() != 1) {
					err("Passing multiple aregument to print");
				}
				jump += 1;//print token
				jump += argumentListSize;
				if (tokenList[idx + jump].id != Lexer::TokenId::SEMI_COLON) {
					//err("No semi colon after calling print function");
					IDE_Handler::handler.CreateException(IDE_Handler::Exception(
						"No semi colon after calling print function",
						__LINE__, __FILE__,
						"", // also dont know what to put here
						null, //dont know what to put here
						tokenList[idx + jump].lineNumber
					));
				}
				jump += 1;
				//mov exp[0] ro reg1 and then call print reg1
				argList.MoveRegister(0, Iasm::REG1, instList);
				lli print_opcode = Iasm::Opcode::PRINT_STRING;
				if (tokenList[idx].id == Lexer::TokenId::PRINT_NUMBER) {
					print_opcode = Iasm::Opcode::PRINT_NUMBER;
				}
				instList.push_back(Iasm::Instruction(print_opcode));
				instList.back().AppendOprand(Iasm::REG1, Iasm::Oprand::REG);
				return jump;
			}
			if (tokenList[idx].id == Lexer::TokenId::INPUT) {
				//instList.push_back(Iasm::Instruction(Iasm::Opcode::INPUT));
				err("Not written yet");
				jump += 1;
			}
			return jump;
		};
		auto is_simple_functionCall = [&](lli idx)->lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::FUNCTION_CALL) {
				lli argTokenCount;
				ArgumentList argList = ReadInArgumentList(tokenList, idx + 1, &argTokenCount);
				//TODO parse argument to function
				instList.push_back(Iasm::Instruction(Iasm::Opcode::CALL));
				instList.back().AppendOprand(tokenList[idx].variableName, Iasm::Oprand::NAME);
				//TODO error check make sure function is defined
				if (tokenList[idx + 1 + argTokenCount].id != Lexer::TokenId::SEMI_COLON) {
					err("No semi colon on line: " + to_string(tokenList[idx].lineNumber));
				}
				jump += 1 + argTokenCount + 1;
			}
			return jump;
		};
		auto is_var_def = [&](lli idx) -> lli{
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::TYPE) {
				if (tokenList[idx + 1].id == Lexer::TokenId::NAME) {
					jump += 2;// TYPE NAME
					vector<lli> variableTypeList(1);
					vector<string> variableNameList(1);
					variableTypeList[0] = tokenList[idx].type;
					variableNameList[0] = tokenList[idx + 1].variableName;
					lli prevType = sbitMax32;
					//find all varaibles
					while (1) {
						//check if there is multiple variables in statment
						bool foundVariableDeclaration = false;
						lli addToJump = 0;
						if (tokenList[idx + jump].id == Lexer::TokenId::COMMA) {
							if (tokenList[idx + jump + 1].id == Lexer::TokenId::TYPE) {
								if (tokenList[idx + jump + 2].id == Lexer::TokenId::NAME) {
									variableNameList.push_back(tokenList[idx + jump + 2].variableName);
									variableTypeList.push_back(tokenList[idx + jump + 1].type);
									prevType = tokenList[idx + jump + 1].type;
									foundVariableDeclaration = true;
									addToJump = 3;
								}
								else {
									IDE_Handler::handler.CreateException(IDE_Handler::Exception(
										"Found type name but was not followed by name",
										__LINE__, __FILE__,
										"", // also dont know what to put here
										null, //dont know what to put here
										tokenList[idx + jump].lineNumber
									));
								}
							}
							//use prevoisloy used type as type for new variable
							else {
								variableNameList.push_back(tokenList[idx + jump + 1].variableName);
								variableTypeList.push_back(prevType);
								foundVariableDeclaration = true;
								addToJump = 2;
							}
						}
						jump += addToJump;
						if (foundVariableDeclaration == false) {
							break;
						}
					}
					//idx + jump is on the equ sign if there is one

					vector<Iasm::Instruction> createVarInstList(variableNameList.size());
					loop(i, 0, variableNameList.size()) {
						createVarInstList[i] = Iasm::Instruction(Iasm::Opcode::CREATE_VARIABLE);
						createVarInstList[i].AppendOprand(variableTypeList[i], Iasm::Oprand::TYPE);
						createVarInstList[i].AppendOprand(variableNameList[i],
							Iasm::Oprand::NAME);
						//createVarInstList[i].PrintInstruction();
					}

					//is simple int a = 10;
					if (variableNameList.size() == 1) {
						bool hasEqu = false;
						if (tokenList[idx + 2].id == Lexer::TokenId::EQU) {
							jump += 1; // EQU
							Exp exp;
							lli expReadSize = exp.ReadInExp(tokenList, idx + 3);
							instList = vecConcaternate(instList, exp.Compile(false));
							print("--------Final--------");
							fin(i, instList) {
								i.PrintInstruction();
							}
							instList = vecConcaternate(instList, createVarInstList);
							instList.push_back(Iasm::Instruction(Iasm::Opcode::READ));
							instList.back().AppendOprand(tokenList[idx + 1].variableName, Iasm::Oprand::NAME);
							hasEqu = true;
							jump += expReadSize + 1; // EXP SEMI_COLON
						}
						else {
							if (tokenList[idx + 2].id != Lexer::TokenId::SEMI_COLON) {
								err("No semi colon after variable decliration");
							}
						}
					}
					//mult return from a function int a, float b = func();
					else {
						//create all the varaibles
						instList = vecConcaternate(instList, createVarInstList);
						//get function call
						if (tokenList[idx + jump].id != Lexer::TokenId::SEMI_COLON) {
							if (tokenList[idx + jump].id != Lexer::TokenId::EQU) {
								IDE_Handler::handler.CreateException(IDE_Handler::Exception(
									"Did not find semi colon after multiple value definition\n"
									"and did not find equ sigh for assignment.",
									__LINE__, __FILE__,
									"", // also dont know what to put here
									null, //dont know what to put here
									tokenList[idx + jump].lineNumber
								));
							}
							//not semi colon and is an equal sign
							FunctionCall fc;
							lli functionCallIndex = idx + jump + 1;
							fc.Init(tokenList, functionCallIndex);
							instList = vecConcaternate(instList, fc.AllocateArgList());
							jump += fc.tokenCount + 1;
							//check for semin colon
							if (tokenList[idx + jump].id != Lexer::TokenId::SEMI_COLON) {
								IDE_Handler::handler.CreateException(IDE_Handler::Exception(
									"Could not find semi colon after multi ret function\n"
									"was called.",
									__LINE__, __FILE__,
									"", // also dont know what to put here
									null, //dont know what to put here
									tokenList[idx + jump].lineNumber
								));
							}
							jump++; // jump over semi colon
							is_simple_functionCall(functionCallIndex);
						}
					}
				}
			}
			return jump;
		};
		auto is_var_assignment = [&](lli idx) -> lli {
			lli jump = 0;
			try {
				if (tokenList[idx].id == Lexer::TokenId::NAME) {
					if (tokenList[idx + 1].id == Lexer::TokenId::EQU) {
						Exp exp;
						lli expReadSize = exp.ReadInExp(tokenList, idx + 2);
						instList = vecConcaternate(instList, exp.Compile(false));
						jump += expReadSize + 1 + 2; // EXP + SEMI_COLON + NAME + EQU
						//instList.push_back(Iasm::Instruction(Iasm::Opcode::READ, { tokenList[idx + 1].variableName }, {Iasm::Oprand::NAME}));
						instList.push_back(Iasm::Instruction(Iasm::Opcode::READ));
						instList.back().AppendOprand(tokenList[idx].variableName, Iasm::Oprand::NAME);						
					}
				}
			}
			catch (std::overflow_error& e) {
				return 0;
			}
			return jump;			
		};
		auto is_return = [&](lli idx)->lli {
			lli jump = 0;
			if (tokenList[idx].id == Lexer::TokenId::RETURN) {				
				jump++;
				if (tokenList[idx + jump].id == Lexer::TokenId::SEMI_COLON) {
					//for void functions
					if (currentFunction.argList.typeIdList.size() != 0) {
						IDE_Handler::handler.CreateException(IDE_Handler::Exception(
							"returning void from function that does not return void.",
							__LINE__, __FILE__,
							"", // also dont know what to put here
							null, //dont know what to put here
							tokenList[idx + jump].lineNumber
						));
					}
				}	
				//read in values
				vector<Exp> expList;
				while (true) {					
					expList.push_back(Exp());
					expList.back().ReadInExp(tokenList, idx + jump, {
						Lexer::TokenId::COMMA,
						Lexer::TokenId::SEMI_COLON
					});
 					jump += expList.back().tokenList.size();
					if (tokenList[idx + jump].id == Lexer::TokenId::SEMI_COLON) {
						break;
					}
					if (tokenList[idx + jump].id != Lexer::TokenId::COMMA) {
						IDE_Handler::handler.CreateException(IDE_Handler::Exception(
							"No comma during return from a multi ret function.",
							__LINE__, __FILE__,
							"", // also dont know what to put here
							null, //dont know what to put here
							tokenList[idx + jump].lineNumber
						));
					}
					jump++;//jump over comma
				}
				loopR(i,0,expList.size()) {
					if (expList[i].resultantType != currentFunction.returnTypeList[i]) {
						IDE_Handler::handler.CreateException(IDE_Handler::Exception(
							"Returned type does not match the one specified\n"
							"The argument in question -> " + to_string(i+1) + ".",
							__LINE__, __FILE__,
							"", // also dont know what to put here
							null, //dont know what to put here
							tokenList[idx + jump].lineNumber
						));
					}
					instList = vecConcaternate(instList, expList[i].Compile(false));
					//exp now in reg1
					instList.push_back(Iasm::Instruction(Iasm::Opcode::WRITE_ARG));
					instList.back().AppendOprand(expList.size() - i - 1, Iasm::Oprand::CONST);
					instList.back().AppendOprand(expList[i].resultantType, Iasm::Oprand::TYPE);
					instList.back().AppendOprand(Iasm::REG1, Iasm::Oprand::REG);
				}
				if (tokenList[idx + jump].id != Lexer::TokenId::SEMI_COLON) {
					IDE_Handler::handler.CreateException(IDE_Handler::Exception(
						"no semi colon after mult return.",
						__LINE__, __FILE__,
						"", // also dont know what to put here
						null, //dont know what to put here
						tokenList[idx + jump].lineNumber
					));
				}
				jump++;
			}
			return jump;
		};

		while (idx < tokenList.size()) {
			lli newIdx;
#define testFunction(func) newIdx = func(idx); if (newIdx != 0) { idx += newIdx; continue; }
			testFunction(is_var_def);
			testFunction(is_scope);
			testFunction(is_IOMacro);
			testFunction(is_if);
			testFunction(is_var_assignment);
			testFunction(is_while);
			testFunction(is_simple_functionCall);
			testFunction(is_return);
#undef testFunction

			string failedToken = tokenList[idx].printToken(true);
			string lineNumber = to_string(tokenList[idx].lineNumber);
			//err("Unknown token combination: " + failedToken + " On line " + lineNumber);
			IDE_Handler::handler.CreateException(IDE_Handler::Exception(
				"Unknown token combination: " + failedToken,
				__LINE__, __FILE__,
				"", // also dont know what to put here
				null, //dont know what to put here
				tokenList[idx].lineNumber
			));
		}
		return instList;
	}
	void CompileFunctions() {
		loop(f, 0, functionList.size()) {			
			vector<Iasm::Instruction> instList(1);
			instList[0] = Iasm::Instruction(Iasm::Opcode::CREATE_FUNCTION);
			instList = vecConcaternate(instList, TokensToIasm(functionList[f].tokenList, functionList[f]));
			functionList[f].instructionList = instList;
			functionList[f].instructionList.emplace_back(Iasm::Instruction(Iasm::Opcode::RET));
		}
	}
	void GenerateIntermediateAsm(vector<Lexer::Token> tokenList) {
		FindFunctions(tokenList);
		CompileFunctions();

		print("-----Functions-----");
		fin(f, functionList) {
			f.PrintFunction();
			print(f.name + "---------IASM---------");
			f.PrintInstructionList();
			print();
		}
		print();		
	}
}