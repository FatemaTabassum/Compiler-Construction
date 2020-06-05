/****************************************************************
 *                                                              *
 *      Name: Fatema Tabassum Liza                              *
 *      Class: COP5621                                          *
 *      Assignment: Asg 6                                       *
 *      Compile: "g++ -std=c++11 cgen.cpp -o cgen"              *
 *                                                              *
 ****************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <map>

using namespace std;

/*
* Macros
*/
# define INPUT_SIZE 110
# define ACTVATION_RECORD_INIT_SIZE 104
# define FORMAL_INIT_SIZE 68
# define TEMPORARY_SPACE_START 96
# define LOCALALLOC "localloc"
# define IS_STR  1
# define IS_FUNC 2
# define IS_LOCAL 3
# define IS_FORMAL 4
# define IS_BEGINSTMT 5
# define IS_CONST 6
# define IS_ASSIGNMENT 7
# define IS_CONVERSION 8
# define IS_DEREFERENCE 9
# define IS_ARGUMENT 10
# define IS_FUNC_TO_CALL 11
# define IS_CALL 12
# define IS_OP_AND_SET 13
# define IS_LOCAL_ALLOC 14
# define IS_PARAM 16
# define IS_FEND 17
# define IS_SYMBOL 18
# define IS_LABEL 19
# define IS_RELATIONAL 20
# define IS_BRANCH_UNCOND 21
# define IS_BRANCH_COND 22
# define IS_RETURN 23
# define IS_ARRAY 24

/*
* Globals *
*/
int totalSizeLocalAlloc = 0;
int localReg = 0;
int floatingPointReg = 0;
int outputReg = 0;
int prevLocalAlloc = 0;
int prevFormal = 0;
int formalCnt = 0;
int space = 1;
bool isReturnPresent = false;

bool didCountLocals = false;
bool isLocalAlloc = false;
bool isFormalAlloc = false;

vector<string> inputLines;
vector<int> localVarList;
vector<int> formalVarList;
map<string,string> reg_mapping;
vector<string> arguments_list;
vector<string>relational_list;

enum dataType {
    doubleType,
    intType
};

/*
 * Function Prototype *
 */
void processBranchCond(string str);
void processLocalAlloc(string str) ;
void processFormal(string str);
void processRelational(string str);
void processParam (string str);
void saveArgument(string str);
void processArgument (string str) ;
void processInput(string str);
void processConversion(string str);
void processConst(string str);
void processAssignment(string str);
void processDereference(string str);
void processOpAndSet(string str);
void processCall(string str);
void processFuncToCall (string str);
void processFunc(string str);
void processLocalAlloc(string str);
void processLabel(string str);
void printSpace();

/*
 * utility functions *
 */
int currentFloatingPointReg() {
    return floatingPointReg;
}

void increaseFloatingPointReg() {
    floatingPointReg += 2;
}
void decreaseFloatingPointReg() {
    floatingPointReg -= 2;
}

int currentOutputReg() {
    return outputReg;
}

void increaseOutputReg() {
    outputReg++;
}
void decreaseOutputReg() {
    outputReg--;
}

int currentLocalReg() {
    return localReg;
}

void increaseLocalReg() {
    localReg++;
}
void decreaseLocalReg() {
    localReg--;
}

void resetLocalRegister() {
    localReg = 0;
}

void resetfloatingPointReg() {
    floatingPointReg = 0;
}

void resetOutputReg() {
    outputReg = 0;
}

void clearDataForPrevStmt() {
    reg_mapping.clear();
    resetLocalRegister();
    resetfloatingPointReg();
    resetOutputReg();
}

void processReturn(string str) {
    // reti t12
    isReturnPresent = true;
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, tp;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 1) {
            temp1 = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    cout << "ret %" << reg_mapping[temp1] << endl;
}

void processArray(string str) {
//    t22 := t21 []f t20
    stringstream ss;
    string temp;
    ss << str;
    dataType type = intType;
    string temp1, temp2, temp3, opr;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 2) {
            temp2 = temp;
        } else if (i == 3) {
            temp3 = temp.substr(temp.size()-2, temp.size()-1);
            opr = temp;
            if (temp3.compare("i")==0) {
                type = intType;
            } else {
                type = doubleType;
            }
        } else if (i == 4) {
            temp3 = temp;
        }
        i++;
        temp = "";
    }
    string rr;
    if (type == intType) {
        rr = "l" + to_string(currentLocalReg());
        increaseLocalReg();
    } else {
        rr = "f" + to_string(currentFloatingPointReg());
        increaseFloatingPointReg();
    }
    reg_mapping[temp1] = rr;
}

