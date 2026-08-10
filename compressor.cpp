#include "compressor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>

// ============================================================================
// IMPLEMENTATION: FileReader
// ============================================================================

// WHAT THIS DOES: Reads entire file contents into a string
// WHY AND INTENT: Centralizes file I/O so errors are handled consistently
std::string FileReader::read(const std::string& filePath) {
    // Open file and check for failures
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("ERROR: Cannot open file: " + filePath);
    }
    
    // Read entire file into stringstream for efficient concatenation
    // WHY: Stringstream is more efficient than repeated string concatenation
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

// WHAT THIS DOES: Check if a file exists on disk
// WHY AND INTENT: Fail fast with clear error before attempting operations
bool FileReader::exists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

// ============================================================================
// IMPLEMENTATION: FileWriter
// ============================================================================

// WHAT THIS DOES: Write text to a file with error checking
// WHY AND INTENT: Ensures output files are created successfully and
    //                  throws clear errors if output cannot be written
void FileWriter::write(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("ERROR: Cannot create output file: " + filePath);
    }
    
    file << content;
    file.close();
    
    if (file.fail()) {
        throw std::runtime_error("ERROR: Failed to write to file: " + filePath);
    }
}

// ============================================================================
// IMPLEMENTATION: Tokenizer
// ============================================================================

// WHAT THIS DOES: Split text into words and whitespace/punctuation tokens
// WHY AND INTENT: Preserves exact spacing and punctuation so decompressed
//                  output matches original character-for-character (except words)
std::vector<Tokenizer::TextToken> Tokenizer::tokenize(const std::string& text) {
    std::vector<TextToken> tokens;
    
    std::string current;
    bool currentIsWord = false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char ch = text[i];
        
        // Determine if this character is part of a word
        // WHY: isalnum covers letters and digits; word boundaries are clear
        bool isWordChar = std::isalnum(ch);
        
        if (isWordChar) {
            // If we were accumulating non-word characters, save them first
            if (!current.empty() && !currentIsWord) {
                tokens.push_back({false, current});
                current.clear();
            }
            
            // Accumulate word characters
            current += ch;
            currentIsWord = true;
        } else {
            // Non-word character (space, punctuation, tab, newline)
            // If we were accumulating word characters, save the word first
            if (!current.empty() && currentIsWord) {
                tokens.push_back({true, current});
                current.clear();
            }
            
            // Accumulate non-word characters
            current += ch;
            currentIsWord = false;
        }
    }
    
    // Don't forget the last token
    if (!current.empty()) {
        tokens.push_back({currentIsWord, current});
    }
    
    return tokens;
}

// ============================================================================
// IMPLEMENTATION: FrequencyAnalyzer
// ============================================================================

// WHAT THIS DOES: Count occurrences of each word in the token stream
// WHY AND INTENT: Frequency data drives candidate selection - words with
//                  frequency >= 2 are worth analyzing for compression
std::unordered_map<std::string, int> FrequencyAnalyzer::analyze(
    const std::vector<Tokenizer::TextToken>& tokens
) {
    std::unordered_map<std::string, int> frequency;
    
    for (const auto& token : tokens) {
        // Only count words, not whitespace/punctuation
        if (token.isWord) {
            frequency[token.value]++;
        }
    }
    
    return frequency;
}

// ============================================================================
// IMPLEMENTATION: CandidateGenerator
// ============================================================================

// WHAT THIS DOES: Filter words that repeat at least minFrequency times
// WHY AND INTENT: Single-occurrence words cannot produce positive savings
//                  because the dictionary overhead would exceed savings.
//                  This filter prevents wasted profitability calculations
std::vector<std::string> CandidateGenerator::generateCandidates(
    const std::unordered_map<std::string, int>& frequencyMap,
    int minFrequency
) {
    std::vector<std::string> candidates;
    
    for (const auto& entry : frequencyMap) {
        // WHY: Only include words that repeat; single-occurrence words
        //      cannot provide positive net savings due to dictionary overhead
        if (entry.second >= minFrequency) {
            candidates.push_back(entry.first);
        }
    }
    
    return candidates;
}

// ============================================================================
// IMPLEMENTATION: SavingsCalculator
// ============================================================================

