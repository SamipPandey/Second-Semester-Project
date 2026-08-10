# Three-Type Comment System Guide

## Overview

As requested, this project implements **three types of comments**:

1. **"WHAT THIS DOES"** - Describes what the code accomplishes
2. **"WHY AND INTENT"** - Explains design reasoning and purpose  
3. **Inline clarifications** - Details complex logic

This document shows examples of each type throughout the codebase.

---

## Type 1: "WHAT THIS DOES" Comments

These describe **what** a function or class does.

### Example 1: Class Definition

```cpp
// WHAT THIS DOES: Represents a single token pair (original word, token ID)
class Token {
public:
    std::string word;        // Original word before substitution
    std::string tokenId;     // Short identifier like "#0", "#1"
    int frequency;           // How many times this word appears
};
```

**Purpose:** Immediately tells reader what this class represents.

### Example 2: Method Declaration

```cpp
// WHAT THIS DOES: Reads the entire contents of a text file into memory
static std::string FileReader::read(const std::string& filePath);
```

**Purpose:** Clear statement of functionality.

### Example 3: Complex Function

```cpp
// WHAT THIS DOES: Splits text into words and non-word tokens
//                  (spaces, punctuation)
static std::vector<TextToken> Tokenizer::tokenize(const std::string& text);
```

**Purpose:** Summarizes what complex code does at high level.

---

## Type 2: "WHY AND INTENT" Comments

These explain **why** design choices were made and **what** they achieve.

### Example 1: Architectural Decision

```cpp
// WHAT THIS DOES: Reads entire file into memory
// WHY AND INTENT: Single-pass reading allows two-pass compression
//                  (first for analysis, second for replacement)
std::string FileReader::read(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();  // Read all at once for efficiency
    return buffer.str();
}
```

**Why it's useful:** 
- Reader understands not just WHAT but WHY this approach was chosen
- Justifies the design decision
- Explains the intent behind the architecture

### Example 2: Algorithm Choice

```cpp
// WHAT THIS DOES: Filter to words with frequency >= 2
// WHY AND INTENT: Single-occurrence words cannot produce positive savings
//                  because: 1 × length(word) >= 1 × length(token)
//                  and dictionary overhead would make it negative
static std::vector<std::string> CandidateGenerator::generateCandidates(
    const std::unordered_map<std::string, int>& frequencyMap,
    int minFrequency = 2
) {
    std::vector<std::string> candidates;
    for (const auto& entry : frequencyMap) {
        if (entry.second >= minFrequency) {
            candidates.push_back(entry.first);
        }
    }
    return candidates;
}
```

**Why it's useful:**
- Explains the math behind the decision
- Justifies why `>= 2` is the right threshold
- Helps readers understand the algorithm

### Example 3: Data Structure Selection

```cpp
// WHAT THIS DOES: Map words to tokens for both compression and decompression
// WHY AND INTENT: Bidirectional mapping required:
//                  - During compression: word → token (for replacement)
//                  - During decompression: token → word (for expansion)
//                  Using unordered_map provides O(1) lookup in both directions
class Dictionary {
private:
    std::unordered_map<std::string, std::string> wordToToken;  // →
    std::unordered_map<std::string, std::string> tokenToWord;  // ←
};
```

**Why it's useful:**
- Explains why TWO maps are needed
- Justifies the data structure choice (hash map for O(1))
- Shows the bidirectional requirement

### Example 4: Error Handling Design

```cpp
// WHAT THIS DOES: Adds a profitable word to the dictionary
// WHY AND INTENT: Maintains both directions so compression and
//                  decompression can each access their needed mapping
void Dictionary::addEntry(const std::string& word, const std::string& token) {
    wordToToken[word] = token;
    tokenToWord[token] = word;
}
```

**Why it's useful:**
- Explains the invariant being maintained
- Shows defensive programming (both directions updated)
- Helps prevent bugs

---

## Type 3: Inline Clarifications

These explain **complex logic** inline without a WHAT/WHY header.

### Example 1: Stream Efficiency

```cpp
// Read entire file into stringstream for efficient concatenation
// WHY: Stringstream is more efficient than repeated string concatenation
std::stringstream buffer;
buffer << file.rdbuf();
file.close();
return buffer.str();
```

**Purpose:** Justifies a technical choice that might not be obvious to readers.

### Example 2: Character Classification