/*
 * Printing fixed space *
 */
void printSpace() {
    printf("%4s", " ");
}

bool checkIfaString(std::string str, char delim = ' ') {
    int cnt = 0;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '\"') {
            cnt++;
        }
    }
    if (cnt == 2) {
        return true;
    }
    return false;
}

/*
 * function to process each individual operation*
 */

void processFend() {
    if (!isReturnPresent) {
        printSpace();
        cout << "ret" << endl;
    }
    printSpace();
    cout << "restore" << endl;
    resetLocalRegister();
    resetfloatingPointReg();
    localVarList.clear();
    formalVarList.clear();
    reg_mapping.clear();
    arguments_list.clear();
    formalCnt = 0;
}


void processSymbol(string str) {
    printSpace();
    cout << str << endl;
}

void processBranchCond(string str) {
    //bt t13 B3
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, tp;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 1) {
            temp1 = temp;
        } else if (i == 2) {
            temp2 = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    if (temp1.compare("<i")) {
        cout << "bl " << temp2;
    } else if (temp1.compare("<f")) {
        cout << "fbl " << temp2;
    }else if (temp1.compare("<=i")) {
        cout << "ble " << temp2;
    }else if (temp1.compare("<=f")) {
        cout << "fble " << temp2;
    }else if (temp1.compare(">i")) {
        cout << "bg " << temp2;
    }else if (temp1.compare(">f")) {
        cout << "fbg " << temp2;
    }else if (temp1.compare(">=i")) {
        cout << "bge " << temp2;
    }else if (temp1.compare(">=f")) {
        cout << "fbge " << temp2;
    }else if (temp1.compare("==i")) {
        cout << "be " << temp2;
    }else if (temp1.compare("==f")) {
        cout << "fbe " << temp2;
    }else if (temp1.compare("!=i")) {
        cout << "bne " << temp2;
    }else if (temp1.compare("!=f")) {
        cout << "fbne " << temp2;
    }
    cout << endl;
    printSpace();
    cout << "nop" << endl;
}

void processBranchUncond(string str) {
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, temp3;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 1) {
            temp1 = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    cout << "ba " << temp1 << endl;
    printSpace();
    cout << "nop" << endl;
}

void processRelational(string str) {
    //t13 := t11 <i t12
    stringstream ss;
    string temp;
    ss << str;
    dataType type = intType;
    string temp1, temp2, temp3, opr;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 2) {
            temp2 = temp;
        } else if (i == 3) {
            temp3 = temp.substr(temp.size()-2, temp.size()-1);
            opr = temp;
            if (temp3.compare("i")==0) {
                type = intType;
            } else {
                type = doubleType;
            }
        } else if (i == 4) {
            temp3 = temp;
        }
        i++;
        temp = "";
    }
    //cmp %l0, 0
    printSpace();
    if (type == intType) {
        cout << "cmp %" <<reg_mapping[temp2] << ",%" << reg_mapping[temp3] << endl;
    } else {
        cout << "fcmpd %" <<reg_mapping[temp2] << ",%" << reg_mapping[temp3] << endl;
    }
    reg_mapping[temp1] = opr;
}

void processLabel(string str) {
    int i = 0;
    stringstream ss;
    ss << str;
    string temp, tempStr;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 1) {
            tempStr = temp;
        }
        temp = "";
        i++;
    }
    cout << tempStr << ":" << endl;
}



bool checkIfaLocalAlloc(std::string str, char delim = ' ') {
    int i = 0;
    std::stringstream ss(str);
    std::string token;
    bool flag = false;
    while (std::getline(ss, token, delim)) {
        if(i == 0 && token.compare(LOCALALLOC) == 0) {
            flag = true;
        } else if (flag  && i == 1) {
            totalSizeLocalAlloc += atoi(token.c_str());
        }
        i++;
    }
    return flag;
}


