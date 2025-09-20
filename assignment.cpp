
#include <bits/stdc++.h>

using namespace std;

// =============================================================================
// TOKEN DEFINITIONS
// =============================================================================

enum class TokenType {
    TOKEN_INT,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_IF,
    TOKEN_EQUAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_SEMICOLON,
    TOKEN_EOF,
    TOKEN_UNKNOWN
};

struct Token {
    TokenType type;
    string text;
    int line;
    int column;
    
    Token(TokenType t = TokenType::TOKEN_UNKNOWN, const string& txt = "", int ln = 1, int col = 1) 
        : type(t), text(txt), line(ln), column(col) {}
};

// =============================================================================
// LEXER CLASS
// =============================================================================

class Lexer {
private:
    string source;
    size_t position;
    int line;
    int column;
    
    char currentChar() {
        if (position >= source.length()) return '\0';
        return source[position];
    }
    
    char peekChar() {
        if (position + 1 >= source.length()) return '\0';
        return source[position + 1];
    }
    
    void advance() {
        if (position < source.length() && source[position] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
    }
    
    void skipWhitespace() {
        while (isspace(currentChar())) {
            advance();
        }
    }
    
    void skipComments() {
        if (currentChar() == '/' && peekChar() == '/') {
            while (currentChar() != '\n' && currentChar() != '\0') {
                advance();
            }
        }
    }
    
    string readIdentifier() {
        string result;
        while (isalnum(currentChar()) || currentChar() == '_') {
            result += currentChar();
            advance();
        }
        return result;
    }
    
    string readNumber() {
        string result;
        while (isdigit(currentChar())) {
            result += currentChar();
            advance();
        }
        return result;
    }
    
public:
    Lexer(const string& src) : source(src), position(0), line(1), column(1) {}
    
    Token getNextToken() {
        while (currentChar() != '\0') {
            skipWhitespace();
            
            if (currentChar() == '/' && peekChar() == '/') {
                skipComments();
                continue;
            }
            
            int tokenLine = line;
            int tokenColumn = column;
            
            if (isalpha(currentChar()) || currentChar() == '_') {
                string identifier = readIdentifier();
                TokenType type = TokenType::TOKEN_IDENTIFIER;
                
                if (identifier == "int") type = TokenType::TOKEN_INT;
                else if (identifier == "if") type = TokenType::TOKEN_IF;
                
                return Token(type, identifier, tokenLine, tokenColumn);
            }
            
            if (isdigit(currentChar())) {
                string number = readNumber();
                return Token(TokenType::TOKEN_NUMBER, number, tokenLine, tokenColumn);
            }
            
            char ch = currentChar();
            advance();
            
            switch (ch) {
                case '=':
                    if (currentChar() == '=') {
                        advance();
                        return Token(TokenType::TOKEN_EQUAL, "==", tokenLine, tokenColumn);
                    }
                    return Token(TokenType::TOKEN_ASSIGN, "=", tokenLine, tokenColumn);
                case '+':
                    return Token(TokenType::TOKEN_PLUS, "+", tokenLine, tokenColumn);
                case '-':
                    return Token(TokenType::TOKEN_MINUS, "-", tokenLine, tokenColumn);
                case '(':
                    return Token(TokenType::TOKEN_LPAREN, "(", tokenLine, tokenColumn);
                case ')':
                    return Token(TokenType::TOKEN_RPAREN, ")", tokenLine, tokenColumn);
                case '{':
                    return Token(TokenType::TOKEN_LBRACE, "{", tokenLine, tokenColumn);
                case '}':
                    return Token(TokenType::TOKEN_RBRACE, "}", tokenLine, tokenColumn);
                case ';':
                    return Token(TokenType::TOKEN_SEMICOLON, ";", tokenLine, tokenColumn);
                default:
                    return Token(TokenType::TOKEN_UNKNOWN, string(1, ch), tokenLine, tokenColumn);
            }
        }
        
        return Token(TokenType::TOKEN_EOF, "", line, column);
    }
};

// =============================================================================
// AST NODE DEFINITIONS
// =============================================================================

enum class ASTNodeType {
    PROGRAM,
    VARIABLE_DECLARATION,
    ASSIGNMENT,
    BINARY_OPERATION,
    IDENTIFIER,
    NUMBER,
    IF_STATEMENT
};

class ASTNode {
public:
    ASTNodeType type;
    
    ASTNode(ASTNodeType t) : type(t) {}
    virtual ~ASTNode() = default;
};

class ProgramNode : public ASTNode {
public:
    vector<shared_ptr<ASTNode>> statements;
    
    ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}
};

