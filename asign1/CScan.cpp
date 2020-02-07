
/****************************************************************
 *                                                              *
 *      Name: Fatema Tabassum Liza                              *
 *      Class: COP5621                                          *
 *      Assignment: Asg 1 (Implementing a C scanner)            *
 *      Compile: "g++ -std=c++11 CScan.cpp"                     *
 *                                                              *
 ****************************************************************/


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <ctype.h>
#include <unistd.h>

using namespace std;

/**************** Preprocessors ******************/

#define TOKENS_OP_DELIMS_SIZE 44
#define TOKENS_OTHER_DEFINITION 7
#define STRING_SIZE 1000
#define NULL_EMPTY_STRING ""
#define QUOTE "'"
#define DITTO "\""
#define BACK "\\"
#define FORWARDSLASH "/"
#define NEWLINE "\n"
#define IDENTIFIER "ident"
#define CHAR "char"
#define STRING "string"
#define DIGITS "number"
#define KEYWORD "keyword"
#define INVALID "invalid"



/************   Global Variables and Arrays   *****************/

string tokens_op_delim_definition[TOKENS_OP_DELIMS_SIZE] = {"+", "-", "/", "*", "=", "%", "<", ">", "!", "~", "&", "|", "^",
    "<=", ">=", "!=", "++", "--", "+=", "-=", "*=", "/=", "^=","<<", ">>", "&&", "||", "->",
    "<<=", ">>=", "&&=", "||=", "==", ".", ",", ":", ";", "(", ")", "?", "{", "}", "[", "]"};

string tokens_other_definition[TOKENS_OTHER_DEFINITION] = { IDENTIFIER, CHAR, STRING, DIGITS, KEYWORD, INVALID, FORWARDSLASH};


map < string, int > tokenTypeMap ;
vector < pair <string, int> > tokenTypeVec;
int programLineCount = 0;
bool isOneLineComment = false;
bool isMultiLineComment = false;
string inputLine;

int leftPosition = 0;
int positionNow = 0;
bool isNumberFlag = false;
bool isAlphaFlag = false;


enum TokenTypeColor {
    ISALPHA,
    ISDIGIT,
    ISQUOTE,
    ISDITTO,
    ISINVALID
};



/**************     Function Prototypes    ********************/

void populateTokenTypeMap();
void setPositionNow (int r);
void setLeftPosition (int l);
void setDidDigitStart (bool b);
bool checkIfCommentStarting ( int positionNow );
bool checkIfCommentEnding ( int positionNow );
void setIsMultiLineComment (bool var);
void setIsOneLineComment (bool var);
bool isDelimiter(string s);
string subString(int left, int len);
bool isOperatorOrDelimter(string s);
bool isDitto(string s);
TokenTypeColor checkAndEvaluateCharTypes(string now, int left, int right);
void checkAndEvaluateDoubleDitto(int left, int right);
void checkAndEvaluateDoubleQuote (int left, int right);
void checkAndEvaluateIdentifier(int left, int right);
void checkAndEvaluateDigits(int left, int right);
bool checkAndEvaluateOperatorsLength(int left, int right);
void printError(char *str);
void printTokenCount();
bool sortByVal(const pair<string, int> &a,
               const pair<string, int> &b);
void sortMapInDescendingOrder();
void performOpForPrevNumberOrDigit();


/***********  main    ************/

