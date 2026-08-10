#ifndef DICTIONARY_COMPRESSOR_H
#define DICTIONARY_COMPRESSOR_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

// ============================================================================
// COMMENT SYSTEM:
// 1. "WHAT THIS DOES" - Describes what the function/class accomplishes
// 2. "WHY AND INTENT" - Explains the reasoning behind design choices
// 3. Inline clarifications for complex logic
// ============================================================================

// ============================================================================
// CLASS: Token
// WHAT THIS DOES: Represents a single token pair (original word, token ID)
// WHY AND INTENT: Encapsulates the mapping of words to short token IDs
//                  to keep token data cohesive and type-safe
// ============================================================================
class Token {
public:
    std::string word;        // Original word before substitution
    std::string tokenId;     // Short identifier like "#0", "#1"
    int frequency;           // How many times this word appears
    
    Token(const std::string& w, const std::string& id, int freq)
        : word(w), tokenId(id), frequency(freq) {}
};

// ============================================================================
// CLASS: FileReader
// WHAT THIS DOES: Reads the entire contents of a text file into memory
// WHY AND INTENT: Centralizes file I/O to handle encoding, errors, and
//                  missing files consistently across the project
// ============================================================================
class FileReader {
public:
    // WHAT THIS DOES: Reads a file and returns its contents as a string
    // WHY AND INTENT: Simplifies file access and provides error reporting
    static std::string read(const std::string& filePath);
    
    // WHAT THIS DOES: Checks if a file exists before attempting to open it
    // WHY AND INTENT: Prevents cryptic error messages by validating early
    static bool exists(const std::string& filePath);
};

// ============================================================================
// CLASS: FileWriter
// WHAT THIS DOES: Writes data to files with error checking
// WHY AND INTENT: Ensures safe file output and handles I/O exceptions
//                  to prevent silent corruption of output files
// ============================================================================
class FileWriter {
public:
    // WHAT THIS DOES: Writes text content to a file
    // WHY AND INTENT: Provides error-safe writing with exception handling
    static void write(const std::string& filePath, const std::string& content);
};

// ============================================================================
// CLASS: Tokenizer
// WHAT THIS DOES: Splits text into words and non-word tokens (spaces, punctuation)
// WHY AND INTENT: Preserves spacing and punctuation structure so the
//                  decompressed output exactly matches the original
// ============================================================================
class Tokenizer {
public:
    // WHAT THIS DOES: Represents a single unit of text (word, space, or punctuation)
    struct TextToken {
        bool isWord;         // True if this is a word, False if whitespace/punctuation
        std::string value;   // The actual text (" ", "word", ".", etc.)
    };
    
    // WHAT THIS DOES: Converts raw text into a sequence of TextTokens
    // WHY AND INTENT: Separates words from formatting so we can replace
    //                  words without affecting spacing or punctuation
    static std::vector<TextToken> tokenize(const std::string& text);
};

// ============================================================================
// CLASS: FrequencyAnalyzer
// WHAT THIS DOES: Counts how many times each word appears in the text
// WHY AND INTENT: Identifies candidates for dictionary replacement by
//                  finding words that repeat (frequency >= 2)
// ============================================================================
class FrequencyAnalyzer {
public:
    // WHAT THIS DOES: Analyzes tokens and returns a map of word → count
    // WHY AND INTENT: Unordered_map provides O(1) lookup when checking
    //                  if a word should be included in the dictionary
    static std::unordered_map<std::string, int> analyze(
        const std::vector<Tokenizer::TextToken>& tokens
    );
};

// ============================================================================
// CLASS: CandidateGenerator
// WHAT THIS DOES: Identifies words that might benefit from compression
// WHY AND INTENT: Filters out single-occurrence words early to avoid
    //                  unnecessary profitability calculations on obvious
    //                  non-candidates (optimization)
// ============================================================================
class CandidateGenerator {
public:
    // WHAT THIS DOES: Returns words with frequency >= 2
    // WHY AND INTENT: A word that appears only once cannot produce positive
    //                  savings because: 1 × length(word) >= 1 × length(token)
    //                  and dictionary overhead would make it negative
    static std::vector<std::string> generateCandidates(
        const std::unordered_map<std::string, int>& frequencyMap,
        int minFrequency = 2
    );
};

// ============================================================================
// CLASS: SavingsCalculator
// WHAT THIS DOES: Determines if replacing a word with a token saves space
// WHY AND INTENT: The core compression decision logic - we only add words
    //                  to the dictionary if they produce positive net savings
// ============================================================================
class SavingsCalculator {
public:
    // WHAT THIS DOES: Calculates savings for a single word candidate
    // WHY AND INTENT: Implements the profit formula:
    //                  originalCost = freq × wordLen
    //                  tokenCost = freq × tokenLen + dictEntryLen
    //                  savings = originalCost - tokenCost
    //                  Only positive savings are kept
    struct SavingsResult {
        std::string word;
        int frequency;
        int originalCost;
        int tokenCost;
        int dictionaryCost;
        int netSaving;
        bool isProfitable;
    };
    
    // WHAT THIS DOES: Computes all costs for a candidate word
    // WHY AND INTENT: Centralizes savings logic so it's consistent and
    //                  easy to audit/modify if compression rules change
    static SavingsResult calculateSavings(
        const std::string& word,
        int frequency,
        const std::string& token
    );
};

// ============================================================================
// CLASS: TokenAllocator
// WHAT THIS DOES: Generates unique token IDs (#0, #1, #2, etc.)
// WHY AND INTENT: Ensures tokens are sequential and prevent collisions.
    //                  Tokens are short to save space (critical for savings)