// WHAT THIS DOES: Compute profit of compressing a single word
// WHY AND INTENT: The decision to include a word in the dictionary is based
//                  on whether net savings > 0. This centralizes that logic
SavingsCalculator::SavingsResult SavingsCalculator::calculateSavings(
    const std::string& word,
    int frequency,
    const std::string& token
) {
    SavingsResult result;
    result.word = word;
    result.frequency = frequency;
    
    // Original cost: how much space the word currently takes
    // WHY: If "software" appears 3 times, that's 8 * 3 = 24 bytes
    result.originalCost = static_cast<int>(word.length()) * frequency;
    
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
    
    // Total replacement cost: tokens in body + dictionary overhead
    // WHY: 6 bytes (tokens) + 12 bytes (dictionary) = 18 total
    int replacementCost = result.tokenCost + result.dictionaryCost;
    
    // Net saving: original cost minus replacement cost
    // WHY: 24 - 18 = 6 bytes saved. Only positive savings are profitable
    result.netSaving = result.originalCost - replacementCost;
    
    // Decision: profitable only if net saving is positive
    // WHY: We only compress if we actually save space; otherwise we're
    //      just adding dictionary overhead with no benefit
    result.isProfitable = result.netSaving > 0;
    
    return result;
}

// ============================================================================
// IMPLEMENTATION: TokenAllocator
// ============================================================================

// WHAT THIS DOES: Generate the next sequential token ID
// WHY AND INTENT: Tokens like #0, #1, #2 are compact and sequential,
//                  making them easy to allocate and track
std::string TokenAllocator::nextToken() {
    return "#" + std::to_string(nextId++);
}

// WHAT THIS DOES: Reset allocator for reuse
// WHY AND INTENT: Allows tests and multiple compression runs without
//                  recreating the allocator object
void TokenAllocator::reset() {
    nextId = 0;
}

// ============================================================================
// IMPLEMENTATION: Dictionary
// ============================================================================

// WHAT THIS DOES: Add a word-token pair to both lookup maps
// WHY AND INTENT: Maintains bidirectional mapping for compression (word → token)
//                  and decompression (token → word)
void Dictionary::addEntry(const std::string& word, const std::string& token) {
    wordToToken[word] = token;
    tokenToWord[token] = word;
}

// WHAT THIS DOES: Retrieve token for a word during compression
// WHY AND INTENT: Used to replace words in the text body
std::string Dictionary::getToken(const std::string& word) const {
    auto it = wordToToken.find(word);
    if (it != wordToToken.end()) {
        return it->second;
    }
    throw std::runtime_error("Word not in dictionary: " + word);
}

// WHAT THIS DOES: Retrieve original word for a token during decompression
// WHY AND INTENT: Used to restore tokens back to words
std::string Dictionary::getWord(const std::string& token) const {
    auto it = tokenToWord.find(token);
    if (it != tokenToWord.end()) {
        return it->second;
    }
    throw std::runtime_error("Token not in dictionary: " + token);
}

// WHAT THIS DOES: Check if word exists in dictionary
// WHY AND INTENT: Prevents errors when looking up unknown words
bool Dictionary::hasWord(const std::string& word) const {
    return wordToToken.find(word) != wordToToken.end();
}

// WHAT THIS DOES: Check if token exists in dictionary
// WHY AND INTENT: Prevents errors when expanding unknown tokens
bool Dictionary::hasToken(const std::string& token) const {
    return tokenToWord.find(token) != tokenToWord.end();
}

// WHAT THIS DOES: Return all word-token pairs
// WHY AND INTENT: Needed to write dictionary header to .cmp file
std::vector<std::pair<std::string, std::string>> Dictionary::getEntries() const {
    std::vector<std::pair<std::string, std::string>> entries;
    for (const auto& entry : wordToToken) {
        entries.push_back({entry.first, entry.second});
    }
    return entries;
}

// WHAT THIS DOES: Clear all dictionary entries
// WHY AND INTENT: Resets dictionary for fresh compression operation
void Dictionary::clear() {
    wordToToken.clear();
    tokenToWord.clear();
}

// WHAT THIS DOES: Return size of dictionary
// WHY AND INTENT: Useful for statistics and validation
int Dictionary::size() const {
    return static_cast<int>(wordToToken.size());
}