void processStr(const std::string str, const std::string delims = ":=") {
    
    printSpace();
    printf("%*s",space, ".seg" );
    //std::cout << ".seg";
    cout << " " << "\"data\"" << std::endl;
    std::stringstream ss(str);
    string tempStr, tempVal, strVal;
    int i = 0;
    std::string temp;
    std::size_t current, previous = 0;
    current = str.find_first_of(delims);
    while (current != std::string::npos) {
        temp = str.substr(previous, current - previous);
        if (i == 0) {
            tempVal = temp.substr(0, temp.size()-1);
            tempStr = "LS" + temp.substr (1,temp.size()-2);
            cout << tempStr << ":" << endl;
        }
        i++;
        previous = current + 1;
        current = str.find_first_of(delims, previous);
    }
    strVal  = str.substr(previous+1, current - previous);
    
    printSpace();
    printf("%*s",space, ".asciz" );
    cout << " " << strVal << endl;
    printSpace();
    printf("%*s",space, ".seg " );
    //cout << ".seg ";
    cout << "\"" << "text" << "\"" << endl;
    printSpace();
    printf("%*s",space, "sethi " );
    //cout << "sethi ";
    cout << "%hi(" << tempStr << ")" << "," << "%l" << currentLocalReg() << endl;
    printSpace();
    printf("%*s",space, "or " );
    //cout << "or ";
    cout << "%l" << currentLocalReg() << "," << "%lo(" << tempStr << ")" << "," << "%l" << currentLocalReg() << endl;
    reg_mapping[tempVal] = "l" + to_string(currentLocalReg());
    
}


void processLocalAlloc(string str) {
    stringstream ss;
    ss << str;
    int i = 0;
    string temp, tempStr;
    int size_ = 0;
    while (!ss.eof()) {
        /* extracting word by word from stream */
        ss >> temp;
        if (i == 1) {
            stringstream(temp) >> size_;
        }
        temp = "";
        i++;
    }
    localVarList.push_back(ACTVATION_RECORD_INIT_SIZE + prevLocalAlloc);
    prevLocalAlloc = size_;
    totalSizeLocalAlloc += size_;
}


void processFunc(string str) {
    resetOutputReg();
    int i = 0;
    stringstream ss;
    ss << str;
    string temp, tempStr;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 1) {
            tempStr = temp;
        }
        temp = "";
        i++;
    }
    printSpace();
    printf("%*s",space, ".seg \"text\"\n" );
    printSpace();
    printf("%*s %s\n", space, ".global", tempStr.c_str());
    //printf(".global %s\n", tempStr.c_str());
    printf("%s:\n", tempStr.c_str());
    isReturnPresent = false;
}


void processParam (string str) {
    string str2, temp, str1;
    int paramno = 0;
    stringstream ss;
    ss << str;
    int found;
    int i = 0;
    string tempStr;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            tempStr = temp;
        }
        if (stringstream(temp) >> found) {
            paramno = found;
        }
        temp = "";
        i++;
    }
    //t36 := param 0
    //add %fp,68,%l1
    //ld [%l1],%l2
    reg_mapping[tempStr] = "l" + to_string(currentLocalReg());
    printSpace();
    printf("%*s", space, "add ");
    //cout << "add";
    cout << "%" << "fp" << "," << formalVarList[paramno] << ",%" << "l" << currentLocalReg() << endl;
    
}

void processLocal (string str) {
    string str2, temp, str1;
    int localno = 0;
    stringstream ss;

        /* Storing the whole string into string stream */
    ss << str;
    int found;
    int i = 0;
    string tempStr;
    while (!ss.eof()) {
        /* extracting word by word from stream */
        ss >> temp;
        if (i == 0) {
            tempStr = temp;
        }
        /* Checking the given word is integer or not */
        if (stringstream(temp) >> found) {
            localno = found;
        }
        /* To save from space at the end of string */
        temp = "";
        i++;
    }
    reg_mapping[tempStr] = "l" + to_string(currentLocalReg());
    printSpace();
    printf("%*s", space, "add ");
    //cout << "add";
    cout << "%" << "sp" << "," << localVarList[localno] << ",%" << "l" << currentLocalReg() << endl;
    
}

