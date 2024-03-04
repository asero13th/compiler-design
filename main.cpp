





#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <unordered_map>

using namespace std;

// Token types
enum class token_type {
    INTEGER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    EOF_TOKEN,
    LPAREN,
    RPAREN,
    VAR,
    CONST,
    DECLARE,
    IDENTIFIER,
    PRINT
};

// Token structure
struct token {
    token_type type;
    string value;
};

// Lexer: Breaks down the source code into tokens
class Lexer {
public:
    Lexer(const string& source) : source(source), currentPosition(0) {}

    // Helper function to extract an integer token from the source code
    token getNextToken() {
        while (currentPosition < source.length() && isspace(source[currentPosition])) {
            currentPosition++;
        }

        if (currentPosition >= source.length()) {
            return { token_type::EOF_TOKEN, "" };
        }

        if (isdigit(source[currentPosition])) {
            return getIntegerToken();
        }

        switch(source[currentPosition]) {
            case '+':
                currentPosition++;
                return { token_type::PLUS, "+" };
            case '-':
                currentPosition++;
                return { token_type::MINUS, "-" };
            case '*':
                currentPosition++;
                return { token_type::MULTIPLY, "*" };
            case '/':
                currentPosition++;
                return { token_type::DIVIDE, "/" };
            case '%':
                currentPosition++;
                return { token_type::MODULO, "%" };
            case '(':
                currentPosition++;
                return { token_type::LPAREN, "(" };
            case ')':
                currentPosition++;
                return { token_type::RPAREN, ")" };
            case '=':
                currentPosition++;
                return { token_type::DECLARE, "=" };
        }

        if (isalpha(source[currentPosition])) {
            return getIdentifierToken();
        }

        // Invalid token
        return { token_type::EOF_TOKEN, "" };
    }

private:

    // Helper function to extract an integer token from the source code
    token getIntegerToken() {
        string value;
        while (currentPosition < source.length() && isdigit(source[currentPosition])) {
            value += source[currentPosition];
            currentPosition++;
        }
        return { token_type::INTEGER, value };
    }

     // Helper function to extract an identifier token from the source code
    token getIdentifierToken() {
        string value;
        while (currentPosition < source.length() && (isalnum(source[currentPosition]) || source[currentPosition] == '_')) {
            value += source[currentPosition];
            currentPosition++;
        }

        if (value == "let" || value == "var") {
            return { token_type::VAR, value };
        } else if (value == "print" || value == "log") {
            return { token_type::PRINT, value };
        }

        return { token_type::IDENTIFIER, value };
    }

    // Source code being processed by the lexer
    string source;
    size_t currentPosition;
};

// Variable symbol table
unordered_map<string, int> variables;

// Parser: Analyzes the syntax and performs BODMAS operations
class Parser {
public:
    Parser(Lexer& lex) : lexer(lex) {}

