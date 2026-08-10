# Byte-Token File Compressor & Decompressor

A lightweight C++ text compression utility that implements single-byte dictionary substitution. The program scans plain text files, identifies high-frequency words, and dynamically substitutes them with single-byte binary tokens (extended ASCII range `0x80`–`0xFF`) to achieve measurable file size reduction.

## Key Features

- **Single-Byte Tokenization**: Uses extended ASCII values (`128`–`255`) as substitution tokens. Each compressed dictionary word occupies exactly 1 byte in the compressed file body.
- **Profitability Calculation**: Analyzes total byte savings before replacing any word, accounting for dictionary header overhead and spacing to guarantee space efficiency.
- **Header Dictionary**: Embeds a compact key-value map at the top of the compressed file for fast, standalone decompression.
- **Header Boundary Protection**: Uses a custom delimiter (`===END_DICT===`) to separate compressed metadata from the body.

## How It Works

1. **Analysis & Frequency Count**: Reads the input document and tallies frequency counts for every space-delimited word.
2. **Profitability Check**: Ranks words by potential byte savings using the formula:

   $$\text{Original Size} > \text{Body Token Cost} + \text{Header Overhead}$$

3. **Dictionary Mapping**: Selects up to 128 of the most profitable candidate words and assigns each a unique 1-byte binary token.
4. **Encoding**: Writes the dictionary header and body text replaced with 1-byte binary tokens to an output file.
5. **Decoding**: Reads the header map, reconstructs the dictionary, and streams original text words back into the destination file.

## Build Instructions

Compile using any C++17-compliant standard compiler (GCC, Clang, MSVC):

```bash
g++ -std=c++17 main.cpp -o compressor
```

## Usage

Run the compiled executable and follow the interactive menu prompts:

```bash
./compressor
```

### Menu Options

```
====================================
     File Compressor & Decompressor  
====================================
1. Compress a text file
2. Decompress a file
3. Exit
```

## Example

### 1. Compress a File

```
Enter your choice (1-3): 1
Enter source input filename (e.g., input.txt): document.txt
Enter target output filename (e.g., compressed.cmp): document.cmp
Successfully compressed: 'document.txt' -> 'document.cmp'
```

### 2. Decompress a File

```
Enter your choice (1-3): 2
Enter compressed filename (e.g., compressed.cmp): document.cmp
Enter target output filename (e.g., restored.txt): document_restored.txt
Successfully decompressed: 'document.cmp' -> 'document_restored.txt'
```

## Requirements

- **C++ Compiler**: Supporting C++17 or higher
- **File Support**: Plain text (`.txt`, `.log`, `.md`, `.cpp`, etc.) with emoji use also