void processConst(string str) {
    string str2, temp;
    int localno = 0;
    bool isNeg = false;
    int found;
    stringstream ss;
    ss << str;
    int i = 0;
    string tempStr;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            tempStr = temp;
        }
        if (stringstream(temp) >> found) {
            localno = atoi(temp.c_str());
        } else if(temp.compare("-") == 0) {
            isNeg = true;
        }
        temp = "";
        i++;
    }
    if (isNeg) {
        localno = -localno;
    }
    //mov 10,%l1
    printSpace();
    printf("%*s", space, "mov ");
    //cout << "mov";
    cout << localno << "," << "%" << "l" << currentLocalReg() << endl;
    reg_mapping[tempStr] = "l" + to_string(currentLocalReg());
    return ;
}

void processAssignment(string str) {
    stringstream ss;
    string temp;
    ss << str;
    string dest_address, src_reg;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 2) {
            dest_address = reg_mapping[temp];
        } else if (i == 4) {
            src_reg = reg_mapping[temp];
        }
        i++;
        temp = "";
    }
    printSpace();
    printf("%*s", space, "st");
    //cout << "st";
    if (src_reg[0] == 'f') {
        cout<<"d";
    }
    cout << " %" << src_reg << "," << "[" << "%" << dest_address << "]" << endl;
}

void saveArgument(string str) {
    arguments_list.push_back(str);
}

void processArgument (string str) {
    int temporaryMem = 0;
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, convSt;
    dataType dType = intType;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
            if(temp[temp.size()-1] == 'f')
                dType = doubleType;
            else dType = intType;
        } else if (i == 1) {
            temp2 = temp;
        }
        i++;
        temp = "";
    }
    if (dType == intType) {
        printSpace();
        cout << "mov %" << reg_mapping[temp2] << ",%o" << currentOutputReg() << endl;
        increaseOutputReg();
    } else if (dType == doubleType) {
        printSpace();
        cout << "std ";
        cout << "%" << reg_mapping[temp2] << "," << "[" << "%sp + " << TEMPORARY_SPACE_START + temporaryMem * 4 << "]" << endl;
        
        printSpace();
        cout << "ld ";
        cout << "[" << "%sp + " << TEMPORARY_SPACE_START + temporaryMem * 4 << "]" << ",%" << "o" << currentOutputReg() << endl;
        increaseOutputReg();
        printSpace();
        temporaryMem++;
        cout << "ld ";
        cout << "[" << "%sp + " << TEMPORARY_SPACE_START + temporaryMem * 4 << "]" << ",%" << "o" << currentOutputReg() << endl;
        increaseOutputReg();
    }
}



void processFuncToCall (string str) {
    //t11 := global printf
    stringstream ss;
    string temp;
    ss << str;
    string temp1, fname, convSt;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 3) {
            fname = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    printf("%*s", space, "sethi ");
    //cout << "sethi ";
    cout << "%hi(" << fname << ")" << "," << "%l" << currentLocalReg() << endl;
    printSpace();
    printf("%*s", space, "or ");
    //cout << "or ";
    cout << "%l" << currentLocalReg() << "," << "%lo(" << fname << ")" << "," << "%l" << currentLocalReg() << endl;
    reg_mapping[temp1] = "l" + to_string(currentLocalReg());
    for (int i = 0; i < arguments_list.size(); i++) {
        processArgument(arguments_list[i]);  // need to test
    }
    arguments_list.clear();
}


void processCall(string str) {
    stringstream ss;
    //t17 := fi t16 2
       // call %l2,3
    string temp;
    ss << str;
    string temp1, tempVal, convSt;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 3) {
            tempVal = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    printf("%*s", space, "call ");
    //cout << "call ";
    cout << "%" << reg_mapping[tempVal] << "," << currentOutputReg() << endl;
    printSpace();
    printf("%*s", space, "nop");
    cout << endl;
    arguments_list.clear();
    //cout << "nop" << endl;
}

void processFormal(string str) {
    //formal 4
    //formal 8
    
    stringstream ss;
    string temp;
    ss << str;
    string temp1;
    int size_ = 0;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 1) {
            stringstream(temp) >> size_;
        }
        i++;
        temp = "";
    }
    int k = size_ / 4;

    for (i = 0; i < k; i++) {
        int sz = FORMAL_INIT_SIZE + prevFormal;
        printSpace();
        cout << "st %i" << formalCnt << ",[%fp + " << sz << "]" << endl;
        formalVarList.push_back(sz);
        prevFormal = size_;
        formalCnt++;
        //totalSizeLocalAlloc += size_;
    }
}

