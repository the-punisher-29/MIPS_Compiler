/*This is the complete combination of Task 1 and Task 2. SInce Task 1 is needed to be done to perform Task 2, we've included it  here.
This code generated the data labels, branching labels, binary code of each instruction and then simulates the program. After each instruction is executed,
the state if all the Registers is printed on the screen.*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <algorithm>
using namespace std;

// Simulate memory (size 1024 words, each word is 4 bytes)
const int MEMORY_SIZE = 1024;
int memory[MEMORY_SIZE] = {0};

int registers[32] = {0};

int maxLines = 0;

// Map to store the address of each variable defined in the .data section
unordered_map<string, int> dataMap;

int memoryPointer = 0; // Points to the current free memory location
unordered_map <int,string> binaryInstructions;

// Function to load data from the .data section
void loadData(const string &filename)
{
    ifstream infile(filename);
    string line;
    bool isDataSection = false;

    // Read the file line by line
    while (getline(infile, line))
    {
        size_t start = line.find_first_not_of(' ');
        if (start == std::string::npos)
        {
            continue;
        }
        size_t end = line.find_last_not_of(' ');
        line = line.substr(start, end - start + 1);

        // Check if we are in the .data section
        if (line.find(".data") != string::npos)
        {
            isDataSection = true;
            continue; // Skip the .data line itself
        }

        // Exit when we reach the .text section
        if (line.find(".text") != string::npos)
        {
            break;
        }

        // Process variable declarations in the .data section
        if (isDataSection && !line.empty())
        {
            stringstream ss(line);
            string varName, directive;
            int value;

            ss >> varName >> directive >> value;

            // Remove colon from variable name
            if (varName.back() == ':')
            {
                varName.pop_back();
            }

            // Store the value in memory and map the variable name to its address
            memory[memoryPointer] = value;
            dataMap[varName] = memoryPointer;

            memoryPointer++; // Move to the next memory location
        }
    }
}

unordered_map<string, int> labelMap; // Map to store label names and line numbers

void preprocessLabels(string &filename)
{

    ifstream infile1(filename);
    string line1;
    bool inTextSection1 = 0;

    vector<string> instructions0;

    while (getline(infile1, line1))
    {
        size_t start = line1.find_first_not_of(' ');
        if (start == std::string::npos)
        {
            continue;
        }
        
        size_t end = line1.find_last_not_of(' ');
        line1 = line1.substr(start, end - start + 1);

        // Check if we have encountered the ".text" section
        if (line1.find(".text") != string::npos)
        {
            inTextSection1 = true; // Start parsing instructions after this point
            continue;              // Skip the line that contains ".text"
        }

        // If we are in the .text section, parse the instructions
        if (inTextSection1)
        {
            maxLines++;
            instructions0.push_back(line1);
        }
    }

    for (int i = 0; i < instructions0.size(); i++)
    {
        string instruction = instructions0[i];
        size_t colonPos = instruction.find(':');
        if (colonPos != string::npos)
        {
            // Extract the label name
            string label = instruction.substr(0, colonPos);
            labelMap[label] = i+1; // Store the line number
        }
    }
}

// Function to convert a register to its corresponding binary value
string regToBinary(const string &reg)
{
    unordered_map<string, string> regMap = {
        {"$zero", "00000"},{"$v0", "00010"},{"$v1", "00011"},{"$a0", "00100"},{"$a1", "00101"},{"$a2", "00110"},{"$a3", "00111"},{"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"}, {"$t3", "01011"}, {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"}, {"$t7", "01111"}, {"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"}, {"$s3", "10011"}, {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"}, {"$s7", "10111"}, {"$t8", "11000"}, {"$t9", "11001"}};
    return regMap[reg];
}

// Function to convert an integer to binary of specified bits
string intToBinary(int num, int bits)
{
    return bitset<32>(num).to_string().substr(32 - bits, bits);
}

// Function to handle memory offset for lw/sw: $rt, offset($rs)
void parseMemoryOffset(const string &operand, string &rt, string &rs, string &offset)
{
    size_t pos1 = operand.find('(');
    size_t pos2 = operand.find(')');
    if (pos1 != string::npos)
    {
        offset = operand.substr(0, pos1);               // Offset value
        rs = operand.substr(pos1 + 1, pos2 - pos1 - 1); // Base register
    }
    else
    {
        offset = "0";
        rs = operand;
    }
}

// R-Type instruction processing
string processRType(const string &instruction, const vector<string> &tokens)
{
    unordered_map<string, string> functMap = {
        {"add", "100000"}, {"sub", "100010"}, {"and", "100100"}, {"or", "100101"}, {"slt", "101010"}, {"mul", "011000"}, {"div", "011010"}, {"xor", "100110"}, {"nor", "100111"}};
    string opCode = "000000";
    string rs = regToBinary(tokens[2]); // Source register
    string rt = regToBinary(tokens[3]); // Target register
    string rd = regToBinary(tokens[1]); // Destination register
    string shamt = "00000";             // Shift amount, for this it's always zero
    string funct = functMap[instruction];
    return opCode + rs + rt + rd + shamt + funct;
}

// I-Type instruction processing (for lw, sw, addi, andi, ori, beq, bne)
string processIType(const string &instruction, const vector<string> &tokens)
{
    unordered_map<string, string> opCodeMap = {
        {"lw", "100011"}, {"sw", "101011"}, {"beq", "000100"}, {"bne", "000101"}, {"addi", "001000"}, {"andi", "001100"}, {"ori", "001101"}};
    string opCode = opCodeMap[instruction];

    if (instruction == "lw" || instruction == "sw")
    {
        string rt = regToBinary(tokens[1]); // Target register (for lw/sw)
        string rs, offset;
        parseMemoryOffset(tokens[2], rt, rs, offset); // Parsing offset($rs)
        string rsBinary = regToBinary(rs);
        string offsetBinary = intToBinary(stoi(offset), 16);
        return opCode + rsBinary + rt + offsetBinary;
    }
    else if (instruction == "beq" || instruction == "bne")
    {
        string rs = regToBinary(tokens[1]); // Source register
        string rt = regToBinary(tokens[2]); // Target register
        string nextLine = tokens[3];        // Offset
        int nextLineIndex = labelMap[nextLine];
        return opCode + rs + rt + intToBinary(nextLineIndex+1, 16);
    }
    else if (instruction == "addi" || instruction == "andi" || instruction == "ori")
    {
        string rt = regToBinary(tokens[1]); // Target register
        string rs = regToBinary(tokens[2]); // Source register
        int imm = stoi(tokens[3]);          // Immediate value
        string immBinary = intToBinary(imm, 16);
        return opCode + rs + rt + immBinary;
    }
    return "";
}

// J-Type instruction processing
string processJType(const string &instruction, const vector<string> &tokens)
{
    unordered_map<string, string> opCodeMap = {
        {"j", "000010"}, {"jal", "000011"}};
    string opCode = opCodeMap[instruction];
    string nextLine = tokens[1];
    int nextLineIndex = labelMap[nextLine];
    string addressBinary = intToBinary(nextLineIndex, 26); // Shift right 2 bits for word alignment
    return opCode + addressBinary;
}

string processLa(const string &instruction, const vector<string> &tokens)
{
    string rt = regToBinary(tokens[1]); // Target register
    string varName = tokens[2];         // Variable name

    // Get the address of the variable from dataMap
    if (dataMap.find(varName) != dataMap.end())
    {
        int address = memory[dataMap[varName]];
        string addressBinary = intToBinary(address, 16); // 32-bit address
        return "000000000000" + rt + addressBinary;      // Format: [0s][rt][address]
    }
    else
    {
        cerr << "Error: Variable " << varName << " not found in dataMap." << endl;
        return "";
    }
}

string processLi(const string &instruction, const vector<string> &tokens)
{
    string rt = regToBinary(tokens[1]); // Target register
    string number = tokens[2];         // Variable name

    // Get the address of the variable from dataMap
    string numBinary = intToBinary(stoi(number),17); // 32-bit address
    return "00000000000" + rt + numBinary;      // Format: [0s][rt][addresss
}

// Function to parse MIPS instruction and determine its type
void parseInstruction(string &filename)
{
    ifstream infile1(filename);
    string line1;
    bool inTextSection1 = 0;
    int lineNumber = 0;

    while (getline(infile1, line1))
    {
        size_t start = line1.find_first_not_of(' ');
        if (start == std::string::npos)
        {
            continue;
        }
        size_t end = line1.find_last_not_of(' ');
        line1 = line1.substr(start, end - start + 1);

        // Check if we have encountered the ".text" section
        if (line1.find(".text") != string::npos)
        {
            inTextSection1 = true; // Start parsing instructions after this point
            continue;              // Skip the line that contains ".text"
        }

        // If we are in the .text section, parse the instructions
        if (inTextSection1)
        {
            lineNumber++;
            string modifiedLine = line1;
            replace(modifiedLine.begin(), modifiedLine.end(), ',', ' ');
            istringstream iss(modifiedLine);
            vector<string> tokens;
            string token;
            while (iss >> token)
            {
                tokens.push_back(token);
            }

            // binaryInstructions[lineNumber] = modifiedLine;

            // First token is the instruction
            string instruction = tokens[0];

            if (instruction == "la")
            {
                binaryInstructions[lineNumber] = processLa(instruction, tokens);
            }
            else if(instruction == "li"){
                binaryInstructions[lineNumber] = processLi(instruction,tokens);
            }
            // Determine instruction type (R, I, or J)
            else if (instruction == "add" || instruction == "sub" || instruction == "and" ||
                     instruction == "or" || instruction == "slt" || instruction == "mul" ||
                     instruction == "div" || instruction == "xor" || instruction == "nor")
            {

                binaryInstructions[lineNumber] = processRType(instruction, tokens);
            }
            else if (instruction == "lw" || instruction == "sw" || instruction == "beq" ||
                     instruction == "bne" || instruction == "addi" || instruction == "andi" ||
                     instruction == "ori")
            {

                binaryInstructions[lineNumber] = processIType(instruction, tokens);
            }
            else if (instruction == "j" || instruction == "jal")
            {

                binaryInstructions[lineNumber] = processJType(instruction, tokens);
            }
        }
    }
}

int binaryToDecimal(string n)
{
    string num = n;
    int dec_value = 0;
 
    // Initializing base value to 1, i.e 2^0
    int base = 1;
 
    int len = num.length();
    for (int i = len - 1; i >= 0; i--) {
        if (num[i] == '1')
            dec_value += base;
        base = base * 2;
    }
 
    return dec_value;
}
void simulate(int currLineNumber);
void executeInstruction(const string &binaryInstruction,int &currLineNumber) {
    string opcode = binaryInstruction.substr(0, 6); // First 6 bits for opcode
    string rs, rt, rd, shamt, funct;

    // Determine instruction type and execute
    if(binaryInstruction.size() == 34){
        rt = binaryInstruction.substr(12, 5);
        string varName = ""; // Extract varName if applicable
        registers[binaryToDecimal(rt)] = binaryToDecimal(binaryInstruction.substr(17,17)); // Load address into register
        currLineNumber++;
    }
    else if (binaryInstruction.size() == 33) { // la instruction
        rt = binaryInstruction.substr(12, 5);
        string varName = ""; // Extract varName if applicable
        registers[binaryToDecimal(rt)] = binaryToDecimal(binaryInstruction.substr(17,16)); // Load address into register
        currLineNumber++;
    } else if (opcode == "000000") { // R-Type instructions
        rs = binaryInstruction.substr(6, 5);
        rt = binaryInstruction.substr(11, 5);
        rd = binaryInstruction.substr(16, 5);
        shamt = binaryInstruction.substr(21, 5);
        funct = binaryInstruction.substr(26, 6);

        if (funct == "100000") { // add
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] + registers[binaryToDecimal(rt)];
        } else if (funct == "100010") { // sub
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] - registers[binaryToDecimal(rt)];
        } else if (funct == "100100") { // and
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] & registers[binaryToDecimal(rt)];
        } else if (funct == "100101") { // or
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] | registers[binaryToDecimal(rt)];
        } else if (funct == "101010") { // slt
            registers[binaryToDecimal(rd)] = (registers[binaryToDecimal(rs)] < registers[binaryToDecimal(rt)]) ? 1 : 0;
        } else if (funct == "011000") { // mul
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] * registers[binaryToDecimal(rt)];
        } else if (funct == "011010") { // div
            int divisor = registers[binaryToDecimal(rt)];
            if (divisor != 0) {
                registers[0] = registers[binaryToDecimal(rs)] / divisor; // Quotient
                registers[1] = registers[binaryToDecimal(rs)] % divisor; // Remainder
            } else {
                cerr << "Error: Division by zero" << endl;
            }
        } else if (funct == "100110") { // xor
            registers[binaryToDecimal(rd)] = registers[binaryToDecimal(rs)] ^ registers[binaryToDecimal(rt)];
        } else if (funct == "100111") { // nor
            registers[binaryToDecimal(rd)] = ~(registers[binaryToDecimal(rs)] | registers[binaryToDecimal(rt)]);
        }
        currLineNumber++;
    } else if (opcode == "100011") { // lw
        rt = binaryInstruction.substr(11, 5);
        rs = binaryInstruction.substr(6, 5);
        int offset = binaryToDecimal(binaryInstruction.substr(16, 16));
        registers[binaryToDecimal(rt)] = registers[binaryToDecimal(rs) + offset / 4];
        currLineNumber++;
    } else if (opcode == "101011") { // sw
        rt = binaryInstruction.substr(11, 5);
        rs = binaryInstruction.substr(6, 5);
        int offset = binaryToDecimal(binaryInstruction.substr(16, 16));
        memory[registers[binaryToDecimal(rs)] + offset / 4] = registers[binaryToDecimal(rt)];
        currLineNumber++;
    } else if (opcode == "000100") { // beq
            rs = binaryInstruction.substr(6, 5);
            rt = binaryInstruction.substr(11, 5);
            int nextIndex = binaryToDecimal(binaryInstruction.substr(16, 16));
            if (registers[binaryToDecimal(rs)] == registers[binaryToDecimal(rt)]) {
                currLineNumber = nextIndex;
            }
            else{
                currLineNumber++;
            }
        } else if (opcode == "000101") { // bne
            rs = binaryInstruction.substr(6, 5);
            rt = binaryInstruction.substr(11, 5);
            int nextIndex = binaryToDecimal(binaryInstruction.substr(16, 16));
            if (registers[binaryToDecimal(rs)] != registers[binaryToDecimal(rt)]) {
                currLineNumber = nextIndex+1;
            }
            else{
                currLineNumber++;
            }
    } else if (opcode == "001000") { // addi
        rt = binaryInstruction.substr(11, 5);
        rs = binaryInstruction.substr(6, 5);
        int immediate = binaryToDecimal(binaryInstruction.substr(16, 16));
        registers[binaryToDecimal(rt)] = registers[binaryToDecimal(rs)] + immediate;
        currLineNumber++;
    } else if (opcode == "001100") { // andi
        rt = binaryInstruction.substr(11, 5);
        rs = binaryInstruction.substr(6, 5);
        int immediate = binaryToDecimal(binaryInstruction.substr(16, 16));
        registers[binaryToDecimal(rt)] = registers[binaryToDecimal(rs)] & immediate;
        currLineNumber++;
    } else if (opcode == "001101") { // ori
        rt = binaryInstruction.substr(11, 5);
        rs = binaryInstruction.substr(6, 5);
        int immediate = binaryToDecimal(binaryInstruction.substr(16, 16));
        registers[binaryToDecimal(rt)] = registers[binaryToDecimal(rs)] | immediate;
        currLineNumber++;
    } else if (opcode == "000010") { // j
        int nextIndex = binaryToDecimal(binaryInstruction.substr(6, 26));
        currLineNumber = nextIndex+1;
        
    } else if (opcode == "000011") { // jal
        int nextIndex = binaryToDecimal(binaryInstruction.substr(6, 26));
        int demo = nextIndex+1;
        simulate(demo);
        currLineNumber++;
    } 

    // Print register state
    cout << "Registers state after execution:" << endl;
    int count = 0;
    for (int i = 0; i < 32; i++) {
        count++;
        cout << "R" << i << ": " << registers[i] << " ";
        if(count%5==0){
            cout<<endl;
        }
    }
    cout << endl;
}

void simulate(int currLineNumber){
    
    while(currLineNumber <= maxLines){
        if(binaryInstructions.find(currLineNumber) == binaryInstructions.end()) break;
        string binary = binaryInstructions[currLineNumber];

        cout<<"Executing Line Number "<<currLineNumber<<" with Binary Code : ";
        cout<<binary<<endl;
        executeInstruction(binary,currLineNumber);
    }
    
}

int main()
{
    string filename = "b22es006_b22cs014_b22cs045_test_code_5.txt";//just write the name of the file you want to run here

    loadData(filename);
    preprocessLabels(filename);
    parseInstruction(filename);

    simulate(1);

    return 0;
}