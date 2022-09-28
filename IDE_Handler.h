#pragma once

//create exception code

//IDE_Handler::Exception e("Expected a comma", __LINE__, __FILE__,
//	Lexer::ReturnTokenLineForException(tokenList, idx), idx, tokenList[idx].lineNumber);
//IDE_Handler::handler.CreateException(e);

namespace AST{
	struct Function;
}
namespace Env {
	struct RunTimeEnvData;
}
namespace Iasm {
	struct Instruction;
}

namespace IDE_Handler {
	struct Exception {
		string errorMsg;
		lli lineNumber;
		string fileName;
		string lineString;
		lli exactPosition;
		lli sourceCodeLineNumber;

		bool init_bool = false;

		void Init(string errorMsg, lli lineNumber,
			string fileName, string lineString, lli exactPosition, lli sourceCodeLineNumber) {
			self.errorMsg = errorMsg;
			self.lineNumber = lineNumber;
			self.fileName = fileName;
			self.lineString = lineString;
			self.exactPosition = exactPosition;
			self.sourceCodeLineNumber = sourceCodeLineNumber;
			init_bool = true;
		}
		Exception(string errorMsg, lli lineNumber,
			string fileName, string lineString, lli exactPosition, lli sourceCodeLineNumber) {
			Init(errorMsg, lineNumber, fileName, lineString, exactPosition, sourceCodeLineNumber);
		}
		Exception() = default;
		string ReturnErrorString() {
			string out;
			out += errorMsg + "\n\n";
			out += "Occured on line -> " + to_string(lineNumber) + "\n";
			out += "In file -> " + fileName + "\n";
			out += "Source Code Error Line -> " + to_string(sourceCodeLineNumber);
			return out;
		}
	};
	struct Handler {
		//renders source code when there is an exception
		fnPtr(void, SourceCodeException, Exception*) = nullptr;
		//for settings break points
		fnPtr(vector<vector<bool>>, ByteCodeSetBreakPoints, vector<AST::Function>*) = nullptr;
		//renders enviroment for debugging
		fnPtr(void, EnvDebugger, Env::RunTimeEnvData*) = nullptr;

		void CreateException(Exception e) {
			print(ERROR_LINE);
			print("Exception has been passed to IDE_Handler");
			if (SourceCodeException == nullptr) {
				err("Exceptoion passed to IDE handler\n"
					"Created source code excetption but source code exception function ptr never set\n"
					"It is nullptr");
			}
			SourceCodeException(&e);
			err("Should not return from error parsed to IDE");
		}
	};
	Handler handler;
}