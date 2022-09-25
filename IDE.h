#pragma once
namespace IDE {
	#pragma comment(lib, "../../Libs/glfw/lib-vc2022/glfw3.lib")
	#pragma comment(lib, "../../Libs/glew/lib/Release/x64/glew32s.lib")
	#pragma comment(lib, "opengl32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "user32.lib")
	#pragma comment(lib, "shell32.lib")
	#define GLEW_STATIC
	#include "../../Libs/glew/include/GL/glew.h"
	#include "../../Libs/glfw/include/GLFW/glfw3.h"

	const int fontX = 8;// x pixles per character
	const int fontY = 12;// y pixels per character
	const int charPerX = 16;//number of characters on x
	const int charPerY = 8;// number of characters on y
	const int fontImgX = fontX * charPerX;
	const int fontImgY = fontY * charPerY;
	//constexpr float fontRenderScale = 0.025f;
	//constexpr float fontRenderSizeX = (float)fontX * fontRenderScale;
	//constexpr float fontRenderSizeY = (float)fontY * fontRenderScale;
	unsigned char font_bitmap[fontImgX * fontImgY * 3];
	namespace Settings {
		float fontScale = 2.f;
		int inline GetLineSpacing(int numberOfLines) {
			return (int)(numberOfLines * (fontY + 1) * fontScale);
		}
		const int sideBarSize = fontX * fontScale * 5;
		const bool multiThread = false;
		string tab = "    ";
	}
	int scroll = 0;
	int cursor = 0;
	int prevMouseState = 0;

	const int screenx = 2560;//3840;
	const int screeny = 1440;// 2560;

	int newScreenx = screenx;
	int newScreeny = screeny;
	

	struct float2 {
		float x, y;
	};
	struct float3 {
		float r, g, b;
		float3() = default;
		float3(float r, float g, float b,const bool div=false) {
			if (div) {
				r /= 255.f;
				g /= 255.f;
				b /= 255.f;
			}
			self.r = r;
			self.g = g;
			self.b = b;
		}
		void operator=(float3 c) {
			r = c.r;
			g = c.g;
			b = c.b;
		}
	};
	struct int2 {
		int x, y;
		int2() = default;
		int2(int x, int y) {
			self.x = x;
			self.y = y;
		}
	};

	vector<Lexer::Token> tokenList;
	vector<string> splitStringList;
	vector<bool> newLineList;


	//render functions
	GLFWwindow* window;
	//textures
	uint fontTexture;

