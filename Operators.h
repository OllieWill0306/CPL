#pragma once
#include "IntermediateCode.h"
namespace Operators {
	class Operator {
	public:
		string name;
		lli argCount = 2;
		lli opcode;

		Operator() = default;
		Operator(string name) { self.name = name; }
		Operator(string name, lli opcode) {
			self.name = name;
			self.opcode = opcode;
		}
	};
	vector<Operator> operatorList;
	vector<vector<Operator>> bidmasList = {
		{Operator("%",Iasm::Opcode::MOD)},
		{
			Operator("*",Iasm::Opcode::MUL),
			Operator("/",Iasm::Opcode::DIV)
		},
		{Operator("+",Iasm::Opcode::ADD)},

		{
			Operator("!=",Iasm::Opcode::SETNE),
			Operator("==",Iasm::Opcode::SETE),
			Operator(">",Iasm::Opcode::SETG),
			Operator("<",Iasm::Opcode::SETL)
		}
	};
	void OperatorInit() {
		//reshape to single list
		loopR(b, 0, bidmasList.size()) {
			loopR(o, 0, bidmasList[b].size()) {
				operatorList.push_back(bidmasList[b][o]);
			}
		}
	}
	vector<string> GetOperatorNameList() {
		vector<string> nameList;
		fin(i, operatorList) {
			nameList.push_back(i.name);
		}
		return nameList;
	}
}