int main(int argc, const char * argv[]) {
    
    populateTokenTypeMap();
    programLineCount = 0;
    
    while (getline(cin, inputLine)) {
        
        tokenTypeVec.clear();
        programLineCount++;
        setPositionNow(0);
        setLeftPosition(0);
        
        while (positionNow <= inputLine.size()) {
            
            if (checkIfCommentEnding(positionNow)) {
                setIsMultiLineComment(false);
                setLeftPosition(positionNow);
                continue;
            }
            if (isMultiLineComment) {
                setPositionNow(positionNow+1);
                setLeftPosition(positionNow);
                continue;
            }
            if(checkIfCommentStarting(positionNow)) {
                if (isOneLineComment) {
                    isOneLineComment = false;
                }
                continue;
            }
            string now = subString (positionNow, 1);
            if (isDelimiter(now) == false) {
                TokenTypeColor type = checkAndEvaluateCharTypes( now, leftPosition, positionNow);
                if (type == ISDITTO) {
                    performOpForPrevNumberOrDigit();
                    checkAndEvaluateDoubleDitto( leftPosition, positionNow);
                } else if (type == ISQUOTE) {
                    performOpForPrevNumberOrDigit();
                    checkAndEvaluateDoubleQuote(leftPosition, positionNow);
                } else if (type == ISALPHA) {
                    isAlphaFlag = true;
                    if (isNumberFlag) {
                        checkAndEvaluateDigits(leftPosition, --positionNow);
                        isNumberFlag = false;
                        setPositionNow(positionNow + 1);
                        setLeftPosition(positionNow);
                    }
                    else {
                        setPositionNow(positionNow + 1);
                    }
                    
                } else if (type == ISDIGIT) {
                    if (isAlphaFlag == true) {
                        // already alpha is set. so this must be an ident
                    } else {
                        isNumberFlag = true;
                    }
                    setPositionNow(positionNow + 1);

                }

            } else if (isDelimiter(now) == true && leftPosition != positionNow) {
                bool alpha = false; bool digit = false;
                if (isNumberFlag == true && isAlphaFlag == true) {
                    alpha = true;
                } else if (isNumberFlag == false && isAlphaFlag == true) {
                    alpha = true;
                } else if (isNumberFlag == false && isAlphaFlag == false) {
                    //cout << "empty isNumberFlag" << endl;
                } else if (isNumberFlag == true &&  isAlphaFlag == false) {
                    digit = true;
                }
                
                if (alpha) {
                    checkAndEvaluateIdentifier(leftPosition, --positionNow);
                    isAlphaFlag = false;
                } else if(digit){
                    checkAndEvaluateDigits(leftPosition, --positionNow);
                    isNumberFlag = false;
                }
                setPositionNow(positionNow + 1);
                setLeftPosition(positionNow);
                
            } else if (isDelimiter(now) == true && leftPosition == positionNow) {
                
                if (isspace(now[0]) || now[0] == '\0' || inputLine[positionNow] == 0) {
                    setPositionNow(positionNow+1);
                    setLeftPosition(positionNow);
                }
                else {
                    checkAndEvaluateOperatorsLength(leftPosition, positionNow);
                }
            }
     
            
        }
    }
    sortMapInDescendingOrder();
    printTokenCount();
    return 0;
}

/***********  setter methods    ************/


void setPositionNow(int r) {
    positionNow = r;
}
void setLeftPosition(int l) {
    leftPosition = l;
}

void setIsOneLineComment(bool var) {
    isOneLineComment = var;
}

void setIsMultiLineComment(bool var) {
    isMultiLineComment = var;
}


/***********  check Comments    ************/

bool checkIfCommentStarting(int positionNow) {
    if (!inputLine.empty() && inputLine.size() > positionNow) {
        if (inputLine[positionNow] == '/' &&
            (positionNow + 1) < inputLine.size()) {
            if (inputLine[positionNow + 1] == '/') {
                setIsOneLineComment (true);
                setPositionNow((int)inputLine.size()+1);
                return true;
            } else if (inputLine[positionNow + 1] == '*') {
                setIsMultiLineComment (true);
                setPositionNow (positionNow + 2);
                return true;
            } else {
                return false;
            }
        }
        return false;
    }
    return false;
}

bool checkIfCommentEnding(int positionNow) {
    if (!inputLine.empty() && inputLine.size() > positionNow) {
        if (inputLine[positionNow] == '*' &&
            (positionNow + 1) < inputLine.size()) {
            if (inputLine[positionNow + 1] == '/') {
                setPositionNow (positionNow + 2);
                return true;
            } else {
                return false;
            }
        }
        return false;
    }
    return false;
}


bool isOperatorOrDelimter(string s) {
    for (int i = 0; i < TOKENS_OP_DELIMS_SIZE; i++) {
        if (tokens_op_delim_definition[i].compare(s) == 0) {
            return true;
        }
    }
    return false;
}

bool isDelimiter(string s) {
    if (s[s.size() - 1] == '\0' ) {
        return true;
    } else if (isOperatorOrDelimter(s)) {
        return true;
    } else if (s.size() == 1 && isspace(s[0])) {
        return true;
    }
    return false;
    
}

string subString(int left, int len) {
    return inputLine.substr(left, len);
    
}

bool isDitto(string s) {
    if (s.compare(DITTO) == 0) {
        return true;
    }
    return false;
}

bool isQuote(string s) {
    if (s.compare(QUOTE) == 0) {
        return true;
    }
    return false;
}

