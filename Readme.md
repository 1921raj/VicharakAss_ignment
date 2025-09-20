
// =============================================================================
// PROJECT ARCHITECTURE DOCUMENTATION
// =============================================================================

/*
PROJECT ARCHITECTURE:

1. LEXER (Tokenization Phase):
   - Converts source code into a stream of tokens
   - Handles keywords, identifiers, operators, literals
   - Tracks line and column information for error reporting
   - Skips whitespace and comments

2. PARSER (Syntax Analysis Phase):
   - Converts tokens into an Abstract Syntax Tree (AST)
   - Implements recursive descent parsing
   - Handles syntax error detection and reporting
   - Supports SimpleLang grammar rules

3. AST (Abstract Syntax Tree):
   - Hierarchical representation of program structure
   - Node types: Program, VariableDeclaration, Assignment, BinaryOperation, etc.
   - Provides clean interface for code generation

4. CODE GENERATOR (Backend):
   - Traverses AST to generate 8-bit assembly code
   - Manages variable memory allocation
   - Handles expression evaluation and control flow
   - Outputs assembly compatible with 8-bit CPU

5. COMPILER CLASS (Main Orchestrator):
   - Coordinates all compilation phases
   - Handles file I/O operations
   - Provides unified interface for compilation process
   - Manages error handling and reporting

INSTRUCTION SET MAPPING:
- Variable storage: Memory locations starting at 0x80
- Arithmetic: ADC (add), SBC (subtract)
- Data movement: LDA (load), STA (store)
- Stack operations: PHA (push), PLA (pop)
- Comparison: CMP (compare)
- Control flow: BNE (branch not equal)
- Program control: HLT (halt)

USAGE:
1. Compile with file: ./compiler source.sl [output.asm]
2. Compile example: ./compiler

The generated assembly can be run on the 8-bit CPU simulator
from https://github.com/lightcode/8bit-computer
*/
