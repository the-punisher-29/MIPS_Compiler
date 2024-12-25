/*This is the complete Task 1. This code generated the data labels, branching labels and the binary code of each instruction. It also prints the
binary codes of each instruction line by line, data labels with their name and branching labels with their line number in the code.*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <algorithm>
using namespace std;

const int MEMORY_SIZE = 1024;
int memory[MEMORY_SIZE] = {0};
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

            cout << "Loaded variable " << varName << " with value " << value
                 << " at memory address " << memoryPointer << endl;

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


int main()
{
    string filename = "test_code_1.txt";//just give the name of required txt file to get its output

    loadData(filename);
    cout<<endl;

    auto it = dataMap.begin();
    cout<<"Data Labels found with values:"<<endl;
    while (it != dataMap.end())
    {
        cout <<it->first<<" : "<<memory[it->second]<< endl;
        it++;
    }
    cout<<endl;
    cout<<"Branching Labels found at line numbers :"<<endl;

    preprocessLabels(filename);
    auto it1 = labelMap.begin();
    while (it1 != labelMap.end())
    {
        cout << it1->first << " : " << it1->second << endl;
        it1++;
    }

    parseInstruction(filename);
    cout<<endl<<"Binary Instruction Line by Line :"<<endl;

    auto it3 = binaryInstructions.begin();
    while(it3 != binaryInstructions.end()){
        cout<<it3->second<<" : "<<it3->first<<endl;
        it3++;
    }

    return 0;
}