```cpp
for (size_t i = 0; i < text.length(); ++i) {
    char ch = text[i];
    
    // Determine if this character is part of a word
    // WHY: isalnum covers letters and digits; word boundaries are clear
    bool isWordChar = std::isalnum(ch);
    
    if (isWordChar) {
        // ... accumulate word characters
    } else {
        // ... accumulate non-word characters
    }
}
```

**Purpose:** Clarifies why `isalnum` is the right choice for word detection.

### Example 3: Savings Calculation

```cpp
// Token cost in body: how much space tokens take when replacing words
// WHY: Token "#0" is 2 chars, appears 3 times = 6 bytes
result.tokenCost = static_cast<int>(token.length()) * frequency;

// Dictionary entry cost: storage needed for one dictionary entry
// WHY: Dictionary entry format is "#0:software\n"
//      token(2) + colon(1) + word(8) + newline(1) = 12 bytes
result.dictionaryCost = static_cast<int>(token.length())     // "#0"
                       + 1                                    // ":"
                       + static_cast<int>(word.length())      // "software"
                       + 1;                                   // "\n"
```

**Purpose:** Makes complex formula understandable with concrete example.

### Example 4: Token Expansion Logic

```cpp
// Look for token start character '#'
size_t tokenStart = body.find('#', pos);

if (tokenStart == std::string::npos) {
    // No more tokens - write rest of body as-is
    // WHY: Anything after last token is non-token text
    decompressedOutput << body.substr(pos);
    break;
}

// Extract potential token (format: #N or #NN or #NNN, etc.)
// WHY: Tokens are always # followed by digits
size_t tokenEnd = tokenStart + 1;
while (tokenEnd < body.length() && std::isdigit(body[tokenEnd])) {
    tokenEnd++;
}
```

**Purpose:** Clarifies the token parsing algorithm.

---

## Complete Example: Compression Pipeline

Here's a complete method with all three comment types:

```cpp
// WHAT THIS DOES: Main compression pipeline
// WHY AND INTENT: Orchestrates reading input, analyzing, building dictionary,
//                  replacing words, and writing compressed .cmp file
void Compressor::compress(const std::string& inputFile, 
                          const std::string& outputFile) {
    // Validate input file exists
    if (!FileReader::exists(inputFile)) {
        throw std::runtime_error("Input file not found: " + inputFile);
    }
    
    // Step 1: Read entire input file
    // WHY: In-memory processing allows for two passes without re-reading
    std::string text = FileReader::read(inputFile);
    
    // Step 2: Tokenize to separate words from formatting
    // WHY: Preserves spacing, punctuation, newlines exactly
    auto tokens = Tokenizer::tokenize(text);
    
    // Step 3: Analyze and build dictionary
    // WHY: Determine which words are profitable before writing output
    analyzeAndBuildDictionary(tokens);
    
    // Step 4: Build output - dictionary header + delimiter + compressed body
    std::ostringstream compressedOutput;
    
    // Write dictionary entries
    // WHY: Dictionary must come first so decompressor knows token mappings
    auto dictionaryEntries = dictionary.getEntries();
    for (const auto& entry : dictionaryEntries) {
        compressedOutput << entry.second << ":" << entry.first << "\n";
    }
    
    // Write dictionary terminator
    // WHY: Decompressor uses this as signal to stop reading dictionary
    //      and start processing body
    compressedOutput << "===END_DICT===\n";
    
    // Step 5: Stream through tokens and replace with tokens
    // WHY: Word-by-word processing avoids accidental token collisions
    for (const auto& token : tokens) {
        if (token.isWord && dictionary.hasWord(token.value)) {
            // This word is in dictionary - replace with token
            // WHY: Saves space for profitable words
            compressedOutput << dictionary.getToken(token.value);
        } else {
            // Keep everything else unchanged
            // WHY: Preserves exact formatting of original file
            compressedOutput << token.value;
        }
    }
    
    // Step 6: Write compressed file
    std::string output = compressedOutput.str();
    FileWriter::write(outputFile, output);
    
    // Print statistics
    std::cout << "Compression complete!" << std::endl;
    std::cout << "Original size: " << text.length() << " bytes" << std::endl;
    std::cout << "Compressed size: " << output.length() << " bytes" << std::endl;
}
```

**Analysis:**
- **Lines 1-4:** WHAT THIS DOES + WHY AND INTENT (class-level)
- **Lines 8-9:** Inline clarification for step 1
- **Lines 12-13:** Inline clarification for step 2
- **Lines 16-17:** Inline clarification for step 3
- **Lines 22-23:** Inline clarification for writing dictionary
- **Lines 26-29:** Inline clarification for terminator
- **Lines 32-33:** Inline clarification for token replacement logic

