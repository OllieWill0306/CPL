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
		float fontScale = 4.f;
		int inline GetLineSpacing(int numberOfLines) {
			return numberOfLines * (fontY + 1) * fontScale;
		}
		const int sideBarSize = fontX * fontScale * 5;
		bool multiThread = false;
		string tab = "    ";
	}
	namespace TextMode {
		enum {
			SOURCE,
			BYTE,
			ASM
		};
	}
	int scroll = 0;
	int cursor = 0;
	int prevMouseState = 0;
	Env::Stack* stack;
	vector<vector<Env::Variable>>* scopeVarList;

	const int screenx = 3840;//3840;
	const int screeny = 2560;// 2560;

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
	};

	vector<Lexer::Token> tokenList;
	vector<string> splitStringList;
	vector<bool> newLineList;


	//render functions
	GLFWwindow* window;
	//textures
	uint fontTexture;

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
	void RenderText(string text, int x, int y, float scale = 4.f, float r = 1.f, float g = 1.f, float b = 1.f) {
		int sizex = fontX * scale;
		int sizey = fontY * scale;
		int xPos = 0;
		int lineCount = 0;
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
				RenderCharacter(x + sizex * xPos, y + sizey * lineCount, sizex, sizey, text[c], r, g, b);
			}	
			xPos++;
		}
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

	void InitEnv(Env::Stack* stackPtr_arg, vector<vector<Env::Variable>>* scopeVarList_arg) {
		scopeVarList = scopeVarList_arg;
		stack = stackPtr_arg;
	}
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
	float3 GetTokenColour(Lexer::Token& token) {
		float3 c(1,1,1);		
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
		case IF:
		case WHILE:
		case FN:
			c = float3(199, 125, 34, true);
			break;
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

	void RenderByteCode(vector<SA::Function>* functionList_ptr, vector<vector<bool>>* breakPointList_ptr,
		lli currentInstruction=(lli)sbitMax64)
	{
		int2 mousePos = GetMousePos();
		int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		vector<SA::Function>& functionList = $(functionList_ptr);
		vector<vector<bool>>& breakPointList = $(breakPointList_ptr);
		string tabs = "";
		auto RenderDebugInfo = [](int* yRenderPos) {
			$(yRenderPos) += (fontY + 1) * Settings::fontScale;
			string s =
				"^ Current Line ^ \n"
				"EAX -> " + to_string(Env::register1) + "\n"
				"EBX -> " + to_string(Env::register2) + "\n"
				"Variable List -> " + Env::Debug::DumpVaraibles()
				;
			int newLineCount = 1;
			loop(i, 0, s.size()) {
				if (s[i] == '\n') {
					newLineCount++;
				}
			}
			RenderText(s, 30, $(yRenderPos), Settings::fontScale, 1, 0, 0);

			$(yRenderPos) += newLineCount * (fontY + 1) * Settings::fontScale;
		};
		int yRenderPos = scroll;
		loop(f, 0, functionList.size()) {
			//RenderText(functionList[f].name+":", 0, -scroll);
			loop(i, 0, functionList[f].instructionList.size()) {
				Iasm::Instruction& inst = functionList[f].instructionList[i];
				string inst_string = inst.PrintInstruction(true);
				if (inst.opcode == Iasm::Opcode::CLOSESCOPE) {
					loop(j, 0, Settings::tab.size()) {tabs.pop_back();}
				}
				vector<string> splitList;
				strSplit(inst_string, { ' ' }, &splitList);
				int lineSize = Settings::sideBarSize;

				//render opcode
				float3 opcodeColour = GetOpcodeColour(inst.opcode);
				yRenderPos += (fontY + 1) * Settings::fontScale;
				//render line number
				RenderText(to_string(i), 30, yRenderPos, 4, 0.7, 0.7, 1.0);
				//print(mousePos.y, yRenderPos, (i + 1) * (fontY + 1) * fontScale + scroll);
				if (mouseState == GLFW_PRESS and prevMouseState != GLFW_PRESS) {
					if (mousePos.y > yRenderPos and mousePos.y < yRenderPos + fontY * Settings::fontScale) {
						breakPointList[f][i] = !breakPointList[f][i];
					}
				}
				if (breakPointList[f][i] == true) {
					RenderRect(5, yRenderPos, 20, 20, 1, 0, 0);
				}
				RenderText(tabs + splitList[0] + " ", lineSize, yRenderPos, Settings::fontScale,
					opcodeColour.r, opcodeColour.g, opcodeColour.b);
				lineSize += fontX * Settings::fontScale * (splitList[0].size() + 1 + tabs.size());
				//render oprands
				loop(s, 1, splitList.size()) {
					RenderText(splitList[s] + " ", lineSize, yRenderPos, Settings::fontScale,
						1.f, 1.f, 1.f);
					lineSize += fontX * Settings::fontScale * (splitList[s].size() + 1);//include the space						
				}
				if (inst.opcode == Iasm::Opcode::OPENSCOPE) {
					tabs += Settings::tab;
				}
				if (i == currentInstruction) {
					RenderDebugInfo(&yRenderPos);
				}				
			}
		}
		prevMouseState = mouseState;
	}

	void UpdateTokenList(string& text) {
		Lexer::SplitStringCode_ret ssc_ret = Lexer::SplitStringCode(text);
		tokenList.resize(ssc_ret.splitList.size());	
		//splitStringList = ssc_ret.splitList;
		//newLineList = ssc_ret.newLineList;
		
		lli lineCount = 0;
		lli usedTokenCount = 0;
		loop(i, 0, ssc_ret.splitList.size()) {
			lli jump = tokenList[usedTokenCount].SetTokenFromString(ssc_ret.splitList, i, &lineCount);
			i += jump;
			usedTokenCount++;
		}
		tokenList.resize(usedTokenCount);
		splitStringList.resize(tokenList.size());
		newLineList.resize(tokenList.size());
		loop(i, 0, tokenList.size()) {
			if(i > 0)newLineList[i] = tokenList[i-1].is_newLine;
			splitStringList[i] = tokenList[i].sourceString;
		}
	}
	void TextEditor(const lli mode,string text,IDE_Handler::Exception* exception=nullptr) {
		int2 mousePos = GetMousePos();
		int sideBarSize;
		int breakPointSize;
		int textLength;
		
		switch (mode) {
		case TextMode::SOURCE: {
			sideBarSize = fontX * Settings::fontScale * 5;
			breakPointSize = 0;
			RenderText("F5 to compile", screenx - fontX * 16 * Settings::fontScale, 0, Settings::fontScale, 1, 1, 1);
			if (exception != nullptr) {
				RenderText("Exception has occured", screenx - fontX * 21 * Settings::fontScale, (fontY + 1) * Settings::fontScale,
					Settings::fontScale, 1, 0, 0);
			}
			
			//RenderText(text, sideBarSize, Settings::GetLineSpacing(0) + scroll, Settings::fontScale, 1, 1, 1);
			UpdateTokenList(text);

			//render words
			lli lineCount = 0;
			lli xRenderPos = sideBarSize;
			lli totalCharactersPrinted = 0;
			textLength = splitStringList.size();
			loop(i, 0, splitStringList.size()) {
				if (newLineList[i] == true) {
					lineCount++;
					xRenderPos = sideBarSize;
				}
				if (exception != nullptr) {
					if (lineCount == exception->sourceCodeLineNumber) {
						lineCount++;
						string errMsg = "Exception has been thrown\n"
							+ exception->errorMsg + "\n" +
							"Caused on Token count -> " + to_string(exception->exactPosition) + "\n"
							"Error thrown in file -> " + exception->fileName + "\n"
							"Error thrown on line -> " + to_string(exception->lineNumber) + "\n";
						RenderText(errMsg, sideBarSize,
							scroll + Settings::GetLineSpacing(lineCount), Settings::fontScale,
							1, 0, 0);
						lineCount += 6;
					}
				}
				
				float3 textColour = GetTokenColour(tokenList[i]);
				RenderText(splitStringList[i], xRenderPos,
					scroll + Settings::GetLineSpacing(lineCount), Settings::fontScale,
					textColour.r, textColour.g, textColour.b);

				lli wordSize = splitStringList[i].size();
				if (cursor >= totalCharactersPrinted and cursor <= totalCharactersPrinted + wordSize) {
					lli word_idx = cursor - totalCharactersPrinted;
					RenderRect(xRenderPos + (word_idx * fontX * Settings::fontScale),
						scroll + Settings::GetLineSpacing(lineCount),
						4, fontY * Settings::fontScale, 1, 1, 1);
				}
				totalCharactersPrinted += wordSize;

				xRenderPos += (splitStringList[i].size())
					* fontX * Settings::fontScale;
			}
			break;
		}
		default:
			err("Mode did not match any availible");
			break;
		}
		RenderRect(0, 0, sideBarSize, screeny, 0.5, 0.5, 0.6);
		
		//side bar
		RenderRect(0, 0, sideBarSize, screeny, 0.5, 0.5, 0.6);
		int newLineCount = 0;
		loop(i, 0, text.size()) {
			if (text[i] == '\n') {
				newLineCount++;
			}
		}
		loop(i, 0, newLineCount) {
			RenderText(to_string(i), breakPointSize, Settings::GetLineSpacing(i) + scroll, Settings::fontScale, 0.7, 0.7, 1.0);
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			cursor = max(0, cursor-1);
		}	
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			cursor = min(textLength-1, cursor + 1);
		}

		
		int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		prevMouseState = mouseState;
		RenderRect(mousePos.x, mousePos.y, fontX * 4, fontY * 4, 1, 1, 1);
	}
	void SourceCodeEditor(IDE_Handler::Exception* exception=nullptr) {
		Duration timer;
		string filePath = "code.w";
		bool loadedFile = false;
		string sourceCode = readFileToString(filePath, &loadedFile);
		if (loadedFile != true) {
			err("Failed to load file: " + filePath);
		}
		//remove tabs from source code and replace them with spaces
		sourceCode = strReplace(sourceCode, "\t", Settings::tab);

		while (!glfwWindowShouldClose(window)) {
			timer.start();
			glClear(GL_COLOR_BUFFER_BIT);

			TextEditor(TextMode::SOURCE, sourceCode, exception);

			double time_sec = timer.milliToSeconds(timer.end());
			string title = "FPS: " + to_string(1.0 / time_sec);
			glfwSetWindowTitle(window, title.c_str());
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	vector<vector<bool>> SetBreakPoints(vector<SA::Function>* functionList_ptr) {
		vector<SA::Function>& functionList = $(functionList_ptr);

		vector<vector<bool>> breakPointList;
		breakPointList.resize(functionList.size());
		loop(i, 0, breakPointList.size()) {
			breakPointList[i].resize(functionList[i].instructionList.size());
			loop(j, 0, breakPointList[i].size()) {
				breakPointList[i][j] = false;
			}
		}

		string tabs = "";
		scroll = 0;
		Duration timer;
		//int prevMouseState = 0;
		const int sideBarSize = 90;
		bool shouldClose = false;
		while (!glfwWindowShouldClose(window) and shouldClose == false) {
			timer.start();
			glClear(GL_COLOR_BUFFER_BIT);
			
			RenderText("F5 to execute", screenx - fontX * 14 * Settings::fontScale, 0, Settings::fontScale, 1, 1, 1);
			RenderByteCode(functionList_ptr, &breakPointList);

			if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
				//glfwSetWindowShouldClose(window, null);
				shouldClose = true;
			}

			glfwSwapBuffers(window);			
			glfwPollEvents();
			double time_sec = timer.milliToSeconds(timer.end());
			string title = "FPS: " + to_string(1.0 / time_sec);
			glfwSetWindowTitle(window, title.c_str());
		}
		//glfwTerminate();
		//glfwDestroyWindow(window);
		return breakPointList;
	}

	
	void EnvRender(lli currentInstruction, void* functionList_ptr_void,
		vector<vector<bool>>* breakPoint_ptr)
	{
		vector<SA::Function>* functionList_ptr = (vector<SA::Function>*)functionList_ptr_void;
		vector<SA::Function>& functionList = $(functionList_ptr);
		auto& breakPoint = $(breakPoint_ptr);

		scroll = 0;
		Duration timer;
		int prevMouseState = 0;
		const int sideBarSize = 90;
		while (!glfwWindowShouldClose(window) and glfwGetKey(window, GLFW_KEY_F5) != GLFW_PRESS) {
			timer.start();
			glClear(GL_COLOR_BUFFER_BIT);

			RenderByteCode(functionList_ptr, breakPoint_ptr, currentInstruction);

			glfwSwapBuffers(window);
			glfwPollEvents();
			double time_sec = timer.milliToSeconds(timer.end());
			string title = "FPS: " + to_string(1.0 / time_sec);
			glfwSetWindowTitle(window, title.c_str());
		}
		//glfwTerminate();
		//glfwDestroyWindow(window);
	}
	

	Env::IDEHandler ideHandler;
	void Init() {
		ideHandler.Init = &InitEnv;
		ideHandler.SetBreakPoints = &SetBreakPoints;

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
		IDE_Handler::handler.SourceCodeException = &SourceCodeEditor;
		IDE_Handler::handler.EnvRender = &EnvRender;

		//if (glfwRawMouseMotionSupported())
		//	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		//Never call glfwTerminate()
	}
}