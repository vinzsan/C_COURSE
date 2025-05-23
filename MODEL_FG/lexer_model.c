#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_TOKENS 128

typedef enum {
    TOKEN_INTEGER,
    TOKEN_IDENTIFIER,
    TOKEN_ASSIGN,
    TOKEN_LITERAL,
    TOKEN_IF,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EQUAL,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_STRING,
    TOKEN_ENDCODE,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
} Token;

Token tokens[MAX_TOKENS];
int token_count = 0;

void add_token(TokenType type, const char* text) {
    tokens[token_count].type = type;
    strncpy(tokens[token_count].text, text, 63);
    token_count++;
}

void lex(const char* src) {
    const char* p = src;

    while (*p) {
        if (isspace(*p)) { p++; continue; }

        if (strncmp(p, "integer", 7) == 0 && isspace(p[7])) {
            add_token(TOKEN_INTEGER, "integer"); p += 7;
        } else if (strncmp(p, "if", 2) == 0 && !isalnum(p[2])) {
            add_token(TOKEN_IF, "if"); p += 2;
        } else if (strncmp(p, "endcode", 7) == 0 && !isalnum(p[7])) {
            add_token(TOKEN_ENDCODE, "endcode"); p += 7;
        } else if (*p == '=' && *(p+1) == '=') {
            add_token(TOKEN_EQUAL, "=="); p += 2;
        } else if (*p == '=') {
            add_token(TOKEN_ASSIGN, "="); p++;
        } else if (*p == '(') {
            add_token(TOKEN_LPAREN, "("); p++;
        } else if (*p == ')') {
            add_token(TOKEN_RPAREN, ")"); p++;
        } else if (*p == '{') {
            add_token(TOKEN_LBRACE, "{"); p++;
        } else if (*p == '}') {
            add_token(TOKEN_RBRACE, "}"); p++;
        } else if (*p == '"') {
            p++;
            char str[64] = {0}; int i = 0;
            while (*p && *p != '"') str[i++] = *p++;
            p++;
            add_token(TOKEN_STRING, str);
        } else if (isdigit(*p)) {
            char num[32] = {0}; int i = 0;
            while (isdigit(*p)) num[i++] = *p++;
            add_token(TOKEN_LITERAL, num);
        } else if (isalpha(*p)) {
            char word[32] = {0}; int i = 0;
            while (isalnum(*p)) word[i++] = *p++;
            add_token(TOKEN_IDENTIFIER, word);
        } else {
            p++; // skip unknown
        }
    }

    add_token(TOKEN_EOF, "<eof>");
}


typedef enum {
    NODE_ASSIGN,
    NODE_IF,
    NODE_LITERAL,
    NODE_VAR,
    NODE_ECHO,
} NodeType;

typedef struct Node {
    NodeType type;
    char var_name[64];
    int literal_value;
    struct Node* condition;   // untuk if: kondisi
    struct Node* then_branch; // untuk if: blok setelah if
    struct Node* expr;        // untuk assignment atau echo: nilai
    struct Node* next;        // untuk statement selanjutnya (linked list)
} Node;

int current_token = 0;

Token* peek() {
    return &tokens[current_token];
}

Token* consume() {
    return &tokens[current_token++];
}

int match(TokenType t) {
    if (peek()->type == t) {
        current_token++;
        return 1;
    }
    return 0;
}

// Buat node literal
Node* make_literal(int val) {
    Node* n = malloc(sizeof(Node));
    n->type = NODE_LITERAL;
    n->literal_value = val;
    n->next = NULL;
    return n;
}

// Buat node variabel
Node* make_var(const char* name) {
    Node* n = malloc(sizeof(Node));
    n->type = NODE_VAR;
    strncpy(n->var_name, name, 63);
    n->next = NULL;
    return n;
}

// Buat node assignment
Node* parse_assignment() {
    // expecting: integer var = literal
    consume(); // consume 'integer'
    Token* varname = consume(); // var
    if (!match(TOKEN_ASSIGN)) {
        printf("Expected '=' after variable name\n");
        return NULL;
    }
    Token* val = consume(); // literal
    if (val->type != TOKEN_LITERAL) {
        printf("Expected literal after '='\n");
        return NULL;
    }

    Node* assign = malloc(sizeof(Node));
    assign->type = NODE_ASSIGN;
    strncpy(assign->var_name, varname->text, 63);
    assign->expr = make_literal(atoi(val->text));
    assign->next = NULL;
    return assign;
}