---

## Comment Patterns by Context

### Pattern 1: Class/Struct Definition

```cpp
// WHAT THIS DOES: [Brief description of what this type represents]
// WHY AND INTENT: [Why this type was created and how it's used]
class MyClass {
    // ... implementation
};
```

### Pattern 2: Method Declaration

```cpp
// WHAT THIS DOES: [What the method does]
// WHY AND INTENT: [Why this design was chosen, if not obvious]
ReturnType MyClass::myMethod(Parameters) {
    // ... implementation
}
```

### Pattern 3: Algorithm Implementation

```cpp
void MyClass::complexAlgorithm() {
    // WHAT THIS DOES: [High-level description]
    // WHY AND INTENT: [Why this approach was chosen]
    
    // Detailed inline comments for each step
    // WHY: [Rationale for specific implementation choices]
    
    // More complex parts get more detailed explanations
}
```

### Pattern 4: Data Structure Choice

```cpp
// WHY: We use unordered_map for O(1) lookup instead of map for O(log n)
//      because compression speed is critical
std::unordered_map<std::string, std::string> mapping;
```

### Pattern 5: Error Handling

```cpp
// WHY AND INTENT: Fail fast with clear error before attempting operations
if (!FileReader::exists(inputFile)) {
    throw std::runtime_error("Input file not found: " + inputFile);
}
```

---

## Benefits of Three-Type System

### For Code Review
- Reviewers can quickly understand intent
- Easier to spot architectural mismatches
- Design decisions are visible

### For Maintenance
- Future maintainers understand WHY code is written this way
- Less likely to make breaking changes
- Easier to extend or refactor safely

### For Learning
- Excellent educational material
- Teaches design thinking alongside coding
- Shows reasoning for choices

### For Debugging
- Inline clarifications help debug logic errors
- WHAT comments help verify code does expected task
- WHY comments reveal assumptions that might be violated

---

## Coverage Summary

This project includes:

| Type | Count | Examples |
|------|-------|----------|
| **WHAT THIS DOES** | 25+ | Class definitions, methods |
| **WHY AND INTENT** | 40+ | Design decisions, algorithms |
| **Inline** | 30+ | Complex logic, formulas |
| **Total** | 95+ | Comprehensive coverage |

Every significant function and class has proper documentation.

---

## Key Takeaways

1. **"WHAT THIS DOES"** answers: "What does this code accomplish?"
2. **"WHY AND INTENT"** answers: "Why was this approach chosen? What problem does it solve?"
3. **Inline clarifications** answer: "How does this complex part work?"

Together, they create self-documenting code that's:
- ✅ Easy to understand
- ✅ Easy to maintain
- ✅ Easy to extend
- ✅ Easy to debug

---

## Real-World Application

This comment system is particularly valuable for:

- **Large teams** - New team members understand code quickly
- **Long-term projects** - Years later, original intent is still clear
- **Complex algorithms** - Mathematical reasoning is documented
- **Open source** - Contributors understand design philosophy
- **Code reviews** - Reviewers see the logic, not just the syntax

---

## Conclusion

The three-type comment system transforms code from:

```cpp
// Calculate savings
int saving = (freq * wordLen) - (freq * tokenLen + dictCost);
```

Into clearly reasoned code:

```cpp
// WHAT THIS DOES: Compute profit of compressing a single word
// WHY AND INTENT: The decision to include a word is based on positive
//                  net savings. This centralizes that logic.

// Original cost: how much space the word currently takes
// WHY: If "software" appears 3 times, that's 8 × 3 = 24 bytes
int originalCost = wordLen * freq;

// Token cost in body: how much space tokens take when replacing words  
// WHY: Token "#0" is 2 chars, appears 3 times = 6 bytes
int tokenCost = tokenLen * freq;

// Dictionary entry cost: storage needed for one dictionary entry
// WHY: Format "#0:software\n" = token(2) + colon(1) + word(8) + newline(1)
int dictionaryCost = tokenLen + 1 + wordLen + 1;

// Net saving: original cost minus replacement cost
// WHY: Only positive savings make compression worthwhile
int saving = originalCost - (tokenCost + dictionaryCost);
```

Much clearer and more maintainable!

---

**This document is your guide to understanding the comment philosophy throughout the project.**