// ============================================================================
// IMPLEMENTATION: Compressor
// ============================================================================

// WHAT THIS DOES: Analyze tokens and build a profitable dictionary
// WHY AND INTENT: First phase separates analysis from compression so we
//                  know which words to replace before writing output
void Compressor::analyzeAndBuildDictionary(
    const std::vector<Tokenizer::TextToken>& tokens
) {
    // Phase 1a: Frequency analysis - count word occurrences
    // WHY: Identifies which words repeat enough to consider for compression
    auto frequencyMap = FrequencyAnalyzer::analyze(tokens);
    
    // Phase 1b: Candidate generation - filter to likely candidates
    // WHY: Words with frequency < 2 cannot produce positive savings
    auto candidates = CandidateGenerator::generateCandidates(frequencyMap);
    
    // Phase 1c: Profitability analysis and dictionary building
    // WHY: Each candidate is evaluated; only profitable words are added
    for (const auto& word : candidates) {
        int frequency = frequencyMap[word];
        std::string token = tokenAllocator.nextToken();
        
        // Calculate if this word is worth compressing
        auto savings = SavingsCalculator::calculateSavings(word, frequency, token);
        
        // Only add to dictionary if it saves space
        // WHY: No point compressing if it doesn't reduce file size
        if (savings.isProfitable) {
            dictionary.addEntry(word, token);
            
            // Debug output for verification (optional)
            // std::cout << "Added: " << word << " -> " << token 
            //           << " (saving " << savings.netSaving << " bytes)" << std::endl;
        }
    }
}

