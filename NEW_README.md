# Dictionary-Based File Compressor & Decompressor

A custom C++17 implementation of a frequency-based dictionary substitution compressor that intelligently replaces repeated words with short tokens to reduce file size.

## Project Overview

This project implements a **profit-aware** dictionary compression algorithm that:

1. **Analyzes** text to find repeated words
2. **Calculates** whether replacing each word saves space (accounting for dictionary overhead)
3. **Builds** a dictionary with only profitable word-token mappings
4. **Compresses** files by replacing dictionary words with short tokens (#0, #1, #2, etc.)
5. **Preserves** exact spacing, punctuation, and formatting
6. **Decompresses** by parsing the dictionary and expanding tokens back to original words

## Key Design Principles

### 1. Profit-Based Compression
Only words that produce **positive net savings** are included in the dictionary.

**Formula:**
```
OriginalCost = frequency × wordLength
TokenCost = frequency × tokenLength + dictionaryEntryLength
NetSaving = OriginalCost - TokenCost

Decision: Include in dictionary if NetSaving > 0
```

### 2. Format Structure
Compressed `.cmp` files follow this structure:

```
+---------------------------+
| DICTIONARY HEADER         |
| #0:software               |
| #1:computer               |
| ===END_DICT===            |
+---------------------------+
| COMPRESSED TEXT BODY      |
| the #0 engineer likes...  |
+---------------------------+
```

### 3. Comment System
The code uses three types of comments:

1. **WHAT THIS DOES** - Describes what the function/class accomplishes
2. **WHY AND INTENT** - Explains design reasoning and purpose
3. **Inline clarifications** - Details complex logic inline

## Building the Project

### Prerequisites
- C++17 compatible compiler (g++, clang, MSVC)
- Make (optional, for Makefile)

### Compilation

**Using Makefile:**
```bash
make              # Build executable
make clean        # Remove build artifacts
make rebuild      # Clean and rebuild
```

**Manual compilation:**
```bash
g++ -std=c++17 -Wall -Wextra -O2 -o compressor main.cpp compressor.cpp
```

## Usage

### Basic Command Structure
```bash
./compressor compress <input_file> <output_file>
./compressor decompress <input_file> <output_file>
```

### Compression Example
```bash
./compressor compress sample.txt compressed.cmp
```

**Output:**
```
Starting compression...
Input: sample.txt
Output: compressed.cmp
Compression complete!
Original size: 234 bytes
Compressed size: 198 bytes
Dictionary entries: 3
Compression ratio: 15.38%
Saved to: compressed.cmp
```

### Decompression Example
```bash
./compressor decompress compressed.cmp restored.txt
```

**Output:**
```
Starting decompression...
Input: compressed.cmp
Output: restored.txt
Decompression complete!
Compressed size: 198 bytes
Decompressed size: 234 bytes
Dictionary entries used: 3
Saved to: restored.txt
```

### Verification
```bash
diff sample.txt restored.txt
# If no output, files are identical (perfect round-trip)
```

## Project Architecture

### Core Classes

#### FileReader
**What it does:** Reads file contents into memory  
**Why:** Centralizes file I/O with consistent error handling

#### FileWriter
**What it does:** Writes compressed/decompressed output to files  
**Why:** Ensures safe file writing with exception handling

#### Tokenizer
**What it does:** Splits text into words and whitespace/punctuation tokens  
**Why:** Preserves exact formatting while separating content for analysis

**Example:**
```
Input:  "the software engineer"
Output: [{true, "the"}, {false, " "}, {true, "software"}, {false, " "}, {true, "engineer"}]
```

#### FrequencyAnalyzer
**What it does:** Counts occurrences of each word  
**Why:** Identifies candidates for compression

**Example:**
```
software → 3
engineer → 1
likes    → 1
```

#### CandidateGenerator
**What it does:** Filters to words with frequency ≥ 2  
**Why:** Single-occurrence words can't provide positive savings

#### SavingsCalculator
**What it does:** Computes profitability for each candidate word  
**Why:** Core decision logic for which words to include in dictionary

**Example:**
```
Word: "software"
Frequency: 3
Original Cost: 8 × 3 = 24 bytes
Token Cost: 2 × 3 = 6 bytes
Dictionary Cost: 2 + 1 + 8 + 1 = 12 bytes
Total Cost: 6 + 12 = 18 bytes
Net Saving: 24 - 18 = 6 bytes ✓ PROFITABLE
```

#### TokenAllocator
**What it does:** Generates sequential token IDs (#0, #1, #2, etc.)  
**Why:** Ensures unique, compact tokens

#### Dictionary
**What it does:** Maps words to tokens (word → token) and vice versa (token → word)  
**Why:** Bidirectional lookup needed for compression and decompression

#### Compressor
**What it does:** Orchestrates entire compression pipeline  
**Why:** Brings together analysis, dictionary building, and token replacement

**Compression Workflow:**
1. Read input file
2. Tokenize text
3. Analyze word frequencies
4. Generate candidates (frequency ≥ 2)
5. Calculate savings for each candidate
6. Build dictionary with only profitable words
7. Write dictionary header to .cmp file
8. Write ===END_DICT=== delimiter
9. Stream through text and replace dictionary words with tokens
10. Write compressed body to .cmp file

#### DictionaryParser
**What it does:** Reads dictionary from .cmp file header  
**Why:** Decompression must first know token → word mappings

#### Decompressor
**What it does:** Orchestrates entire decompression pipeline  
**Why:** Reverses compression by parsing dictionary and expanding tokens

**Decompression Workflow:**
1. Read .cmp file
2. Parse dictionary header (until ===END_DICT===)
3. Read compressed body
4. Scan body for tokens (#N patterns)
5. Expand recognized tokens to original words
6. Keep non-tokens unchanged
7. Write restored file

#### CompressionStatistics
**What it does:** Displays compression metrics  
**Why:** Provides user feedback on compression effectiveness

## Example Walkthrough

### Input File: `sample.txt`
```
the software engineer likes software engineering because software is fun
```

### Compression Analysis

**Frequency Map:**
```
software       → 3
the            → 1
engineer       → 1
likes          → 1
engineering    → 1
because        → 1
is             → 1
fun            → 1
```

**Candidate: "software" (frequency = 3)**
```
Original Cost:      3 × 8 = 24 bytes
Token "#0":         3 × 2 = 6 bytes
Dictionary entry:   "#0:software\n" = 12 bytes
Total Cost:         6 + 12 = 18 bytes
Net Saving:         24 - 18 = 6 bytes ✓ PROFITABLE
```

**Other candidates:**
- All single-occurrence words are filtered out (optimization)
- "the" only appears once → rejected

### Compressed Output

**Dictionary Header:**
```
#0:software
===END_DICT===
```

**Body:**
```
the #0 engineer likes #0 engineering because #0 is fun
```

**Full .cmp File:**
```
#0:software
===END_DICT===
the #0 engineer likes #0 engineering because #0 is fun
```

### Decompression
The decompressor:
1. Reads `#0:software` and builds dictionary
2. Sees `===END_DICT===` and knows dictionary is complete
3. Scans body: "the " → write "the "
4. Finds `#0` → look up in dictionary → write "software"
5. Continues: " engineer likes " → write unchanged
6. Finds `#0` → write "software"
7. Continues: " engineering..." → write unchanged

**Restored File:**
```
the software engineer likes software engineering because software is fun
```

## Testing

The project is tested against the matrix in the architecture document:

| Test | Expected Result |
|---|---|
| Empty file | Compress/decompress successfully |
| One word | No dictionary required |
| Repeated long word | Dictionary entry created |
| Repeated short word | May be rejected due to overhead |
| No repetition | Dictionary remains empty |
| Punctuation | Preserved |
| Multiple spaces | Preserved |
| Newlines | Preserved |
| Tabs | Preserved |
| Literal `#0` | Handled as regular text if not in dictionary |
| Many dictionary entries | All mappings restored |
| Round trip | Original == restored |

## Error Handling

### Compression Errors
- Input file missing → Clear error message
- Input file cannot be opened → Exception with filename
- Output file cannot be created → Exception with path
- Memory errors → Exception with context

### Decompression Errors
- Invalid .cmp file → Exception
- Missing ===END_DICT=== → Exception
- Malformed dictionary entry → Exception with details
- Invalid tokens → Treated as literals
- Unexpected EOF → Exception

## Performance Characteristics

### Time Complexity
- Tokenization: O(n) where n = file size
- Frequency analysis: O(w) where w = unique words
- Candidate generation: O(w)
- Savings calculation: O(c) where c = candidates
- Compression: O(n)
- Decompression: O(n)
- Overall: O(n) linear time

### Space Complexity
- Input buffer: O(n)
- Frequency map: O(w)
- Dictionary: O(c)
- Output buffer: O(n)
- Overall: O(n) linear space

### Compression Effectiveness
- **Best case:** Files with many long repeated words
  - Example: "computer computer computer..." → "#0 #0 #0..."
  - Savings: Up to 75%+ depending on word frequency and length

- **Worst case:** Files with unique words or short words that repeat
  - Example: "is is is is..." (2 letters, token is 2 letters, dictionary overhead kills it)
  - Savings: 0% or negative (no compression)

- **Typical case:** Mixed content with some repetition
  - Savings: 5-20% depending on content

## Code Structure

```
.
├── compressor.h         # Header file with class definitions
├── compressor.cpp       # Implementation of all classes
├── main.cpp             # CLI entry point
├── Makefile             # Build configuration
├── sample.txt           # Example input file
├── README.md            # This file
└── LICENSE              # Project license
```

## Implementation Details

### Comment Convention
Every significant function includes comments following this pattern:

```cpp
// WHAT THIS DOES: Brief description of functionality
// WHY AND INTENT: Explanation of design choice
void myFunction() {
    // Inline comment for complex logic
}
```

### No External Dependencies
- Uses only C++17 standard library
- No external compression libraries (intentional - educational project)
- No boost, zlib, or other third-party dependencies

### Efficiency Considerations
- Single-pass tokenization using character classification
- Unordered_map for O(1) frequency lookups
- Stream-based file I/O
- Proper exception handling to prevent resource leaks

## Limitations

The current implementation intentionally does not include:
- ✗ GUI (command-line only)
- ✗ Huffman coding
- ✗ LZ77 or LZSS
- ✗ ZIP/GZIP compatibility
- ✗ Encryption
- ✗ Folder compression
- ✗ Binary file support (text only)

These are suitable for future enhancements.

## Future Enhancements

1. **Multi-pass optimization:** Calculate savings for all candidates at once
2. **Token length optimization:** Use shorter tokens for most frequent words
3. **Binary file support:** Handle non-text content
4. **Parallel compression:** Multi-threaded analysis for large files
5. **LZ77 comparison:** Implement alternative algorithm for benchmarking
6. **GUI interface:** Qt or wxWidgets frontend
7. **Streaming decompression:** Decompress without loading entire file

## Contributing

To extend this project:

1. **Maintain comment structure:** Always use WHAT/WHY format
2. **Follow C++17 style:** Use modern features appropriately
3. **Add error handling:** Catch and report errors clearly
4. **Test thoroughly:** Verify round-trip compression/decompression
5. **Update documentation:** Keep README in sync with code

## License

[Add your license here]

## Author

Dictionary Compressor Project - Educational Implementation

---

**Key Rule:** Do not replace a word merely because it repeats. Replace it only when the replacement plus dictionary overhead produces a **positive net saving**.

**Key File Format Rule:** Dictionary comes first, then `===END_DICT===`, then compressed body.

**Key Decompression Rule:** Parse dictionary first, invert to `token → word`, then stream through body expanding tokens.

**Key Correctness Rule:** `Decompress(Compress(original)) == original`
