#pragma once
#include <vector>
#include <string>
#include <cstdint>

// ── Tagged Value ───────────────────────────────────
// Lets `print` show booleans as true/false vs ints.
struct Value {
    enum Type : uint8_t { INT, BOOL } type;
    int32_t i;
    static Value mkInt(int32_t v)  { return {INT,  v}; }
    static Value mkBool(bool v)    { return {BOOL, v ? 1 : 0}; }
    bool truthy() const { return i != 0; }
};

class VM {
public:
    // Returns the last value popped (or 0/INT if none).
    Value run(const std::vector<uint8_t>& code,
              size_t numVars,
              bool trace = false);

private:
    std::vector<Value> stack;
    std::vector<Value> globals;
};