bool checkIfValidString(string str) {
    bool flagValidStr = true;
    for(char c: str) {
        if (isascii(c)) {
            continue;
        } else {
            flagValidStr = false;
        }
    }
    return flagValidStr;
}

/***********  print error   ************/

void printError(char *str) {
    printf("%s on line %d\n", str, programLineCount);
}

/***************   checks valid token types  ****************/

TokenTypeColor checkAndEvaluateCharTypes(string now, int left, int right) {
    
    if (isalpha(now[0]) || now[0] == '_') {
        return ISALPHA;
    }
    else if (isdigit(now[0])) {
        return ISDIGIT;
    }
    else if (isDitto(now)) {
        return ISDITTO;
        
    } else if (isQuote(now)) {
        return ISQUOTE;
    } else {
        
        char buffer[100];
        sprintf (buffer, "illegal character: %s", now.c_str());
        printError(buffer);
        setPositionNow(positionNow  + 1);
        setLeftPosition(positionNow);
        return ISINVALID;

    }
}

/***********  check valid string inside dittos    ************/

void checkAndEvaluateDoubleDitto(int left, int right) {
    int temp_right = right + 1;
    bool flag_ditto_string_ditto = false;
    while (temp_right < inputLine.size() ) {

        string st = inputLine.substr(right + 1, temp_right - right);
        
        if(!st.compare(BACK)){
            temp_right++;
            string st2 = inputLine.substr(right + 2, temp_right - right);
            if (st.compare(QUOTE) == 0 || st.compare( BACK) == 0 || st.compare(DITTO) == 0)  {
                temp_right++;
                continue;
            }
        }
        
        
        if (inputLine.compare(temp_right,1,"\"") == 0) {
            if(checkIfValidString(inputLine.substr(right + 1, temp_right - right - 1))) {
                tokenTypeMap[STRING]++;
                cout << inputLine.substr(left, temp_right - right + 1) << endl;
            }
            flag_ditto_string_ditto = true;
            break;
        }
        temp_right++;
    }
    if (flag_ditto_string_ditto) {
        setPositionNow(temp_right + 1);
        setLeftPosition(positionNow);
    } else {
        string ss = "missing \" for \"";
        printError((char *)ss.c_str());
        setPositionNow((int)inputLine.size());
        setLeftPosition(positionNow);
    }
    
}

/***********  check valid char inside quotes    ************/


void checkAndEvaluateDoubleQuote (int left, int right) {
    int temp_right = right + 1;
    bool flag_quote_char_quote = false;
    bool isvalidChar = false;
    while (temp_right < inputLine.size() ) {
        
        string st = inputLine.substr(right + 1, temp_right - right);
        
        if(!st.compare(BACK)){
            temp_right++;
            string st2 = inputLine.substr(right + 2, temp_right - right);
            if (st.compare(QUOTE) == 0 || st.compare( BACK) == 0 || st.compare(DITTO) == 0)  {
                temp_right++;
                continue;
            }
        }
        
        if (inputLine.compare(temp_right,1,"\'") == 0) {
            
            string st = inputLine.substr(right + 1, temp_right - right - 1);
            if (st.size() == 2 && st[0] == '\\') {
                isvalidChar = true;
                
            } else if (st.size() == 1) {
                if(isascii(st[0])){
                    isvalidChar = true;
                }
            } else if(st.size() == 0) {
                string st = "character has zero length";
                printError((char *)(st.c_str()));
                setPositionNow(temp_right + 1);
                setLeftPosition(positionNow);
                return;
            } else if(st.size() >= 2) {
                char buffer[100];
                sprintf (buffer, "character constant %s too long", inputLine.substr(right, temp_right+1).c_str());
                printError(buffer);
                setPositionNow(temp_right + 1);
                setLeftPosition(positionNow);
                return;
            }
           
            flag_quote_char_quote = true;
            
            if (isvalidChar) {
                tokenTypeMap[CHAR]++;
                cout << inputLine.substr(left, temp_right - right + 1) << endl;
            }
            
            break;
        }
        temp_right++;
    }
    if (flag_quote_char_quote) {
        setPositionNow(temp_right + 1);
        setLeftPosition(positionNow);
    } else {
        string ss = "missing \' for \'";
        printError((char *)ss.c_str());
        setPositionNow((int)inputLine.size());
        setLeftPosition(positionNow);
    }
}

/***********   check identifier  *************/

