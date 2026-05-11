#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "opcodes.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

struct Options {
    bool showAst   = false;
    bool showBc    = false;
    bool trace     = false;
    std::string file;
};

static void usage() {
    std::cout <<
        "CVM++ — toy stack VM\n"
        "Usage:\n"
        "  cvm                       start REPL\n"
        "  cvm <file.cvm>            run file\n"
        "  cvm --ast <file.cvm>      print AST\n"
        "  cvm --bc  <file.cvm>      print bytecode\n"
        "  cvm --debug <file.cvm>    print AST + bytecode then run\n"
        "  cvm --trace <file.cvm>    trace VM dispatch\n";
}

static int runSource(const std::string& src, const Options& opt) {
    try {
        Lexer lexer(src);
        auto tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        auto program = parser.parseProgram();

        if (opt.showAst) {
            std::cout << "=== AST ===\n";
            printAst(program.get());
            std::cout << "=== end ===\n";
        }

        Compiler compiler;
        auto bytecode = compiler.compile(program.get());

        if (opt.showBc) {
            disassemble(bytecode, compiler.getVars());
        }

        VM vm;
        Value result = vm.run(bytecode, compiler.getVars().size(), opt.trace);
        (void)result;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

static int runFile(const Options& opt) {
    std::ifstream in(opt.file);
    if (!in) { std::cerr << "Cannot open: " << opt.file << "\n"; return 1; }
    std::stringstream ss;
    ss << in.rdbuf();
    return runSource(ss.str(), opt);
}

static int repl() {
    std::cout << "CVM++ REPL — type 'exit' to quit.\n"
                 "Each line is a complete program; statements end with ';'.\n";
    std::string line;
    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        Options opt;
        runSource(line, opt);
    }
    return 0;
}

int main(int argc, char** argv) {
    Options opt;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--ast")   opt.showAst = true;
        else if (a == "--bc")    opt.showBc  = true;
        else if (a == "--debug") { opt.showAst = true; opt.showBc = true; }
        else if (a == "--trace") opt.trace = true;
        else if (a == "-h" || a == "--help") { usage(); return 0; }
        else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown flag: " << a << "\n"; usage(); return 1;
        }
        else opt.file = a;
    }
    if (opt.file.empty()) return repl();
    return runFile(opt);
}