void processOpAndSet(string str) {
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, op, temp3, type;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if (i == 2) {
            temp2 = temp;
        } else if (i == 3) {
            op = temp.substr(0, temp.size()-1);
            type = temp.substr(temp.size()-1,temp.size()-1 );
        } else if (i == 4) {
            temp3 = temp;
        }
        i++;
        temp = "";
    }
    
    if (!op.compare("+")) {
        //add %l2,%l1,%l2
        if (!type.compare("i")) {
            
            printSpace();
            printf("%*s", space, "add ");
            //cout << "add ";
            cout << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        } else if(!type.compare("f")) {
            
            printSpace();
            printf("%*s", space, "faddd ");
            //cout << "faddd ";
            cout << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        }
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("-")) {
        
        if (!type.compare("i")) {
            
            printSpace();
            cout << "sub " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        } else if(!type.compare("f")) {
            
            printSpace();
            cout << "fsubd " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        }
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("/")) {
        
        if (!type.compare("i")) {
            
            printSpace();
            cout << "sdiv " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        } else if(!type.compare("f")) {
            
            printSpace();
            cout << "fdivd " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
            
        }
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("*")) {
        
        if (!type.compare("i")) {
            
            printSpace();
            cout << "smul " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2]  << endl;
            
        } else if(!type.compare("f")) {
            
            printSpace();
            cout << "fmuld " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2]  << endl;
            
        }
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("%")) {
        if (!type.compare("i")) {
            cout << "smod " << "%" << reg_mapping[temp2] << "," << "%" <<
            reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
        }
    } else if (!op.compare("<<")) {
        
        printSpace();
        cout << "sll " << "|" << reg_mapping[temp2] << "," << "%" <<
        reg_mapping[temp3] << "%" << reg_mapping[temp2] << endl;
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare(">>")) {
        
        printSpace();
        cout << "srl " << "|" << reg_mapping[temp2] << "," << "%" <<
        reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("^")) {
        
        printSpace();
        cout << "xor " << "|" << reg_mapping[temp2] << "," << "%" <<
        reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("&")) {
        
        printSpace();
        cout << "and " << "&" << reg_mapping[temp2] << "," << "%" <<
        reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
        reg_mapping[temp1] = reg_mapping[temp2];
        
    } else if (!op.compare("|")) {
        
        printSpace();
        cout << "or " << "|" << reg_mapping[temp2] << "," << "%" <<
        reg_mapping[temp3] << "," << "%" << reg_mapping[temp2] << endl;
        reg_mapping[temp2] = "l" + to_string(currentLocalReg());
        reg_mapping[temp1] = reg_mapping[temp2];
    }
}

void processDereference(string str) {
    /*t10 := @i t9
     

    ld [%l1],%l2
    sethi %hi(printf),%l3
     or %l3,%lo(printf),%l3

     */
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, convSt;
    dataType dType = intType;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if ( i == 2) {
            if(temp[temp.size()-1] == 'f')
                dType = doubleType;
            else dType = intType;
        } else if (i == 3) {
            temp2 = temp;
        }
        i++;
        temp = "";
    }
    if (dType == doubleType) {
        printSpace();
        printf("%*s", space, "ldd");
        //cout << "ldd";
        cout << " [" << "%"  <<  reg_mapping[temp2] << "]" << ",%" << "f" << currentFloatingPointReg() << endl;
        reg_mapping[temp1] = "f" + to_string(currentFloatingPointReg());
        increaseFloatingPointReg();
    } else {
        printSpace();
        printf("%*s", space, "ld");
        //cout << "ld";
        cout << " [" << "%"  << reg_mapping[temp2] << "]" << ",%" << "l" << currentLocalReg() << endl;
        reg_mapping[temp1] = "l" + to_string(currentLocalReg());
        increaseLocalReg();
    }
}