class VariableDeclarationNode : public ASTNode {
public:
    string variableName;
    
    VariableDeclarationNode(const string& name) 
        : ASTNode(ASTNodeType::VARIABLE_DECLARATION), variableName(name) {}
};

class AssignmentNode : public ASTNode {
public:
    string variableName;
    shared_ptr<ASTNode> expression;
    
    AssignmentNode(const string& name, shared_ptr<ASTNode> expr) 
        : ASTNode(ASTNodeType::ASSIGNMENT), variableName(name), expression(expr) {}
};

class BinaryOperationNode : public ASTNode {
public:
    shared_ptr<ASTNode> left;
    string operator_;
    shared_ptr<ASTNode> right;
    
    BinaryOperationNode(shared_ptr<ASTNode> l, const string& op, shared_ptr<ASTNode> r) 
        : ASTNode(ASTNodeType::BINARY_OPERATION), left(l), operator_(op), right(r) {}
};

class IdentifierNode : public ASTNode {
public:
    string name;
    
    IdentifierNode(const string& n) : ASTNode(ASTNodeType::IDENTIFIER), name(n) {}
};

class NumberNode : public ASTNode {
public:
    int value;
    
    NumberNode(int v) : ASTNode(ASTNodeType::NUMBER), value(v) {}
};

class IfStatementNode : public ASTNode {
public:
    shared_ptr<ASTNode> condition;
    shared_ptr<ASTNode> thenStatement;
    
    IfStatementNode(shared_ptr<ASTNode> cond, shared_ptr<ASTNode> then) 
        : ASTNode(ASTNodeType::IF_STATEMENT), condition(cond), thenStatement(then) {}
};

// =============================================================================
// PARSER CLASS
// =============================================================================

class Parser {
private:
    vector<Token> tokens;
    size_t position;
    
    Token currentToken() {
        if (position >= tokens.size()) 
            return Token(TokenType::TOKEN_EOF);
        return tokens[position];
    }
    
    Token peekToken() {
        if (position + 1 >= tokens.size()) 
            return Token(TokenType::TOKEN_EOF);
        return tokens[position + 1];
    }
    
    void advance() {
        if (position < tokens.size()) position++;
    }
    
    bool match(TokenType expected) {
        if (currentToken().type == expected) {
            advance();
            return true;
        }
        return false;
    }
    
    void expect(TokenType expected) {
        if (!match(expected)) {
            throw runtime_error("Parse error at line " + to_string(currentToken().line) + 
                              ": expected token type " + to_string((int)expected));
        }
    }
    
    shared_ptr<ASTNode> parseExpression() {
        return parseAdditionExpression();
    }
    
    shared_ptr<ASTNode> parseAdditionExpression() {
        shared_ptr<ASTNode> left = parsePrimaryExpression();
        
        while (currentToken().type == TokenType::TOKEN_PLUS || 
               currentToken().type == TokenType::TOKEN_MINUS) {
            string op = currentToken().text;
            advance();
            shared_ptr<ASTNode> right = parsePrimaryExpression();
            left = make_shared<BinaryOperationNode>(left, op, right);
        }
        
        return left;
    }
    
    shared_ptr<ASTNode> parsePrimaryExpression() {
        Token token = currentToken();
        
        if (token.type == TokenType::TOKEN_NUMBER) {
            advance();
            return make_shared<NumberNode>(stoi(token.text));
        }
        
        if (token.type == TokenType::TOKEN_IDENTIFIER) {
            advance();
            return make_shared<IdentifierNode>(token.text);
        }
        
        if (token.type == TokenType::TOKEN_LPAREN) {
            advance();
            shared_ptr<ASTNode> expr = parseExpression();
            expect(TokenType::TOKEN_RPAREN);
            return expr;
        }
        
        throw runtime_error("Parse error at line " + to_string(token.line) + 
                          ": unexpected token in expression");
    }
    
    shared_ptr<ASTNode> parseComparison() {
        shared_ptr<ASTNode> left = parseExpression();
        
        if (currentToken().type == TokenType::TOKEN_EQUAL) {
            string op = currentToken().text;
            advance();
            shared_ptr<ASTNode> right = parseExpression();
            return make_shared<BinaryOperationNode>(left, op, right);
        }
        
        return left;
    }
    