    int parse() {
        token tok = lexer.getNextToken();
        if (tok.type == token_type::EOF_TOKEN) {
            // Empty program
            return 0;
        }
        int result = parseStatement(tok);
        return result;
    }

private:
    int parseStatement(token& tok) {
        if (tok.type == token_type::PRINT) {
            tok = lexer.getNextToken();  // Consume "print"
            int result = parseExpression(tok);
            cout << result << endl;
            return result;
        } else if (tok.type == token_type::VAR) {
            tok = lexer.getNextToken();  // Consume a variable name
            if (tok.type == token_type::IDENTIFIER) {
                string variableName = tok.value;
                tok = lexer.getNextToken();  // Consume "="

                if (tok.type == token_type::DECLARE) {
                    tok = lexer.getNextToken();  // Consume a value

                    // check if the variable is already declared
                    if (variables.find(variableName) != variables.end()) {
                        cerr << "Error: Variable '" << variableName << "' already declared" << endl;
                        return 0;
                    }

                    int value = parseExpression(tok);
                    variables[variableName] = value;
                } else {
                    cerr << "Error: Invalid variable declaration" << endl;
                    return 0;
                }
            } else {
                cerr << "Error: Invalid variable declaration" << endl;
                return 0;
            }
        } else if (tok.type == token_type::IDENTIFIER) {
            // Variable assignment
            string variableName = tok.value;
            tok = lexer.getNextToken();  // Consume '='

            if (tok.type == token_type::DECLARE) {
                tok = lexer.getNextToken();

                // check if the variable is already declared
                if (variables.find(variableName) == variables.end()) {
                    cerr << "Error: Variable '" << variableName << "' not declared" << endl;
                    return 0;
                }

                int value = parseExpression(tok);
                variables[variableName] = value;

            } else if (tok.type == token_type::PLUS || tok.type == token_type::MINUS || tok.type == token_type::MULTIPLY || tok.type == token_type::DIVIDE || tok.type == token_type::MODULO) {
                tok = lexer.getNextToken();  // Consume '='
                int value = parseExpression(tok);
                return value + variables[variableName];

            }
            else {
                cerr << "Error: Invalid variable assignment" << endl;
                return 0;
            }

        } else {
            // Regular expression evaluation
            return parseExpression(tok);
        }

        return 0;
    }

    int parseExpression(token& tok) {
        int result = parseTerm(tok);

        while (tok.type == token_type::PLUS || tok.type == token_type::MINUS) {
            if (tok.type == token_type::PLUS) {
                tok = lexer.getNextToken();  // Consume '+'
                result += parseTerm(tok);
            } else if (tok.type == token_type::MINUS) {
                tok = lexer.getNextToken();  // Consume '-'
                result -= parseTerm(tok);
            }
        }

        return result;
    }

    int parseTerm(token& tok) {
        int result = parseFactor(tok);

        while (tok.type == token_type::MULTIPLY || tok.type == token_type::DIVIDE || tok.type == token_type::MODULO) {
            if (tok.type == token_type::MULTIPLY) {
                tok = lexer.getNextToken();  // Consume '*'
                result *= parseFactor(tok);
            } else if (tok.type == token_type::DIVIDE) {
                tok = lexer.getNextToken();  // Consume '/'
                int divisor = parseFactor(tok);
                if (divisor != 0) {
                    result /= divisor;
                } else {
                    cerr << "Error: Division by zero" << endl;
                    return 0;
                }
            } else if (tok.type == token_type::MODULO) {
                tok = lexer.getNextToken();  // Consume '%'
                int divisor = parseFactor(tok);
                if (divisor != 0) {
                    result %= divisor;
                } else {
                    cerr << "Error: Modulo by zero" << endl;
                    return 0;
                }
            }
        }

        return result;
    }

    int parseFactor(token& tok) {
        if (tok.type == token_type::INTEGER) {
            int value = stoi(tok.value);
            tok = lexer.getNextToken();  // Consume variable
            return value;
        } else if (tok.type == token_type::IDENTIFIER) {
            string variableName = tok.value;
            tok = lexer.getNextToken();  // Consume variable

            if (variables.find(variableName) != variables.end()) {
                return variables[variableName];
            } else {
                cerr << "Error: Variable '" << variableName << "' not found" << endl;
                return 0;
            }

        } else if (tok.type == token_type::LPAREN) {
            tok = lexer.getNextToken();  // Consume '('
            int result = parseExpression(tok);
            if (tok.type == token_type::RPAREN) {
                tok = lexer.getNextToken();  // Consume ')'
                return result;
            } else {
                cerr << "Error: Unmatched parenthesis" << endl;
                return 0;
            }
        }

        // Invalid factor
        cerr << "Error: Invalid factor" << tok.value << endl;
        return 0;
    }

    Lexer& lexer;
};

int main() {
    string line;
    while (true) {
        cout << "Enter a line of code (or 'exit' to quit): ";
        getline(cin, line);

        if (line == "exit") {
            break;
        }

        Lexer lex(line);
        Parser parse(lex);
        int result = parse.parse();

    }

    return 0;
}