void processConversion(string str) {
    int temporaryMem = 0;
    stringstream ss;
    string temp;
    ss << str;
    string temp1, temp2, convSt;
    dataType dType = intType;
    int i = 0 ;
    while (!ss.eof()) {
        ss >> temp;
        if (i == 0) {
            temp1 = temp;
        } else if ( i == 2) {
            if(temp[temp.size()-1] == 'f')
                dType = doubleType;
            else dType = intType;
        } else if (i == 3) {
            temp2 = temp;
        }
        i++;
        temp = "";
    }
    printSpace();
    printf("%*s", space, "st");
    //cout << "st";
    cout << " %" << reg_mapping[temp2] << "," << "[" << "%sp + " << TEMPORARY_SPACE_START + temporaryMem * 4 << "]" << endl;
    
    printSpace();
    cout << "ld";
    cout << " [" << "%sp + " << TEMPORARY_SPACE_START + temporaryMem * 4 << "]" << ",%" << "f" << currentFloatingPointReg() << endl;
    
    printSpace();
    if (dType == doubleType) {
        convSt = "fitod";
    } else {
        convSt = "fdtoi";
    }
    cout << convSt;
    cout << " %f" << currentFloatingPointReg() << ",%f" << currentFloatingPointReg() << endl;
    reg_mapping[temp1] = "f" + to_string(currentFloatingPointReg());
}

int checkInputType(string str) {
    char s[100];
    int var, var1, var2, var3;
    
    if ( sscanf(str.c_str(),"bgnstmt %d",&var) == 1) {
        return IS_BEGINSTMT;
    } else if ( (sscanf(str.c_str(),"t%d := local %d",&var, &var2) == 2) ) {
        return IS_LOCAL;
    } else if ( sscanf(str.c_str(),"func %s",s) == 1) {
        return IS_FUNC;
    } else if ( (sscanf(str.c_str(),"t%d := %d",&var, &var2) == 2) ) {
        return IS_CONST;
    } else if ( (sscanf(str.c_str(),"t%d := t%d []i t%d",&var3, &var, &var2) == 3) ||
               (sscanf(str.c_str(),"t%d := t%d []f t%d",&var3, &var, &var2) == 3)) {
        return IS_ARRAY;
    }
    else if ( (sscanf(str.c_str(),"t%d := t%d =i t%d",&var3, &var, &var2) == 3) ||
                (sscanf(str.c_str(),"t%d := t%d =f t%d",&var3, &var, &var2) == 3) ) {
        return IS_ASSIGNMENT;
    } else if ( (sscanf(str.c_str(),"t%d := cvf t%d",&var3, &var) == 2) ||
                (sscanf(str.c_str(),"t%d := cvi t%d", &var3, &var) == 2) ) {
        return IS_CONVERSION;
    } else if ( (sscanf(str.c_str(),"t%d := @i t%d",&var3, &var) == 2) ||
                (sscanf(str.c_str(),"t%d := @f t%d", &var3, &var) == 2) ) {
        return IS_DEREFERENCE;
    }  else if ( (sscanf(str.c_str(),"argi t%d",&var3) == 1) ||
                 (sscanf(str.c_str(),"argf t%d", &var3) == 1) ) {
        return IS_ARGUMENT;
    } else if ( (sscanf(str.c_str(),"t%d := global %s",&var3, s) == 2)  ) {
        return IS_FUNC_TO_CALL;
    } else if ( (sscanf(str.c_str(),"t%d := fi t%d %d",&var3, &var3, &var) == 3) ||
               (sscanf(str.c_str(),"t%d := ff t%d %d",&var3, &var3, &var) == 3) ) {
        return IS_CALL;
    } else if ((sscanf(str.c_str(),"t%d := t%d <i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d >i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d <=i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d >=i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d ==i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d !=i t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d <f t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d >f t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d <=f t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d >=f t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d ==f t%d",&var3, &var1, &var) == 3) ||
              (sscanf(str.c_str(),"t%d := t%d != t%d",&var3, &var1, &var) == 3)) {
        return IS_RELATIONAL;
    }
    else if ( (sscanf(str.c_str(),"t%d := t%d %s t%d",&var3, &var3, s, &var) == 4) ) {
        return IS_OP_AND_SET;
    } else if ( (sscanf(str.c_str(),"localloc %d",&var3) == 1) ) {
        return IS_LOCAL_ALLOC;
    } else if (  (sscanf(str.c_str(),"formal %d",&var3) == 1) ) {
        return IS_FORMAL;
    } else if ( (sscanf(str.c_str(),"t%d := param %d",&var3, &var2) == 2) ) {
        return IS_PARAM;
    } else if ( str.compare("fend") == 0) {
        return IS_FEND;
    } else if ( (str.compare("ret") == 0) ||
        (sscanf(str.c_str(),"ret%s t%d", s, &var2) == 2)){
        return IS_RETURN;
    }
    else if ( (sscanf(str.c_str(),"B%d=L%d",&var3, &var2) == 2)) {
        return IS_SYMBOL;
    } else if ( (sscanf(str.c_str(),"label L%d",&var3) == 1) ) {
        return IS_LABEL;
    } else if ((sscanf(str.c_str(),"br := B%d",&var3) == 1) ||
               (sscanf(str.c_str(),"br := L%d",&var3) == 1) ) {
        return IS_BRANCH_UNCOND;
    } else if ( (sscanf(str.c_str(),"bt t%d B%d", &var, &var3) == 2 ) ) {
        return IS_BRANCH_COND;
    }
    return true;
}