    shared_ptr<ASTNode> parseStatement() {
        Token token = currentToken();
        
        // Variable declaration
        if (token.type == TokenType::TOKEN_INT) {
            advance();
            if (currentToken().type != TokenType::TOKEN_IDENTIFIER) {
                throw runtime_error("Parse error: expected identifier after 'int'");
            }
            string varName = currentToken().text;
            advance();
            expect(TokenType::TOKEN_SEMICOLON);
            return make_shared<VariableDeclarationNode>(varName);
        }
        
        // Assignment
        if (token.type == TokenType::TOKEN_IDENTIFIER) {
            string varName = token.text;
            advance();
            expect(TokenType::TOKEN_ASSIGN);
            shared_ptr<ASTNode> expr = parseExpression();
            expect(TokenType::TOKEN_SEMICOLON);
            return make_shared<AssignmentNode>(varName, expr);
        }
        
        // If statement
        if (token.type == TokenType::TOKEN_IF) {
            advance();
            expect(TokenType::TOKEN_LPAREN);
            shared_ptr<ASTNode> condition = parseComparison();
            expect(TokenType::TOKEN_RPAREN);
            expect(TokenType::TOKEN_LBRACE);
            shared_ptr<ASTNode> thenStmt = parseStatement();
            expect(TokenType::TOKEN_RBRACE);
            return make_shared<IfStatementNode>(condition, thenStmt);
        }
        
        throw runtime_error("Parse error at line " + to_string(token.line) + 
                          ": unexpected token in statement");
    }
    
public:
    Parser(const vector<Token>& toks) : tokens(toks), position(0) {}
    
    shared_ptr<ProgramNode> parse() {
        auto program = make_shared<ProgramNode>();
        
        while (currentToken().type != TokenType::TOKEN_EOF) {
            try {
                auto stmt = parseStatement();
                program->statements.push_back(stmt);
            } catch (const runtime_error& e) {
                cerr << "Parser error: " << e.what() << endl;
                break;
            }
        }
        
        return program;
    }
};

// =============================================================================
// CODE GENERATOR CLASS
// =============================================================================

class CodeGenerator {
private:
    map<string, int> variableAddresses;
    int nextAddress;
    vector<string> assembly;
    int labelCounter;
    
    string generateLabel() {
        return "L" + to_string(labelCounter++);
    }
    
    void generateExpression(shared_ptr<ASTNode> node) {
        switch (node->type) {
            case ASTNodeType::NUMBER: {
                auto numNode = static_pointer_cast<NumberNode>(node);
                assembly.push_back("    LDA #" + to_string(numNode->value) + "  ; Load immediate value");
                break;
            }
            
            case ASTNodeType::IDENTIFIER: {
                auto idNode = static_pointer_cast<IdentifierNode>(node);
                if (variableAddresses.find(idNode->name) == variableAddresses.end()) {
                    throw runtime_error("Undefined variable: " + idNode->name);
                }
                assembly.push_back("    LDA $" + to_string(variableAddresses[idNode->name]) + "  ; Load variable " + idNode->name);
                break;
            }
            
            case ASTNodeType::BINARY_OPERATION: {
                auto binNode = static_pointer_cast<BinaryOperationNode>(node);
                
                if (binNode->operator_ == "==") {
                    // Generate comparison
                    generateExpression(binNode->left);
                    assembly.push_back("    PHA              ; Push left operand");
                    generateExpression(binNode->right);
                    assembly.push_back("    TAX              ; Transfer A to X");
                    assembly.push_back("    PLA              ; Pop left operand");
                    assembly.push_back("    CMP X            ; Compare A with X");
                } else {
                    // Generate left operand
                    generateExpression(binNode->left);
                    assembly.push_back("    PHA              ; Push left operand");
                    
                    // Generate right operand
                    generateExpression(binNode->right);
                    assembly.push_back("    TAX              ; Transfer A to X");
                    assembly.push_back("    PLA              ; Pop left operand");
                    
                    // Perform operation
                    if (binNode->operator_ == "+") {
                        assembly.push_back("    ADC X            ; Add X to A");
                    } else if (binNode->operator_ == "-") {
                        assembly.push_back("    SBC X            ; Subtract X from A");
                    }
                }
                break;
            }
            
            default:
                throw runtime_error("Unsupported expression type in code generation");
        }
    }
    
public:
    CodeGenerator() : nextAddress(0x80), labelCounter(0) {
        assembly.push_back("; SimpleLang Compiler Output");
        assembly.push_back("; Generated Assembly for 8-bit CPU");
        assembly.push_back("");
    }
    
    void generateCode(shared_ptr<ProgramNode> program) {
        for (auto& stmt : program->statements) {
            generateStatement(stmt);
        }
        
        // Add program termination
        assembly.push_back("");
        assembly.push_back("    HLT              ; Halt the processor");
    }
    
