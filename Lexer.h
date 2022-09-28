#pragma once
#include "Operators.h"
namespace Lexer {
	namespace TokenId{
		#define c(...) enum { __VA_ARGS__ };\
			namespace ForCreationOfStringList{\
				string nameList = # __VA_ARGS__ ;\
			}
		c(
			NONE, NAME, CONST, OPP, FN,
			EQU,
			OPENBRACKET, CLOSEBRACKET,
			OPENSQUAREBRACKET, CLOSESQUAREBRACKET,
			OPENSCOPE, CLOSESCOPE,
			COMMA, SEMI_COLON, DOLLAR,
			TYPE, FUNCTION_CALL,
			IF, WHILE, RETURN,
			EXP,
			PRINT_STRING, PRINT_NUMBER,
			INPUT,
			SPACE
		);
		#undef c
		vector<string> tokenNameList;
	}

	//tokens in return or exp that should not come before a comma (to find missing simicolons better)
	vector<lli> illegalTokenList = {
		TokenId::PRINT_STRING, TokenId::PRINT_NUMBER,
		TokenId::OPENSCOPE, TokenId::CLOSESCOPE,
		TokenId::IF, TokenId::WHILE, TokenId::RETURN
	};

	namespace IO {
		vector<string> functionNameList = {"print", "read"};
	}

	byte* StringWritePosition = 0x0;
	struct CharArray {
		byte* ptr;
		string content;		

		CharArray(string s) {
			ptr = StringWritePosition;
			content = s + "\n";
			StringWritePosition += content.size()+1;
		}
	};
	vector<CharArray> charArrayList;

	class Token {
	private:
	public:
		string db_tokenName = "NOT SET";//debug only
		lli id = TokenId::NONE;
		bool failedToInit = false;
		lli lineNumber = sbitMax64;

		//source string used for IDE
		string sourceString;

		//only for type tokens
		lli type = -1;
		lli list_size;
		bool is_list = false;
		bool is_ptr = false;
		//only for opp tokens
		lli operatorBidmasLevel;
		lli operatorIndex;
		//only for name tokens
		string variableName = "";
		//only for const tokens
		ulli constant = (ulli)ubitMax64;
		//only for exp
		lli expIndex;	

		bool is_newLine = false;

		Token() = default;
		Token(lli id) {
			self.id = id;
			db_tokenName = TokenId::tokenNameList[self.id];
		}
		Token(lli id, lli expIndex) {
			self.id = id;
			self.expIndex = expIndex;
			db_tokenName = TokenId::tokenNameList[self.id];
		}

