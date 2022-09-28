//#define STB_IMAGE_IMPLEMENTATION
//#include "../../aaa_Headers/stb_image.h"
#ifndef _DEBUG
	#define safe false
#else
	#define safe true
#endif

//#include <any>
#include <thread>
#include <atomic>

//#include "../../aaa_Headers/preProcessor.h"
#include "PreProc/preProcessor.h"

using namespace PreProc;
//(gci -include *.cpp,*.h -recurse | select-string .).Count


//Must all be in same order eg void comes first in each list
vector<string> typeNameList = { "void","int","float" };
vector<lli> typeSizeList = { 0,       4,      4 };
namespace TypeIdList{
	enum {
		VOID,INT,FLOAT
	};
}
string mainSourceCode;

#include "IDE_Handler.h"
#include "Operators.h"
#include "IntermediateCode.h"
#include "Lexer.h"
#include "SA.h"
#include "Env.h"
#include "Assemble.h"
#include "IDE.h"

void CompileCode(string fileContent) {
	Lexer::GenerateTokens(fileContent);
	Lexer::PrintTokenList();

	AST::GenerateIntermediateAsm(Lexer::tokenList);
}

int main() {
	string filePath = "code.w";

	bool succ = false;
	string fileContent = readFileToString(filePath,&succ);
	if (succ == false) { err("Failed to load input file"); }
	mainSourceCode = fileContent;

	IDE::Init();
	Lexer::Init();
	//IDE::SourceCodeEditor();	
	Operators::OperatorInit();
	Iasm::Opcode::Init();
	CompileCode(fileContent);

	const bool compile = false;
	if (compile == false) {
		//IDE::SetBreakPoints(&SA::functionList);

		Env::Settings::debug = true;
		Env::Settings::time = true;
		Env::Init();
		Env::Execute(AST::functionList);
	}
	else {
		Assemble::Settings::os = false;
		Assemble::Settings::is_nasm = true;
		string code = Assemble::AssembleCode(AST::functionList);
		std::ofstream outFile;
		//.asm appended
		string writeFile = "C:\\Users\\xbox\\Documents\\Prog\\cpp\\TestGenerateAsmCode\\asm";
		outFile.open(writeFile + ".asm");
		outFile << code;
		outFile.close();
		
		if (Assemble::Settings::is_nasm) {
			//execute code
			system(("echo Executing.. " + writeFile + ".asm").c_str());
			print();
			string compileInstruction = str("nasm -fwin32 ") + writeFile + ".asm";
			system(compileInstruction.c_str());
			string linkInstruction = "gcc " + writeFile + ".obj -o " + writeFile + ".exe";
			system(linkInstruction.c_str());
			//Duration t;
			//t.start();
			system((writeFile + ".exe").c_str());
			//print("Time: ",t.end());
		}
	}

	print("Done");

	return 0;
}