void checkAndEvaluateIdentifier(int left, int right) {
    bool flag_identifier = true;
    
    if (isalpha(inputLine[left]) || inputLine[left] == '_') {
        for (int i = left + 1; i <= right; i++) {
            if (!(isalpha(inputLine[i]) || isdigit(inputLine[i]) || inputLine[i] == '_')) {
                flag_identifier = false;
            }
        }
    } else {
        flag_identifier = false;
    }
    if (flag_identifier == true) {
        tokenTypeMap[IDENTIFIER]++;
        cout << inputLine.substr(left, right - left + 1) << endl;
        setPositionNow(right);
        setLeftPosition(positionNow);
    } else {
        
    }
}

/************** checks valid digits ******************/

void checkAndEvaluateDigits(int left, int right) {
    bool flag_digits = true;
    
    
    for (int i = left; i <= right; i++) {
        if(!isdigit(inputLine[i])) {
            flag_digits = false;
        }
    }
    if (flag_digits == true) {
        tokenTypeMap[DIGITS]++;
        cout << inputLine.substr(left, right - left + 1) << endl;
        setPositionNow(right);
        setLeftPosition(positionNow);
    }
    else {
        
    }
}


/***********    checks 1 or 2 or 3 lengths operators and Delimiters   *************/

bool checkAndEvaluateOperatorsLength(int left, int right) {
    int temp_right = right;
    int mask = 0;
    int jj = 0;
    bool flag = false;
    string subStr;
    while (temp_right < inputLine.size()) {
        string subStr = subString(left, temp_right - right + 1);
        if (isOperatorOrDelimter(subStr)) {
            mask = mask | (1 << jj);
        }
        temp_right++;
        jj++;
        if (jj > 3) {
            break;
        }
    }
    if (mask & (1 << 2)) {
        subStr = subString(left, 3);
        tokenTypeMap[subStr]++;
        setPositionNow(positionNow + 3);
        flag = true;
        
    } else if(mask & (1 << 1) ) {
        subStr = subString(left, 2);
        tokenTypeMap[subStr]++;
        setPositionNow(positionNow + 2);
        flag = true;
        
    } else if(mask & 1) {
        subStr = subString(left, 1);
        tokenTypeMap[subStr]++;
        setPositionNow(positionNow + 1);
        flag = true;
        
    }
    if (flag) {
        setLeftPosition(positionNow);
        cout << subStr << endl;
    }
    return false;
}

/***********    populate the map with (key,value) pairs    ***********/

void populateTokenTypeMap() {
    for (int i = 0; i < TOKENS_OP_DELIMS_SIZE; i++) {
        tokenTypeMap.insert(pair<string, int>(tokens_op_delim_definition[i], 0));
    }
    for (int i = 0; i < TOKENS_OTHER_DEFINITION; i++) {
        tokenTypeMap.insert(pair<string, int>(tokens_other_definition[i], 0));
    }
}


/**************  Sort and Print Map according to value  ***********/

void sortMapInDescendingOrder() {
    map<string, int> :: iterator it2;
    for (it2 = tokenTypeMap.begin(); it2 != tokenTypeMap.end(); it2++)
    {
        tokenTypeVec.push_back(make_pair(it2->first, it2->second));
    }
    sort(tokenTypeVec.begin(), tokenTypeVec.end(), sortByVal);
}

/***********   sort compare function    ***********/

bool sortByVal(const pair<string, int> &a,
               const pair<string, int> &b) {
    if (a.second == b.second) {
        if (a.first.size() == b.first.size()) {
            return (a.first < b.first);
        }
        return (a.first.size() > b.first.size());
    }
    return (a.second > b.second);
}


/*********  print tokens types and counts  ***************/

void printTokenCount() {
    printf("\n");
    printf("%13s%15s\n", "token", "count");
    printf("---------------------%2s-----\n", " ");
    for (int i = 0; i < tokenTypeVec.size(); i++) {
        if (tokenTypeVec[i].second) {
            printf("%21s%7d\n", tokenTypeVec[i].first.c_str(), tokenTypeVec[i].second);
        }
    }
}

void performOpForPrevNumberOrDigit() {
    if (isNumberFlag) {
        checkAndEvaluateDigits(leftPosition, --positionNow);
        isNumberFlag = false;
        setPositionNow(positionNow + 1);
        setLeftPosition(positionNow);
    }
    if (isAlphaFlag) {
        checkAndEvaluateIdentifier(leftPosition, --positionNow);
        isAlphaFlag = false;
        setPositionNow(positionNow + 1);
        setLeftPosition(positionNow);
    }
}