		ulli StringToConst(string s) {
			//TODO add support for double hex and strings
			ulli ret;
			ret = std::stol(s);
			return ret;
		}
		//return jump as combines multiple strings 
		lli SetTokenFromString(vector<string>& stringList, lli idx, lli* lineCount) {
			lli jump = 0;
			string& s = stringList[idx];
			sourceString = s;
			string nextString = "__NULL_STRING__";
			if (idx + 1 < stringList.size()) {
				nextString = stringList[idx + 1];
			}
			lineNumber = $(lineCount);

			if (idx + 1 < stringList.size()) {
				if (stringList[idx + 1] == "\n") {
					is_newLine = true;
					$(lineCount)++;
					jump = 1;
				}
			}

			lli index;
			lli failedReturn = -1;

			//is opp
			index = failedReturn;
			lli bidmas_level;
			lli bidmas_index;
			lli operatorTokenSize = sbitMax64;//number of tokens operator takes up
			loop(l, 0, Operators::bidmasList.size()) {
				loop(i, 0, Operators::bidmasList[l].size()) {
					bool hit = true;					
					loop(charCount, 0, Operators::bidmasList[l][i].name.size()) {
						//err
						if (charCount + idx >= stringList.size()) {
							hit = false;
							break;
						}
						string operatorCharacter = "e";
						operatorCharacter[0] = Operators::bidmasList[l][i].name[charCount];
						if (stringList[idx + charCount] != operatorCharacter) {
							hit = false;
							break;
						}
					}
					if (hit) {
						index++;//so does not equal failed return
						bidmas_level = l;
						bidmas_index = i;
						//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						//assumed that number of characters in operator = number of tokens
						operatorTokenSize = Operators::bidmasList[l][i].name.size();
						sourceString = Operators::bidmasList[l][i].name;
					}
				}
			}
			if (index != failedReturn) {
				id = TokenId::OPP;
				operatorBidmasLevel = bidmas_level;
				operatorIndex = bidmas_index;
#if safe == true
				if (operatorTokenSize == sbitMax64) {
					err("operatorTokenSize not set");
				}
#endif
				jump += operatorTokenSize - 1;//allready jumping 1
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}

#define c(a,b) if(s == a){id = TokenId::b; db_tokenName = TokenId::tokenNameList[self.id]; return jump;}
			c("fn", FN);
			c("[", OPENSQUAREBRACKET);
			c("]", CLOSESQUAREBRACKET);
			c(";", SEMI_COLON);
			c("=", EQU);
			c("{", OPENSCOPE);
			c("}", CLOSESCOPE);
			c("(", OPENBRACKET);
			c(")", CLOSEBRACKET);
			c(",", COMMA);
			c("$", DOLLAR);
			c("if", IF);
			c("while", WHILE);
			c("return", RETURN);
			c(" ", SPACE);
#undef c
			//is IO
			if (s == "prints") {
				id = TokenId::PRINT_STRING;
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			if (s == "printn") {
				id = TokenId::PRINT_NUMBER;
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			if (s == "read") {
				id = TokenId::INPUT;
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			//is type
			index = indexVec<string>(s, typeNameList, failedReturn);
			if (index != failedReturn) {
				id = TokenId::TYPE;
				type = index;
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			//is const
			//simple
			if (is_digits(s) == true) {
				id = TokenId::CONST;
				constant = StringToConst(s);
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			//char*
			if (s[0] == '\"') {
				string charArray;
				charArray += s + " ";				
				//read till end of string
				lli newIdx = idx;
				if (s.back() == '\"') {
					charArray.pop_back();
					goto singleTokenString;
				}
				while (1 == 1) {
					newIdx++;
					jump++;
					if (newIdx >= stringList.size()) {
						err("New Idx out of range could not find end of stirng");
					}
					charArray += stringList[newIdx] + " ";
					if (stringList[newIdx].back() == '\"') {
						charArray.pop_back();//remove last space
						break;
					}
				}
				singleTokenString:
				//removes " 
				charArray.pop_back();
				charArray.erase(0, 1);
				charArrayList.push_back(CharArray(charArray));
				id = TokenId::CONST;
				constant = (ulli)charArrayList.back().ptr;
				db_tokenName = TokenId::tokenNameList[self.id];
				return jump;
			}
			//assumed name
			id = TokenId::NAME;
			if (s == "\n") {failedToInit = true;}
			else {variableName = s;}
			db_tokenName = TokenId::tokenNameList[self.id];
			return jump;
		}
		string printToken(const bool ret = false) {
			string out;
			switch (id) {
			case TokenId::NONE:
				out += "NONE";
				break;
			case TokenId::NAME:
				out += "NAME:" + variableName;
				break;
			case TokenId::CONST:
				out += "CONST:" + to_string(constant);
				break;
			case TokenId::TYPE:
				out += "TYPE:" + to_string(type) + ":" + typeNameList[type];
				{
					if (is_ptr) {
						out += ":PTR";
					}
					if (is_list) {
						out += ":LIST[" + to_string(list_size) + "]";
					}
				}
				break;
			case TokenId::OPP:
				out += "OPP:" + to_string(operatorBidmasLevel) + ":" + to_string(operatorIndex) + ":" + Operators::bidmasList[operatorBidmasLevel][operatorIndex].name;
				break;
			case TokenId::PRINT_STRING:
				out += "PRINT_STRING";
				break;
			case TokenId::PRINT_NUMBER:
				out += "PRINT_NUMBER";
				break;
			case TokenId::INPUT:
				out += "INPUT";
				break;
#define simple(name) case TokenId::name: out += #name; break;
			
			simple(OPENSCOPE);
			simple(CLOSESCOPE);
			simple(OPENBRACKET);
			simple(CLOSEBRACKET);
			simple(COMMA);
			simple(FN);
			simple(EQU);
			simple(SEMI_COLON);
			simple(OPENSQUAREBRACKET);
			simple(CLOSESQUAREBRACKET);
			simple(FUNCTION_CALL);
			simple(EXP);
			simple(DOLLAR);
			simple(IF);
			simple(WHILE);
			simple(SPACE);
			simple(RETURN);
#undef simple
			default:
				err("Cannot print token id does not match any casses: id= " + to_string(id));
				break;
			}
			if (ret == false) {
				print(out);
				return "";
			}
			return out;
		}
	};
	vector<Token> tokenList;

	void Init() {
		//create token name list
		TokenId::ForCreationOfStringList::nameList = strReplace(TokenId::ForCreationOfStringList::nameList, " ", "");
		TokenId::tokenNameList = StringSplit(TokenId::ForCreationOfStringList::nameList, { "," });
	}

	string PrintTokenList(const bool format = true, const bool ret = false, vector<Token>* tokenList_ptr = nullptr) {
		string out = "[\n";
		string indent = "";
		bool just_newLined = false;
		if (tokenList_ptr == nullptr) {
			tokenList_ptr = &tokenList;
		}
		vector<Token>& tokenList = $(tokenList_ptr);
		loop(i, 0, tokenList.size()) {
			if (format == true) {
				if (tokenList[i].id == TokenId::CLOSESCOPE) {
					//check for unequal open and close scopes
					if (indent.size() == 0) {
						err("Unequal count of close scopes and open scopes");
					}
					indent.pop_back();
				}
			}
			if (just_newLined) {
				out += indent;
				just_newLined = false;
			}
			out += tokenList[i].printToken(true);
			if (i != tokenList.size() - 1) { out += ", "; }
			if (format == true) {
				if (tokenList[i].is_newLine == true) {
					out += "\n";
					just_newLined = true;
				}
				if (tokenList[i].id == TokenId::OPENSCOPE) {
					indent += "\t";
				}
			}
		}
		out += "\n]";
		if (ret == false) {
			print(out);
			return "";
		}
		return out;
	}

	struct SplitStringCode_ret {
		vector<string> splitList;
		vector<bool> newLineList;
		vector<int> indentList;
	};
	SplitStringCode_ret SplitStringCode(string fileContent) {
		//vector<string> splitList = Operators::GetOperatorNameList();
		//vector<string> additionalSplit = {" ","\t","\n", ";", "(",")","{","}","[","]"};
		//vector<string> stringList = StringSplit(fileContent, vecConcaternate(splitList, additionalSplit));
		SplitStringCode_ret ret;
		vector<string> splitList = Operators::GetOperatorNameList();
		vector<string> additionalList = {";","(",")","[","]","{","}",",","=","$","\n"};
		splitList = vecConcaternate<string>(splitList, additionalList);

		char SEPARATE_CHAR = 0x1F;

		loop(i, 0, splitList.size()) {
			replaceAll(fileContent, splitList[i], SEPARATE_CHAR + splitList[i] + SEPARATE_CHAR);
		}
		replaceAll(fileContent, "\t", "    ");
		vector<lli> tabCount(1);
		tabCount[0] = 0;
		bool is_newLine = true;
		loop(i, 0, fileContent.size()) {
			if (fileContent[i] == SEPARATE_CHAR) {
				continue;
			}
			if (in(fileContent[i], string(" \t\n")) == false) {
				is_newLine = false;
			}
			if (is_newLine == true) {
				tabCount[tabCount.size() - 1]++;
			}
			if (fileContent[i] == '\n') {
				tabCount.push_back(0);
				is_newLine = true;
			}
		}
		//string split spaces
		vector<string> stringList;// = StringSplit(fileContent, { " " });
		string word = "";
		loop(i, 0, fileContent.size()) {
			if (fileContent[i] == ' ' or fileContent[i] == SEPARATE_CHAR) {
				if (word.size() > 0) {
					stringList.push_back(word);
					word = "";
				}
				if (fileContent[i] == ' ') {
					stringList.push_back(" ");
				}
			}
			else {
				word += fileContent[i];
			}
		}
		if (word.size() > 0) {
			stringList.push_back(word);
		}
		vector<int> indentList(stringList.size());
		vector<bool> newLineList(stringList.size());
		loop(i, 0, indentList.size()) {
			indentList[i] = 0;
			newLineList[i] = false;
		}
		lli found = 0;
		loop(i, 0, stringList.size()) {
			if (stringList[i] == "\n") {
				newLineList[i] = true;
				if (i != stringList.size() - 1) {
					indentList[i + 1] = tabCount[found];
					found++;
				}
			}
		}
		ret.splitList = stringList;
		ret.indentList = indentList;
		ret.newLineList = newLineList;		
		return ret;
	}
	void RemoveSpaceTokens(vector<Token>& tokenList) {
		loop(i, 0, tokenList.size()) {
			if (tokenList[i].id == TokenId::SPACE) {
				tokenList.erase(tokenList.begin() + i);
				i--;
			}
		}
	}
	void RefineTokens(vector<Token>& tokenList) {
		loop(i, 0, tokenList.size()) {
			//function call
			try {
				if (tokenList[i].id == TokenId::NAME) {
					if (tokenList[i + 1].id == TokenId::OPENBRACKET) {
						tokenList[i].id = TokenId::FUNCTION_CALL;
#if safe == true
						tokenList[i].db_tokenName = "FUNCTION_CALL";
#endif
					}
				}
			}
			catch (std::overflow_error& e) {}
			//types
			try {
				if (tokenList[i].id == TokenId::TYPE) {
					bool is_list = false;
					bool is_ptr = false;
					if (tokenList[i + 1].id == TokenId::DOLLAR) {
						tokenList[i].is_ptr = true;
						is_ptr = true;						
					}
					if (tokenList[i + 1 + is_ptr].id == TokenId::OPENSQUAREBRACKET) {
						//TODO array only takes single const not expresion
						if (tokenList[i + 2 + is_ptr].id != TokenId::CONST) {
							err("No const after ");
						}
						tokenList[i].list_size = tokenList[i + 2].constant;
						tokenList[i].is_list = true;
						if (tokenList[i + 3 + is_ptr].id != TokenId::CLOSESQUAREBRACKET) {
							err("No square bracket after list");
						}
						is_list = true;
					}
					//deleting useless tokens
					if (is_ptr) {
						tokenList.erase(tokenList.begin() + i + 1);
					}
					if (is_list) {//assumed size of 3 as exp not taken in list size declaration
						loop(j, 0, 3) {
							tokenList.erase(tokenList.begin() + i + 1);
						}
					}
				}
			}
			catch (std::overflow_error& e) {}
		}
	}
	vector<Token> GenerateTokens(string fileContent, const bool ret=false) {
		SplitStringCode_ret ssc_ret = SplitStringCode(fileContent);
		auto& stringList = ssc_ret.splitList;
		vector<Token> newTokenList;
		lli lineCount = 1;//was 0
		loop(i, 0, stringList.size()) {
			newTokenList.push_back(Token());
			i += newTokenList.back().SetTokenFromString(stringList, i, &lineCount);
			if (newTokenList.back().failedToInit == true) {//remove failed tokens
#if safe == true
				if (newTokenList.size() == 0) {
					err("Error here");
				}
#endif
				newTokenList.pop_back();
			}
		}	
		RemoveSpaceTokens(newTokenList);
		RefineTokens(newTokenList);
		if (ret) {
			return newTokenList;
		}
		else {
			tokenList = newTokenList;
		}
		return vector<Token>();//return empty list
	}
	string ReturnTokenLineForException(vector<Token>& tokenList, lli index) {		
		vector<Token> tokensOnLine;
		lli count = 0;
		//loop backward to find new line
		while (index-count >= 0) {
			if (tokenList[index - count].is_newLine == true) {
				//tokensOnList.push_back(tokenList[index - count]);
				break;
			}
			else {
				tokensOnLine.push_back(tokenList[index - count]);
			}
			count++;
		}
		//loop forward to find end of line
		count = 1;
		while (index + count < tokenList.size()) {
			tokensOnLine.push_back(tokenList[index + count]);
			if (tokenList[index + count].is_newLine == true) {
				break;
			}
			count++;
		}
		//turn tokens into string
		string out;
		fin(i, tokensOnLine) {
			out += i.printToken(true);
		}
		return out;
	}
}