    void generateStatement(shared_ptr<ASTNode> node) {
        switch (node->type) {
            case ASTNodeType::VARIABLE_DECLARATION: {
                auto varNode = static_pointer_cast<VariableDeclarationNode>(node);
                variableAddresses[varNode->variableName] = nextAddress++;
                assembly.push_back("; Declare variable: " + varNode->variableName + 
                                 " at address $" + to_string(variableAddresses[varNode->variableName]));
                break;
            }
            
            case ASTNodeType::ASSIGNMENT: {
                auto assignNode = static_pointer_cast<AssignmentNode>(node);
                assembly.push_back("; Assignment: " + assignNode->variableName);
                
                generateExpression(assignNode->expression);
                
                if (variableAddresses.find(assignNode->variableName) == variableAddresses.end()) {
                    throw runtime_error("Undefined variable: " + assignNode->variableName);
                }
                
                assembly.push_back("    STA $" + to_string(variableAddresses[assignNode->variableName]) + 
                                 "  ; Store to variable " + assignNode->variableName);
                break;
            }
            
            case ASTNodeType::IF_STATEMENT: {
                auto ifNode = static_pointer_cast<IfStatementNode>(node);
                string endLabel = generateLabel();
                
                assembly.push_back("; If statement");
                generateExpression(ifNode->condition);
                assembly.push_back("    BNE " + endLabel + "    ; Branch if not equal (condition false)");
                
                generateStatement(ifNode->thenStatement);
                
                assembly.push_back(endLabel + ":");
                break;
            }
            
            default:
                throw runtime_error("Unsupported statement type in code generation");
        }
        
        assembly.push_back("");
    }
    
    void printAssembly() {
        for (const string& line : assembly) {
            cout << line << endl;
        }
    }
    
    void saveAssembly(const string& filename) {
        ofstream file(filename);
        if (file.is_open()) {
            for (const string& line : assembly) {
                file << line << endl;
            }
            file.close();
            cout << "Assembly code saved to " << filename << endl;
        } else {
            cerr << "Error: Could not open file " << filename << " for writing" << endl;
        }
    }
};

// =============================================================================
// COMPILER CLASS - MAIN ORCHESTRATOR
// =============================================================================

class SimpleLangCompiler {
private:
    string sourceCode;
    
public:
    bool loadSource(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open source file " << filename << endl;
            return false;
        }
        
        string line;
        sourceCode.clear();
        while (getline(file, line)) {
            sourceCode += line + "\n";
        }
        file.close();
        
        cout << "Source code loaded from " << filename << endl;
        return true;
    }
    
    void setSource(const string& code) {
        sourceCode = code;
    }
    
    bool compile(const string& outputFilename = "output.asm") {
        try {
            cout << "\n=== LEXICAL ANALYSIS ===" << endl;
            Lexer lexer(sourceCode);
            vector<Token> tokens;
            
            Token token;
            do {
                token = lexer.getNextToken();
                tokens.push_back(token);
                
                if (token.type != TokenType::TOKEN_EOF) {
                    cout << "Token: " << (int)token.type << " '" << token.text 
                         << "' at line " << token.line << endl;
                }
            } while (token.type != TokenType::TOKEN_EOF);
            
            cout << "\n=== SYNTAX ANALYSIS ===" << endl;
            Parser parser(tokens);
            auto ast = parser.parse();
            cout << "Abstract Syntax Tree generated successfully" << endl;
            
            cout << "\n=== CODE GENERATION ===" << endl;
            CodeGenerator generator;
            generator.generateCode(ast);
            
            cout << "\n=== GENERATED ASSEMBLY ===" << endl;
            generator.printAssembly();
            generator.saveAssembly(outputFilename);
            
            return true;
            
        } catch (const exception& e) {
            cerr << "Compilation error: " << e.what() << endl;
            return false;
        }
    }
};

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char* argv[]) {
    cout << "SimpleLang Compiler for 8-bit CPU" << endl;
    cout << "=================================" << endl;
    
    SimpleLangCompiler compiler;
    
    if (argc > 1) {
        // Compile from file
        string filename = argv[1];
        if (compiler.loadSource(filename)) {
            string outputFile = (argc > 2) ? argv[2] : "output.asm";
            compiler.compile(outputFile);
        }
    } else {
        // Compile example program
        cout << "\nCompiling example program..." << endl;
        
        string exampleCode = R"(
// Variable declarations
int a;
int b;
int c;

// Assignments
a = 70;
b = 120;
c = a + b;

// Conditional
if (c == 30) {
    c = c + 1;
}
)";
        
        compiler.setSource(exampleCode);
        compiler.compile();
    }
    
    return 0;
}