// Parse echo statement
Node* parse_echo() {
    consume(); // echo
    if (!match(TOKEN_LPAREN)) {
        printf("Expected '(' after echo\n");
        return NULL;
    }
    Token* strtoken = consume();
    if (strtoken->type != TOKEN_STRING) {
        printf("Expected string literal in echo\n");
        return NULL;
    }
    if (!match(TOKEN_RPAREN)) {
        printf("Expected ')' after echo string\n");
        return NULL;
    }

    Node* echo = malloc(sizeof(Node));
    echo->type = NODE_ECHO;
    strncpy(echo->var_name, strtoken->text, 63);
    echo->next = NULL;
    return echo;
}

// Parse if statement
Node* parse_if() {
    consume(); // consume 'if'

    if (!match(TOKEN_LPAREN)) {
        printf("Expected '(' after if\n");
        return NULL;
    }

    Token* var_token = consume();
    if (var_token->type != TOKEN_IDENTIFIER) {
        printf("Expected variable in if condition\n");
        return NULL;
    }

    Token* eq_token = consume();
    if (eq_token->type != TOKEN_EQUAL) {
        printf("Expected '==' in if condition\n");
        return NULL;
    }

    Token* val_token = consume();
    if (val_token->type != TOKEN_LITERAL) {
        printf("Expected literal in if condition\n");
        return NULL;
    }

    if (!match(TOKEN_RPAREN)) {
        printf("Expected ')' after if condition\n");
        return NULL;
    }

    if (!match(TOKEN_LBRACE)) {
        printf("Expected '{' after if condition\n");
        return NULL;
    }

    // Parse statements inside { }
    Node* then_branch = NULL;
    Node* last_stmt = NULL;
    while (peek()->type != TOKEN_RBRACE && peek()->type != TOKEN_EOF) {
        Node* stmt = NULL;
        if (peek()->type == TOKEN_IDENTIFIER && strcmp(peek()->text, "echo") == 0) {
            stmt = parse_echo();
        } else {
            printf("Unknown statement inside if block\n");
            return NULL;
        }

        if (then_branch == NULL)
            then_branch = stmt;
        else
            last_stmt->next = stmt;
        last_stmt = stmt;
    }

    if (!match(TOKEN_RBRACE)) {
        printf("Expected '}' to close if block\n");
        return NULL;
    }

    // Buat node if
    Node* ifnode = malloc(sizeof(Node));
    ifnode->type = NODE_IF;
    ifnode->condition = make_var(var_token->text); // var
    ifnode->expr = make_literal(atoi(val_token->text)); // nilai dibandingkan
    ifnode->then_branch = then_branch;
    ifnode->next = NULL;

    return ifnode;
}

Node* parse_program() {
    Node* head = NULL;
    Node* last = NULL;

    while (peek()->type != TOKEN_EOF) {
        Node* stmt = NULL;

        if (peek()->type == TOKEN_INTEGER) {
            stmt = parse_assignment();
        } else if (peek()->type == TOKEN_IF) {
            stmt = parse_if();
        } else if (peek()->type == TOKEN_ENDCODE) {
            consume();
            break;
        } else {
            printf("Unknown token: %s\n", peek()->text);
            break;
        }

        if (stmt) {
            if (head == NULL) head = stmt;
            else last->next = stmt;
            last = stmt;
        }
    }
    return head;
}

void print_ast(Node* node, int indent) {
    while (node) {
        for (int i=0; i<indent; i++) printf("  ");
        switch(node->type) {
            case NODE_ASSIGN:
                printf("Assign: %s = %d\n", node->var_name, node->expr->literal_value);
                break;
            case NODE_IF:
                printf("If: %s == %d\n", node->condition->var_name, node->expr->literal_value);
                print_ast(node->then_branch, indent + 1);
                break;
            case NODE_ECHO:
                printf("Echo: \"%s\"\n", node->var_name);
                break;
            default:
                printf("Unknown node\n");
        }
        node = node->next;
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* buffer = malloc(len + 1);
    fread(buffer, 1, len, f);
    buffer[len] = '\0';
    fclose(f);

    // 1. Lexer
    lex(buffer);

    // 2. Debug print token (boleh di-comment jika sudah yakin)
    const char* token_names[] = {
        "TOKEN_INTEGER", "TOKEN_IDENTIFIER", "TOKEN_ASSIGN",
        "TOKEN_LITERAL", "TOKEN_IF", "TOKEN_LPAREN",
        "TOKEN_RPAREN", "TOKEN_EQUAL", "TOKEN_LBRACE",
        "TOKEN_RBRACE", "TOKEN_STRING", "TOKEN_ENDCODE", "TOKEN_EOF"
    };
    for (int i = 0; i < token_count; i++) {
        printf("Token: %-15s | Text: %s\n", token_names[tokens[i].type], tokens[i].text);
    }

    // 3. Parser
    current_token = 0; // reset indeks parser
    Node* ast = parse_program();

    // 4. Print AST hasil parse
    printf("=== AST ===\n");
    print_ast(ast, 0);

    free(buffer);
    return 0;
}




