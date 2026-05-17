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
    bool emitJson  = false;
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
        "  cvm --trace <file.cvm>    trace VM dispatch\n"
        "  cvm --json <file.cvm>     emit tokens/AST/bytecode/output as JSON\n";
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

// ── JSON mode ──────────────────────────────────────
// Captures every pipeline stage into one JSON blob on stdout.
// Stops early on the first failing stage and records the error.
static int runJsonMode(const std::string& src) {
    std::ostringstream out;
    out << "{";

    bool ok = true;
    std::string errStage, errMsg;
    std::vector<Token> tokens;
    NodePtr program;
    std::vector<uint8_t> bytecode;
    std::vector<std::string> vars;
    std::ostringstream programOut;

    try {
        Lexer lexer(src);
        tokens = lexer.tokenize();
    } catch (const std::exception& e) {
        ok = false; errStage = "lexer"; errMsg = e.what();
    }

    if (ok) {
        try {
            std::vector<Token> copy = tokens;
            Parser parser(std::move(copy));
            program = parser.parseProgram();
        } catch (const std::exception& e) {
            ok = false; errStage = "parser"; errMsg = e.what();
        }
    }

    Compiler compiler;
    if (ok) {
        try {
            bytecode = compiler.compile(program.get());
            vars = compiler.getVars();
        } catch (const std::exception& e) {
            ok = false; errStage = "compiler"; errMsg = e.what();
        }
    }

    // Capture VM stdout so program prints land in the JSON instead of mingling.
    if (ok) {
        std::streambuf* oldCout = std::cout.rdbuf(programOut.rdbuf());
        try {
            VM vm;
            vm.run(bytecode, vars.size(), false);
        } catch (const std::exception& e) {
            ok = false; errStage = "vm"; errMsg = e.what();
        }
        std::cout.rdbuf(oldCout);
    }

    out << "\"ok\":" << (ok ? "true" : "false");

    // tokens
    out << ",\"tokens\":[";
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i) out << ",";
        out << "{\"type\":\"" << tokenTypeName(tokens[i].type) << "\""
            << ",\"value\":\"" << jsonEscape(tokens[i].value) << "\""
            << ",\"line\":" << tokens[i].line << "}";
    }
    out << "]";

    // ast
    out << ",\"ast\":";
    if (program) astToJson(program.get(), out);
    else         out << "null";

    // bytecode
    out << ",\"bytecode\":";
    if (!bytecode.empty()) bytecodeToJson(bytecode, vars, out);
    else                   out << "null";

    // captured output
    out << ",\"output\":\"" << jsonEscape(programOut.str()) << "\"";

    // error
    if (!ok) {
        out << ",\"error\":{\"stage\":\"" << errStage << "\""
            << ",\"message\":\"" << jsonEscape(errMsg) << "\"}";
    } else {
        out << ",\"error\":null";
    }
    out << "}";

    std::cout << out.str();
    return 0;
}

static int runFile(const Options& opt) {
    std::ifstream in(opt.file);
    if (!in) {
        if (opt.emitJson) {
            std::cout << "{\"ok\":false,\"error\":{\"stage\":\"io\","
                         "\"message\":\"cannot open file\"}}";
            return 0;
        }
        std::cerr << "Cannot open: " << opt.file << "\n"; return 1;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    if (opt.emitJson) return runJsonMode(ss.str());
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
        else if (a == "--json")  opt.emitJson = true;
        else if (a == "-h" || a == "--help") { usage(); return 0; }
        else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown flag: " << a << "\n"; usage(); return 1;
        }
        else opt.file = a;
    }
    if (opt.file.empty()) {
        if (opt.emitJson) {
            // read code from stdin
            std::stringstream ss;
            ss << std::cin.rdbuf();
            return runJsonMode(ss.str());
        }
        return repl();
    }
    return runFile(opt);
}