// ============================================================================
class TokenAllocator {
private:
    int nextId;  // Next token ID to assign
    
public:
    TokenAllocator() : nextId(0) {}
    
    // WHAT THIS DOES: Returns the next sequential token ID
    // WHY AND INTENT: Guarantees each word gets a unique, compact token
    //                  like #0, #1, #2, etc.
    std::string nextToken();
    
    // WHAT THIS DOES: Resets the allocator for a fresh compression run
    // WHY AND INTENT: Allows reusing the allocator in tests without
    //                  creating new objects
    void reset();
};

// ============================================================================
// CLASS: Dictionary
// WHAT THIS DOES: Maps words to tokens and vice versa
// WHY AND INTENT: Serves as the central data structure for both compression
    //                  (word → token) and decompression (token → word)
// ============================================================================
class Dictionary {
private:
    std::unordered_map<std::string, std::string> wordToToken;  // word → token lookup
    std::unordered_map<std::string, std::string> tokenToWord;  // token → word lookup
    
public:
    // WHAT THIS DOES: Adds a profitable word to the dictionary
    // WHY AND INTENT: Maintains both directions so compression and
    //                  decompression can each access their needed mapping
    void addEntry(const std::string& word, const std::string& token);
    
    // WHAT THIS DOES: Retrieves the token for a given word
    // WHY AND INTENT: Used during compression to replace words
    std::string getToken(const std::string& word) const;
    
    // WHAT THIS DOES: Retrieves the original word for a given token
    // WHY AND INTENT: Used during decompression to restore original text
    std::string getWord(const std::string& token) const;
    
    // WHAT THIS DOES: Checks if a word exists in the dictionary
    // WHY AND INTENT: Prevents attempting to get tokens for unknown words
    bool hasWord(const std::string& word) const;
    
    // WHAT THIS DOES: Checks if a token exists in the dictionary
    // WHY AND INTENT: Prevents attempting to expand unknown tokens
    bool hasToken(const std::string& token) const;
    
    // WHAT THIS DOES: Returns all word-token pairs
    // WHY AND INTENT: Needed for writing the dictionary header to file
    std::vector<std::pair<std::string, std::string>> getEntries() const;
    
    // WHAT THIS DOES: Clears all entries for fresh compression
    // WHY AND INTENT: Allows dictionary to be reused across multiple
    //                  compression operations
    void clear();
    
    // WHAT THIS DOES: Returns the number of entries in the dictionary
    // WHY AND INTENT: Useful for statistics and debugging
    int size() const;
};

// ============================================================================
// CLASS: Compressor
// WHAT THIS DOES: Orchestrates the entire compression pipeline
// WHY AND INTENT: Brings together all components (analysis, dictionary building,
    //                  token replacement) into a single, testable workflow
// ============================================================================
class Compressor {
private:
    Dictionary dictionary;
    TokenAllocator tokenAllocator;
    
    // WHAT THIS DOES: First pass - analyze and build dictionary
    // WHY AND INTENT: Separates analysis from compression so we know
    //                  exactly which words are profitable before writing output
    void analyzeAndBuildDictionary(const std::vector<Tokenizer::TextToken>& tokens);
    
public:
    // WHAT THIS DOES: Compresses a text file into a .cmp file
    // WHY AND INTENT: Main entry point for compression. Reads input,
    //                  analyzes frequency, builds dictionary, replaces words,
    //                  and writes .cmp output with header and body sections
    void compress(const std::string& inputFile, const std::string& outputFile);
    
    // WHAT THIS DOES: Returns the built dictionary for inspection
    // WHY AND INTENT: Useful for debugging and testing compression results
    const Dictionary& getDictionary() const { return dictionary; }
};

// ============================================================================
// CLASS: DictionaryParser
// WHAT THIS DOES: Reads a .cmp file header and reconstructs the dictionary
// WHY AND INTENT: Decompression depends on first parsing the header to
    //                  learn which tokens map to which words, then using
    //                  that mapping to expand the body
// ============================================================================
class DictionaryParser {
public:
    // WHAT THIS DOES: Parses dictionary header and builds token → word mapping
    // WHY AND INTENT: Reads lines until ===END_DICT=== is found, extracting
    //                  token:word pairs. Uses stream abstraction for testability
    static Dictionary parseDictionary(std::istringstream& stream);
};

// ============================================================================
// CLASS: Decompressor
// WHAT THIS DOES: Orchestrates the decompression pipeline
// WHY AND INTENT: Reverses compression by parsing the dictionary header,
    //                  then scanning the body and expanding recognized tokens
// ============================================================================
class Decompressor {
public:
    // WHAT THIS DOES: Decompresses a .cmp file back to original text
    // WHY AND INTENT: Main entry point for decompression. Reads .cmp file,
    //                  parses dictionary, scans body for tokens, expands them,
    //                  and writes restored text to output file
    void decompress(const std::string& inputFile, const std::string& outputFile);
};

// ============================================================================
// CLASS: CompressionStatistics
// WHAT THIS DOES: Calculates and reports compression metrics
// WHY AND INTENT: Provides visibility into compression effectiveness so
    //                  users can see how much space was saved
// ============================================================================
class CompressionStatistics {
public:
    long originalSize;
    long compressedSize;
    int dictionaryEntries;
    
    CompressionStatistics()
        : originalSize(0), compressedSize(0), dictionaryEntries(0) {}
    
    // WHAT THIS DOES: Computes compression ratio and displays results
    // WHY AND INTENT: Gives user feedback on compression effectiveness
    void print() const;
};

#endif // DICTIONARY_COMPRESSOR_H