void processBeginStmt(int stmtLineNo) {
    clearDataForPrevStmt();
}

void processInput(string str) {
    int i = 0;
    bool isStr = checkIfaString(str);
    if (isStr) {
        processStr(str);
        increaseLocalReg();
        i++;
        return;
    }
    int type = checkInputType(str);
    if ((didCountLocals && type != IS_LOCAL_ALLOC) ||
        (didCountLocals && type != IS_LOCAL_ALLOC) ) {
        prevLocalAlloc = 0;
        prevFormal = 0;
        didCountLocals = false;
        isLocalAlloc = false;
        printSpace();
        printf("%*s", space, "save");
        int sz = ACTVATION_RECORD_INIT_SIZE + totalSizeLocalAlloc;
        sz = sz + sz % 8;
        //printSpace();
        cout << " %sp,(-" << sz << "),%sp" << endl;
        //printf(" %csp,(-%d),%csp\n", '%', sz, '%');
        totalSizeLocalAlloc = 0;
    }
    if (type == IS_FUNC) {
        processFunc(str);
        didCountLocals = true;
    } else if (type == IS_LOCAL_ALLOC) {
        isLocalAlloc = true;
        processLocalAlloc(str);
        i++;
    } else if (type == IS_BEGINSTMT) {
        processBeginStmt(i);
        i++;
    } else if (type == IS_LOCAL) {
        processLocal(str);
        increaseLocalReg();
        i++;
    } else if (type == IS_CONST) {
        processConst(str);
        increaseLocalReg();
        i++;
    } else if (type == IS_ASSIGNMENT) {
        processAssignment(str);
        i++;
    }   else if (type == IS_CONVERSION) {
        processConversion(str);
        increaseFloatingPointReg();
        i++;
    }  else if (type == IS_DEREFERENCE) {   // need to test
        processDereference(str);
        i++;
    } else if (type == IS_ARGUMENT) {
        saveArgument(str);
        i++;
    } else if (type == IS_FUNC_TO_CALL) {
        processFuncToCall(str);    // need to test
        increaseLocalReg();
        i++;
    } else if (type == IS_CALL) {
        processCall(str);    // need to test
        i++;
    } else if (type == IS_OP_AND_SET) {
        processOpAndSet(str);
        i++;
    } else if (type == IS_FORMAL) {
        processFormal(str);
        i++;
    } else if (type == IS_PARAM) {
        processParam(str);
        increaseLocalReg();
        i++;
    } else if (type == IS_FEND) {
        processFend();
        i++;
    } else if (type == IS_SYMBOL) {
        processSymbol(str);
        i++;
    } else if (type == IS_LABEL) {
        processLabel(str);
        i++;
    } else if (type == IS_RELATIONAL) {
        processRelational(str);
        i++;
    } else if (type == IS_BRANCH_UNCOND) {
        processBranchUncond(str);
        i++;
    } else if (type == IS_BRANCH_COND) {
        processBranchCond(str);
        i++;
    } else if (type == IS_RETURN) {
        processReturn(str);
        i++;
    }
    else if (type == IS_ARRAY) {
        processArray(str);
        i++;
    }
    else {
        fprintf(stderr, "wrong translation");
        i++;
    }
    
}

/*
 * main
 */
int main() {
    string line;
    while (getline(cin, line)) {
        processInput(line);
    }
}
