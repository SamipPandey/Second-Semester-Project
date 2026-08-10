# File Compressor & Decompressor

A simple C++ command-line tool that compresses text files using **word-frequency dictionary substitution**. Frequently repeated words are replaced with short tokens (e.g. `#0`, `#1`) to reduce file size, and the process can be reversed to restore the original file.

## How It Works

1. **Dictionary Building** — The compressor scans the input file and counts word frequencies. Words longer than 3 characters that appear more than once are considered candidates for tokenization.
2. **Smart Selection** — Candidates are sorted by potential savings (word length × frequency), and each word is only tokenized if doing so actually reduces the total size (accounting for the space the token takes up in the dictionary header).
3. **Encoding** — The compressed file stores a header dictionary (`token:word` pairs) followed by a delimiter (`===END_DICT===`), then the body of the text with selected words replaced by their tokens.
4. **Decoding** — The decompressor reads the header to rebuild the token-to-word mapping, then reconstructs the original text by replacing tokens with their corresponding words.

## Requirements

- A C++ compiler supporting C++17 (e.g. `g++`, `clang++`)

## Build

```bash
g++ -std=c++17 -O2 -o compressor main.cpp
```

## Usage

Run the compiled program and follow the interactive prompts:

```bash
./compressor
```

You'll see a menu:
====================================
File Compressor & Decompressor
Compress a text file
Decompress a file
Exit
Enter your choice (1-3):

### Compress a file
Choose option `1`, then provide:
- Source input filename (e.g. `input.txt`)
- Target output filename (e.g. `compressed.cmp`)

### Decompress a file
Choose option `2`, then provide:
- Compressed filename (e.g. `compressed.cmp`)
- Target output filename (e.g. `restored.txt`)

## Example

```bash
./compressor
# Choose 1, input.txt -> compressed.cmp
./compressor
# Choose 2, compressed.cmp -> restored.txt
```

## Limitations

- Whitespace formatting (multiple spaces, tabs, newlines) is **not preserved** — the decompressed file will have words separated by single spaces.
- Compression works best on text with many repeated, longer words (e.g. technical documents, logs).
- Not intended as a general-purpose or production-grade compression algorithm — it's a lightweight demonstration of dictionary-based text substitution.