	//loading textures
	void LoadTextureMap() {
		readFileToBuffer("font.bin", font_bitmap, sizeof(font_bitmap));
		glGenTextures(1, &fontTexture);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fontImgX, fontImgY, 0, GL_RGB, GL_UNSIGNED_BYTE, font_bitmap);
		glGenerateMipmap(GL_TEXTURE_2D);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glEnable(GL_TEXTURE_2D);
	}
	//reander code
	float2 inline toOpenglCoord(int x, int y) {
		float2 ret;
		ret.x = (float)(x - screenx) / (screenx / 2.f) + 1.f;
		ret.y = -((float)(y - screeny) / (screeny / 2.f) + 1.f);
		return ret;
	}
	int2 inline toNormalCoord(float x, float y) {
		int2 ret;
		ret.x = (x + 1.f) / 2.f * screenx;
		ret.y = (y + 1.f) / 2.f * screeny;
		return ret;
	}
	void RenderCharacter(int x, int y, int sizex, int sizey, char asciiValue, float r = 1.f, float g = 1.f, float b = 1.f) {
		float2 topLeft = toOpenglCoord(x, y);
		float2 topRight = toOpenglCoord(x + sizex, y);
		float2 bottemRight = toOpenglCoord(x + sizex, y + sizey);
		float2 bottemLeft = toOpenglCoord(x, y + sizey);

		constexpr float charXSize_rel = (float)fontX / (float)fontImgX;
		constexpr float charYSize_rel = (float)fontY / (float)fontImgY;

		//top left vertex x position
		float xTexturePosition = (float)(asciiValue % charPerX * fontX) / (float)fontImgX;
		// top left vertex y position
		float yTexturePosition = (float)(floor((float)asciiValue / (float)charPerX) * fontY) / (float)fontImgY;


		glBegin(GL_QUADS);

		glColor3f(r, g, b);
		glTexCoord2f(xTexturePosition, yTexturePosition + charYSize_rel);	//top left	// 0,1				
		glVertex3f(bottemLeft.x, bottemLeft.y, 0);
		//topLeft.x, topLeft.y

		glColor3f(r, g, b);
		glTexCoord2f(xTexturePosition + charXSize_rel, yTexturePosition + charYSize_rel);//bottom  left // 1, 1				
		glVertex3f(bottemRight.x, bottemRight.y, 0);
		//bottemLeft.x, bottemLeft.y

		glColor3f(r, g, b);
		glTexCoord2f(xTexturePosition + charXSize_rel, yTexturePosition);// bottem right // 1, 0				
		glVertex3f(topRight.x, topRight.y, 0);
		//bottemRight.x, bottemRight.y


		glColor3f(r, g, b);
		glTexCoord2f(xTexturePosition, yTexturePosition);//top right // 0,0
		glVertex3f(topLeft.x, topLeft.y, 0);
		//topRight.x, topRight.y

		glEnd();
		//glFlush();
	}
	int2 RenderText(string text, int x, int y, float scale = 4.f, float r = 1.f, float g = 1.f, float b = 1.f) {
		int sizex = fontX * scale;
		int sizey = fontY * scale;
		int xPos = 0;
		int lineCount = 0;
		int2 size(0,0);
		loop(c, 0, text.size()) {
			switch (text[c]) {
			case '\n':
				//y += fontY * Settings::fontScale;
				xPos = 0;
				lineCount++;
				break;
			case '\t':
				xPos += Settings::tab.size();
				break;
			case ' ':
				break;
			default:
				size.x = max(size.x, sizex * (xPos + 1));
				size.y = max(size.y, sizey * (lineCount + 1));
				RenderCharacter(x + sizex * xPos, y + sizey * lineCount, sizex, sizey, text[c], r, g, b);
			}	
			xPos++;
		}
		return size;
	}
	void RenderRect(int x, int y, int sizex, int sizey, float r, float g, float b) {
		float2 topLeft = toOpenglCoord(x, y);
		float2 topRight = toOpenglCoord(x + sizex, y);
		float2 bottemLeft = toOpenglCoord(x, y + sizey);
		float2 bottemRight = toOpenglCoord(x + sizex, y + sizey);

		glBindTexture(GL_TEXTURE_2D, 0);

		glBegin(GL_QUADS);

		glColor3f(r, g, b);
		glVertex3f(topLeft.x, topLeft.y, 0);

		glColor3f(r, g, b);
		glVertex3f(bottemLeft.x, bottemLeft.y, 0);

		glColor3f(r, g, b);
		glVertex3f(bottemRight.x, bottemRight.y, 0);

		glColor3f(r, g, b);
		glVertex3f(topRight.x, topRight.y, 0);

		glEnd();

		glBindTexture(GL_TEXTURE_2D, fontTexture);
	}
	//call backs
	void WindowSizeCallback(GLFWwindow* window, int width, int height){
		//glfwSetWindowSize(window, width, height);
		glViewport(0, 0, width, height);
		newScreenx = width;
		newScreeny = height;
	}
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)	{
		constexpr float sens = 100.f;
		scroll += yoffset * sens;
	}
	//stuff
	float3 GetOpcodeColour(lli opcode) {
		float3 ret;
		float opcode_red = 0.5f;
		float opcode_green = 1.f;
		float opcode_blue = 1.f;
		using namespace Iasm::Opcode;
		switch (opcode) {
		case LABEL:
			opcode_red = 1.f;
			opcode_green = 0.f;
			opcode_blue = 0.f;
			break;
		case CLOSESCOPE:
		case OPENSCOPE:
			opcode_red = 252.f;
			opcode_green = 0.f;
			opcode_blue = 219.f;
			break;
		}
		ret.r = opcode_red;
		ret.g = opcode_green;
		ret.b = opcode_blue;
		return ret;
	}
	int2 GetMousePos() {
		//get mouse pos
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		int2 ret;
		ret.x = mx / newScreenx * screenx;
		ret.y = my / newScreeny * screeny;
		return ret;
	}


	void DisplayControllList(vector<string>& stringList) {		
		loop(i, 0, stringList.size()) {	
			int ts = stringList[i].size();//text size
			RenderText(stringList[i], screenx - (ts * fontX * Settings::fontScale),
				Settings::GetLineSpacing(i), Settings::fontScale, 1, 1, 1);
		}
	}
	void DisplayText(vector<string>& splitList, vector<float3>& colourList, vector<bool>& newLineList,
		IDE_Handler::Exception* exception = nullptr)
	{
//#if safe == true
//		if (splitList.size() == colourList.size() and splitList.size() != newLineList.size()) {
//			err("List do not match size");
//		}
//#endif
		const int sideBarX = fontX * Settings::fontScale * 5;
		RenderRect(0, 0, sideBarX, screeny, 0.5, 0.5, 0.6);
		int lineCount = 0;
		int lineSize = sideBarX;
		int errorMsgOffset = 0;
		lli minSize = min(colourList.size(), newLineList.size());
		minSize = min(splitList.size(), minSize);
		loop(i, 0, minSize) {
			if (newLineList[i]) {
				lineCount++;
				lineSize = sideBarX;
			}
			int currentYPos = Settings::GetLineSpacing(lineCount) + scroll + errorMsgOffset;
			float r = colourList[i].r; float g = colourList[i].g; float b = colourList[i].b;
			RenderText(splitList[i], lineSize, currentYPos, Settings::fontScale,
				r, g, b);
			//render line number
			RenderText(to_string(lineCount), 0, currentYPos, Settings::fontScale,
				0.5, 0.3, 0.7);
			lineSize += splitList[i].size() * fontX * Settings::fontScale;
			if (exception != nullptr) {
				if (lineCount == exception->sourceCodeLineNumber and newLineList[i]) {
					string lines = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
					string newError = lines + "\n" + exception->ReturnErrorString() + "\n" + lines;
					int2 dims = RenderText(newError, sideBarX, currentYPos, Settings::fontScale,
						1, 0, 0);
					errorMsgOffset += dims.y;
				}
			}			
		}
	}
	//
	float3 SourceCodeGetColour(Lexer::Token& token) {
		float3 c(1, 1, 1);
		using namespace Lexer::TokenId;
		switch (token.id) {
		case TYPE:
			c = float3(176, 9, 98, true);
			break;
		case OPENBRACKET:
		case CLOSEBRACKET:
		case OPP:
			c = float3(128, 128, 128, true);
			break;
		//controll flow
		case RETURN:
		case IF:
		case WHILE:
		case FN:
			c = float3(199, 125, 34, true);
			break;
		//inbuilt function
		case PRINT_STRING:
		case PRINT_NUMBER:
			c = float3(78, 189, 47, true);
			break;
		case CONST:
			c = float3(168, 30, 37, true);
			break;
		case SEMI_COLON:
			c = float3(15, 199, 219, true);
			break;
		case EQU:
			c = float3(45, 176, 19, true);
			break;
		}
		return c;
	}
	void ThreadUpdateColourList(bool* kill,	ThreadMaster<vector<string>>* stringSplitListMaster,
		ThreadMaster<vector<bool>>* newLineListMaster, ThreadMaster<vector<float3>>* colourListMaster,
		IDE_Handler::Exception* exception)
	{
		ThreadSlave<vector<string>> stringSplitList(stringSplitListMaster);
		ThreadSlave<vector<bool>> newLineList(newLineListMaster);
		ThreadSlave<vector<float3>> colourList(colourListMaster);

		while ($(kill) == false) {
			Lexer::SplitStringCode_ret ssc_ret = Lexer::SplitStringCode(mainSourceCode);			
			stringSplitList.Set(ssc_ret.splitList);
			newLineList.Set(ssc_ret.newLineList);
			vector<Lexer::Token> tokenList(ssc_ret.splitList.size());
			
			lli lineCount = 0;
			loop(i, 0, ssc_ret.splitList.size()) {
				lli jump = tokenList[i].SetTokenFromString(ssc_ret.splitList, i, &lineCount);
				if (tokenList[i].is_newLine) {
					lineCount++;
				}
				//if (exception != nullptr) {
				//	if (lineCount == exception->sourceCodeLineNumber) {
				//		ssc_ret.splitList.insert(ssc_ret.splitList.begin() + i, exception->errorMsg);
				//		ssc_ret.newLineList.insert(ssc_ret.newLineList.begin() + i, false);
				//		errorTokenIdx = i;
				//	}
				//}
				loop(j, 1, jump+1) {
					tokenList[i + j] = tokenList[i];
				}
				i += jump;
			}

			
			vector<float3> newColourList(tokenList.size());
			loop(i, 0, tokenList.size()) {
				newColourList[i] = SourceCodeGetColour(tokenList[i]);								
			}
			colourList.Set(newColourList);
		}
	exit:
		pass;
	}
	void SourceCodeEditor(IDE_Handler::Exception* exception = nullptr) {
		Duration timer;

		bool killThread = false;
		vector<string> keyList = {
			"F5 to compile and run"
		};
		ThreadMaster<vector<bool>> newLineList;
		ThreadMaster<vector<string>> splitStringList;
		ThreadMaster<vector<float3>> colourList;
		newLineList.SetCurrentData(vector<bool>());
		splitStringList.SetCurrentData(vector<string>());
		colourList.SetCurrentData(vector<float3>());	

		std::thread UpdateColourThread(&ThreadUpdateColourList,&killThread,
			&splitStringList, &newLineList, &colourList, exception);

		while (!glfwWindowShouldClose(window)) {
			timer.start();
			glClear(GL_COLOR_BUFFER_BIT);

			DisplayControllList(keyList);
			//Lexer::SplitStringCode_ret ssc_ret = Lexer::SplitStringCode(mainSourceCode);
			
			//colourList.resize(ssc_ret.splitList.size());
			DisplayText(splitStringList.Get(), colourList.Get(), newLineList.Get(), exception);

			double time_sec = timer.milliToSeconds(timer.end());
			string title = "FPS: " + to_string(1.0 / time_sec);
			glfwSetWindowTitle(window, title.c_str());
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		killThread = true;
		UpdateColourThread.join();//make sure it has quitted
	}
	//
	vector<float3> ByteCodeGetColourList(vector<string>& splitList) {
		vector<float3> colourList;

		return colourList;
	}
	vector<vector<bool>> ByteCodeSetBreakPoints(vector<SA::Function>* functionList_ptr) {
		vector<vector<bool>> breakPointList;
		vector<SA::Function>& functionList = $(functionList_ptr);
		
		breakPointList.resize(functionList.size());
		loop(f, 0, functionList.size()) {
			breakPointList[f].resize(functionList[f].instructionList.size());
			loop(i, 0, functionList[f].instructionList.size()) {
				breakPointList[f][i] = false;
			}
		}

		return breakPointList;
	}
	bool b_enterPressed = false;
	void DisplayStackMemory(Env::RunTimeEnvData* rted) {
		const int BYTES_PER_LINE = 4;
		const int renderX = screenx - (Settings::fontScale * fontX *
			BYTES_PER_LINE * 2 * 2);
		string render;
		loop(i, 0, Env::stack.usedStack) {
			if (i % BYTES_PER_LINE == 0) {
				render += "\n";
			}
			byte b = Env::stack[i];
			render += byteListToString(&b, 1, -1);
			render += " ";
		}
		RenderText(render, renderX, Settings::fontScale* fontY * 2, Settings::fontScale, 1, 1, 1);
	}
	void ByteCodeEnvDebuger(Env::RunTimeEnvData* rted) {	
		b_enterPressed = false;
		auto keyCallBack = [](GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
			if (key == GLFW_KEY_ENTER and action == GLFW_PRESS) {
				b_enterPressed = true;
			}
		};
		glfwSetKeyCallback(window, keyCallBack);
		vector<string> keyList = {
			"Enter to continue"
		};
		while (true) {
			glClear(GL_COLOR_BUFFER_BIT);

			DisplayStackMemory(rted);
			DisplayControllList(keyList);
			//Lexer::SplitStringCode_ret ssc_ret = Lexer::SplitStringCode(mainSourceCode);

			//colourList.resize(ssc_ret.splitList.size());
			//sum number of instructions
			lli totalInstructionCount = 0;
			lli totalFunctionCount = $(rted->functionListPtr).size();
			loop(f, 0, totalFunctionCount) {
				totalInstructionCount += $(rted->functionListPtr)[f].instructionList.size();
			}
			vector<string> stringSplitList(totalInstructionCount + totalFunctionCount);
			vector<float3> colourList(totalInstructionCount + totalFunctionCount);
			vector<bool> newLineList(totalInstructionCount + totalFunctionCount);
			lli index = 0;
			loop(f, 0, $(rted->functionListPtr).size()) {
				string tabString = "";
				stringSplitList[index] = "->" + $(rted->functionListPtr)[f].name + "<-";
				colourList[index] = float3(255, 255, 255, true);
				newLineList[index] = true;
				index++;
				loop(i, 0, $(rted->functionListPtr)[f].instructionList.size()) {
					Iasm::Instruction& currentInst = $(rted->functionListPtr)[f].instructionList[i];
					string instructionString = currentInst.PrintInstruction(true);
					if (currentInst.opcode == Iasm::Opcode::OPENSCOPE) {
						tabString += Settings::tab;
					}
					stringSplitList[index] = tabString + instructionString;
					if (currentInst.opcode == Iasm::Opcode::CLOSESCOPE) {
						loop(j, 0, Settings::tab.size())tabString.pop_back();
					}
					colourList[index] = float3(1, 1, 1);
					if (i == rted->currentInstruction and f == rted->currentFunction) {
						colourList[index] = float3(1, 0, 0);
					}
					newLineList[index] = true;
					index++;
				}
			}

			DisplayText(stringSplitList, colourList, newLineList);

			if (b_enterPressed) {
				break;
			}
			if (glfwWindowShouldClose(window) == true) {
				//exit(1);
				break;
			}
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	//
	void Init() {

		if (!glfwInit()) {
			err("Failed to initilize glfw");
		}
		window = glfwCreateWindow(screenx, screeny, "IDE", NULL, NULL);
		if (!window) {
			glfwTerminate();
			err("Failed to create window")
		}
		glfwMakeContextCurrent(window);
		if (glewInit() != GLEW_OK) {
			err("Failed to inizalize glew");
		}
		LoadTextureMap();

		glfwSetFramebufferSizeCallback(window, WindowSizeCallback);
		glfwSetScrollCallback(window, ScrollCallback);

		//assign exception handlers
		IDE_Handler::handler.SourceCodeException = SourceCodeEditor;
		IDE_Handler::handler.EnvDebugger = ByteCodeEnvDebuger;
		IDE_Handler::handler.ByteCodeSetBreakPoints = ByteCodeSetBreakPoints;

		//if (glfwRawMouseMotionSupported())
		//	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		//Never call glfwTerminate()
	}
}