// WHAT THIS DOES: Main compression pipeline
// WHY AND INTENT: Orchestrates reading input, analyzing, building dictionary,
//                  replacing words, and writing compressed .cmp file
void Compressor::compress(const std::string& inputFile, const std::string& outputFile) {
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
    
    // Step 5: Stream through tokens and replace dictionary words with tokens
    // WHY: Word-by-word processing avoids accidental token collisions
    //      (e.g., "software" containing "#0" if we did naive string replacement)
    for (const auto& token : tokens) {
        if (token.isWord && dictionary.hasWord(token.value)) {
            // This word is in dictionary - replace with token
            // WHY: Saves space for profitable words
            compressedOutput << dictionary.getToken(token.value);
        } else {
            // Keep everything else unchanged (spaces, punctuation, non-dictionary words)
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
    std::cout << "Dictionary entries: " << dictionary.size() << std::endl;
    if (text.length() > 0) {
        double ratio = (100.0 * (text.length() - output.length())) / text.length();
        std::cout << "Compression ratio: " << ratio << "%" << std::endl;
    }
}

// ============================================================================
// IMPLEMENTATION: DictionaryParser
// ============================================================================

// WHAT THIS DOES: Parse dictionary from start of .cmp file
// WHY AND INTENT: Decompression requires knowing token → word mappings.
//                  Parser reads header until ===END_DICT=== marker
Dictionary DictionaryParser::parseDictionary(std::istringstream& stream) {
    Dictionary dict;
    std::string line;
    
    // Read lines until we hit the dictionary terminator
    while (std::getline(stream, line)) {
        // Stop when we reach the end marker
        if (line == "===END_DICT===") {
            break;
        }
        
        // Parse line in format: token:word
        // WHY: This format is compact and easy to parse
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string token = line.substr(0, colonPos);
            std::string word = line.substr(colonPos + 1);
            
            // Add to dictionary (inverse direction compared to compression)
            // WHY: During decompression, we need token → word lookup
            dict.addEntry(word, token);
        } else if (!line.empty()) {
            // Malformed entry - neither empty nor properly formatted
            throw std::runtime_error("Malformed dictionary entry: " + line);
        }
    }
    
    return dict;
}

// ============================================================================
// IMPLEMENTATION: Decompressor
// ============================================================================

// WHAT THIS DOES: Main decompression pipeline
// WHY AND INTENT: Reverses compression by: (1) parsing dictionary header,
//                  (2) scanning body for tokens, (3) expanding them,
//                  (4) keeping non-tokens unchanged
void Decompressor::decompress(const std::string& inputFile, const std::string& outputFile) {
    // Validate input file exists
    if (!FileReader::exists(inputFile)) {
        throw std::runtime_error("Input file not found: " + inputFile);
    }
    
    // Step 1: Read entire compressed file
    std::string compressedText = FileReader::read(inputFile);
    std::istringstream input(compressedText);
    
    // Step 2: Parse dictionary from header
    // WHY: Must know token mappings before processing body
    Dictionary dictionary = DictionaryParser::parseDictionary(input);
    
    // Step 3: Read remaining content (the compressed body)
    // WHY: Must preserve exact content after dictionary marker
    // We read everything that remains after the dictionary header
    std::string body = input.str().substr(input.tellg());
    
    // Handle case where tellg() returns -1 (if we're at end)
    // WHY: Some implementations return -1, meaning we need to check
    if (body.empty() && input.eof() == false) {
        // Try reading directly by finding the delimiter in original text
        // WHY: Direct string search is more reliable than stream position
        size_t delimiterPos = compressedText.find("===END_DICT===\n");
        if (delimiterPos != std::string::npos) {
            body = compressedText.substr(delimiterPos + strlen("===END_DICT===\n"));
        }
    }
    
    // Step 4: Process body - expand tokens to words
    // WHY: Token-aware expansion prevents false matches (e.g., "#0" inside text)
    std::ostringstream decompressedOutput;
    
    size_t pos = 0;
    while (pos < body.length()) {
        // Look for token start character '#'
        size_t tokenStart = body.find('#', pos);
        
        if (tokenStart == std::string::npos) {
            // No more tokens - write rest of body as-is
            // WHY: Anything after last token is non-token text
            decompressedOutput << body.substr(pos);
            break;
        }
        
        // Write everything before the token
        decompressedOutput << body.substr(pos, tokenStart - pos);
        
        // Extract potential token (format: #N or #NN or #NNN, etc.)
        // WHY: Tokens are always # followed by digits
        size_t tokenEnd = tokenStart + 1;
        while (tokenEnd < body.length() && std::isdigit(body[tokenEnd])) {
            tokenEnd++;
        }
        
        std::string potentialToken = body.substr(tokenStart, tokenEnd - tokenStart);
        
        // Check if this is actually a dictionary token
        if (dictionary.hasToken(potentialToken)) {
            // Expand token back to original word
            // WHY: This is the core decompression operation
            decompressedOutput << dictionary.getWord(potentialToken);
        } else {
            // Not a dictionary token - keep the literal text
            // WHY: Handles cases like literal "#0" in original text that wasn't
            //      compressed (if it was never in dictionary)
            decompressedOutput << potentialToken;
        }
        
        pos = tokenEnd;
    }
    
    // Step 5: Write decompressed file
    std::string output = decompressedOutput.str();
    FileWriter::write(outputFile, output);
    
    // Print statistics
    std::cout << "Decompression complete!" << std::endl;
    std::cout << "Compressed size: " << compressedText.length() << " bytes" << std::endl;
    std::cout << "Decompressed size: " << output.length() << " bytes" << std::endl;
    std::cout << "Dictionary entries used: " << dictionary.size() << std::endl;
}

// ============================================================================
// IMPLEMENTATION: CompressionStatistics
// ============================================================================

// WHAT THIS DOES: Display compression metrics to user
// WHY AND INTENT: Gives immediate feedback on compression effectiveness
void CompressionStatistics::print() const {
    std::cout << "\n=== Compression Statistics ===" << std::endl;
    std::cout << "Original size: " << originalSize << " bytes" << std::endl;
    std::cout << "Compressed size: " << compressedSize << " bytes" << std::endl;
    std::cout << "Dictionary entries: " << dictionaryEntries << std::endl;
    
    if (originalSize > 0) {
        long saved = originalSize - compressedSize;
        double ratio = (100.0 * saved) / originalSize;
        std::cout << "Bytes saved: " << saved << std::endl;
        std::cout << "Compression ratio: " << ratio << "%" << std::endl;
        
        if (saved > 0) {
            std::cout << "Status: COMPRESSION SUCCESSFUL" << std::endl;
        } else {
            std::cout << "Status: NO COMPRESSION BENEFIT" << std::endl;
        }
    }
}
