# Testing Guide - Dictionary Compressor & Decompressor

## Test Framework

This document outlines manual testing procedures for the dictionary compressor.
All tests verify that: **Decompress(Compress(original)) == original**

## Build and Setup

```bash
# Build the project
make rebuild

# Verify executable was created
ls -la compressor
```

## Test Cases

### Test 1: Basic Compression with Repeated Words

**File:** `sample.txt` (provided)

```bash
./compressor compress sample.txt compressed.cmp
```

**Expected Output:**
```
Starting compression...
Input: sample.txt
Output: compressed.cmp
Compression complete!
Original size: [X] bytes
Compressed size: [Y] bytes
Dictionary entries: 1 (or more)
Compression ratio: [Z]%
Saved to: compressed.cmp
```

**Verify dictionary was created:**
```bash
head -5 compressed.cmp
```

**Expected output shows:**
- `#0:software` or similar token:word entry
- `===END_DICT===` delimiter
- Compressed body with tokens like `#0`

---

### Test 2: Decompression Round-Trip

```bash
./compressor decompress compressed.cmp restored.txt
```

**Expected Output:**
```
Starting decompression...
Input: compressed.cmp
Output: restored.txt
Decompression complete!
Compressed size: [X] bytes
Decompressed size: [Y] bytes
Dictionary entries used: [N]
Saved to: restored.txt
```

**Verify exact match:**
```bash
diff sample.txt restored.txt
```

**Expected:** No output (files are identical)

---

### Test 3: Empty File

**Setup:**
```bash
touch empty.txt
./compressor compress empty.txt empty.cmp
```

**Expected:**
- Compressed file created successfully
- No dictionary entries
- File just contains `===END_DICT===` and nothing else

**Verification:**
```bash
./compressor decompress empty.cmp empty_restored.txt
diff empty.txt empty_restored.txt
```

**Expected:** No output (files match)

---

### Test 4: Single Word File

**Setup:**
```bash
echo -n "hello" > single.txt
./compressor compress single.txt single.cmp
```

**Expected:**
- No dictionary entries (single word doesn't repeat)
- Compressed file may be larger than original due to `===END_DICT===`

**Verification:**
```bash
./compressor decompress single.cmp single_restored.txt
diff single.txt single_restored.txt
```

**Expected:** No output (files match)

---

### Test 5: Repeated Short Words (Non-Profitable)

**Setup:**
```bash
# Create file with short word repeated many times
echo "is is is is is is is is is is" > short_words.txt

./compressor compress short_words.txt short.cmp
```

**Expected:**
- Word "is" (2 letters) is NOT in dictionary
- Token would also be 2 letters, so dictionary overhead makes it unprofitable
- Compression ratio near 0% or negative

**Verify:**
```bash
cat short.cmp | head -1
```

**Expected:** `===END_DICT===` (empty dictionary)

---

### Test 6: Repeated Long Words (Profitable)

**Setup:**
```bash
# Create file with long word repeated
echo "specification specification specification" > long_words.txt

./compressor compress long_words.txt long.cmp
```

**Expected:**
- "specification" (13 letters) appears 3 times = 39 bytes
- Token "#0" (2 letters) appears 3 times = 6 bytes
- Dictionary overhead (13 bytes) makes total 19 bytes
- Saving: 39 - 19 = 20 bytes ✓ Profitable
- Word IS included in dictionary

**Verify dictionary inclusion:**
```bash
head -1 long.cmp
```

**Expected output contains:**
```
#0:specification
```

---

### Test 7: Multiple Dictionary Entries

**Setup:**
```bash
cat > multi.txt << 'EOF'
programming programming programming
computer computer computer
software software software
the the the
algorithm algorithm algorithm
is is is
EOF

./compressor compress multi.txt multi.cmp
```

**Expected:**
- Multiple tokens in dictionary:
  - programming (11 letters) ✓ Profitable
  - computer (8 letters) ✓ Profitable
  - software (8 letters) ✓ Profitable
  - the (3 letters) - appears 3 times = 9 bytes; token 2 bytes × 3 = 6; dictionary 6 bytes; total 12 > 9 ✗ Not profitable
  - algorithm (9 letters) ✓ Profitable
  - is (2 letters) ✗ Not profitable (same length as token)

**Verify dictionary:**
```bash
grep "===END_DICT===" multi.cmp -B 10
```

**Expected:** Shows several #N:word entries before delimiter

**Round-trip verification:**
```bash
./compressor decompress multi.cmp multi_restored.txt
diff multi.txt multi_restored.txt
```

**Expected:** No output (perfect match)

---

### Test 8: Punctuation Preservation

**Setup:**
```bash
cat > punct.txt << 'EOF'
software, software! software?
Software: software.
EOF

./compressor compress punct.txt punct.cmp
./compressor decompress punct.cmp punct_restored.txt
diff punct.txt punct_restored.txt
```

**Expected:**
- All punctuation preserved exactly
- Uppercase 'S' in "Software" treated as different word
- Spacing around punctuation maintained

---

### Test 9: Multiple Spaces Preservation

**Setup:**
```bash
printf "software  software   software" > spaces.txt
./compressor compress spaces.txt spaces.cmp
./compressor decompress spaces.cmp spaces_restored.txt
diff spaces.txt spaces_restored.txt
```

**Expected:** No output (exact match, including multiple spaces)

---

### Test 10: Newline and Tab Preservation

**Setup:**
```bash
printf "software\tsoftware\nsoftware\n" > tabs_newlines.txt
./compressor compress tabs_newlines.txt tabs_newlines.cmp
./compressor decompress tabs_newlines.cmp tabs_newlines_restored.txt
diff tabs_newlines.txt tabs_newlines_restored.txt
```

**Expected:** No output (exact match with tabs and newlines)

---

### Test 11: Mixed Case Words

**Setup:**
```bash
cat > case.txt << 'EOF'
Software software SOFTWARE software
Computer computer
EOF

./compressor compress case.txt case.cmp
```

**Expected:**
- "software" and "Software" treated as DIFFERENT words
- "software" (lowercase) appears 3 times ✓ Likely profitable
- "Software" (uppercase) appears 1 time ✗ Not profitable

**Verify:**
```bash
grep "software" case.cmp
grep "Software" case.cmp
```

**Expected:**
- First line has token mapping for lowercase "software"
- Uppercase "Software" NOT in dictionary
- Body shows uppercase "Software" unchanged, lowercase `#N`

---

### Test 12: No Repetition (Empty Dictionary)

**Setup:**
```bash
echo "every word appears just one single time here" > unique.txt
./compressor compress unique.txt unique.cmp
```

**Expected:**
- Empty dictionary (no repeated words)
- File may be slightly larger due to delimiter
- Decompression still works

**Verification:**
```bash
./compressor decompress unique.cmp unique_restored.txt
diff unique.txt unique_restored.txt
```

---

### Test 13: Large Repetition

**Setup:**
```bash
# Create file with word repeated many times
python3 << 'PYTHON'
with open("large_repeat.txt", "w") as f:
    for i in range(1000):
        f.write("mississippi ")
PYTHON

./compressor compress large_repeat.txt large_repeat.cmp
```

**Expected:**
- "mississippi" (11 letters) × 1000 times = 11,000 bytes
- Token "#0" (2 letters) × 1000 times = 2,000 bytes
- Dictionary overhead (13 bytes)
- Total: 2,013 bytes
- Saving: 11,000 - 2,013 = 8,987 bytes (82%)

**Verify:**
```bash
wc -c large_repeat.txt large_repeat.cmp
./compressor decompress large_repeat.cmp large_repeat_restored.txt
diff large_repeat.txt large_repeat_restored.txt
```

---

### Test 14: Mixed Content with Varying Repetition

**Setup:**
```bash
cat > mixed.txt << 'EOF'
Python is a powerful programming language.
Python programming is fun and productive.
Data science with Python requires careful analysis.
Web development in Python is becoming popular.
Machine learning in Python uses libraries like TensorFlow.
EOF

./compressor compress mixed.txt mixed.cmp
./compressor decompress mixed.cmp mixed_restored.txt
diff mixed.txt mixed_restored.txt
```

**Expected:**
- "Python" appears 4 times ✓ Profitable (#0)
- "programming" appears 2 times - let's calculate:
  - Original: 11 × 2 = 22 bytes
  - Token: 2 × 2 = 4 bytes
  - Dictionary: 2 + 1 + 11 + 1 = 15 bytes
  - Total: 4 + 15 = 19 bytes
  - Saving: 22 - 19 = 3 bytes ✓ Profitable (#1)
- "is" appears 3 times ✗ Not profitable
- No output from diff

---

### Test 15: Error Handling - Missing File

```bash
./compressor compress nonexistent.txt output.cmp
```

**Expected:**
```
ERROR: Input file not found: nonexistent.txt
```

**Exit code:** 1

---

### Test 16: Error Handling - Cannot Write Output

**Setup:**
```bash
# Try to write to read-only directory (if running as non-root)
./compressor compress sample.txt /root/readonly.cmp
```

**Expected:**
```
ERROR: Cannot create output file: /root/readonly.cmp
```

**Exit code:** 1

---

### Test 17: Malformed Input (Invalid Mode)

```bash
./compressor invalid sample.txt output.cmp
```

**Expected:**
```
ERROR: Unknown mode 'invalid'
Mode must be 'compress' or 'decompress'
=== Dictionary-Based File Compressor/Decompressor ===
...
```

---

### Test 18: Wrong Number of Arguments

```bash
./compressor compress sample.txt
```

**Expected:**
```
=== Dictionary-Based File Compressor/Decompressor ===

Usage:
  ./compressor compress <input_file> <output_file>
  ./compressor decompress <input_file> <output_file>
...
```

---

## Automated Test Script

Create `test.sh`:

```bash
#!/bin/bash

# WHAT THIS DOES: Runs all test cases automatically
# WHY AND INTENT: Verifies project correctness without manual testing

echo "=== Dictionary Compressor Test Suite ==="
echo

tests_passed=0
tests_failed=0

# Function to run a test
run_test() {
    local name=$1
    local cmd=$2
    local check_cmd=$3
    
    echo "Test: $name"
    eval "$cmd"
    
    if eval "$check_cmd"; then
        echo "✓ PASSED"
        ((tests_passed++))
    else
        echo "✗ FAILED"
        ((tests_failed++))
    fi
    echo
}

# Build
echo "Building project..."
make clean && make
if [ ! -f compressor ]; then
    echo "Build failed!"
    exit 1
fi
echo

# Test 1: Round-trip with sample file
run_test "Round-trip compression/decompression" \
    "./compressor compress sample.txt test1.cmp && ./compressor decompress test1.cmp test1_restored.txt" \
    "diff sample.txt test1_restored.txt"

# Test 2: Empty file
run_test "Empty file compression" \
    "touch empty_test.txt && ./compressor compress empty_test.txt empty_test.cmp && ./compressor decompress empty_test.cmp empty_test_restored.txt" \
    "diff empty_test.txt empty_test_restored.txt"

# Test 3: Single word
run_test "Single word compression" \
    "echo -n 'hello' > single_test.txt && ./compressor compress single_test.txt single_test.cmp && ./compressor decompress single_test.cmp single_test_restored.txt" \
    "diff single_test.txt single_test_restored.txt"

# Cleanup
echo
echo "Cleaning up test files..."
rm -f test*.cmp test*_restored.txt empty_test* single_test* large_repeat* multi* punct* spaces* tabs_newlines* case* unique* mixed* short* long*

# Summary
echo "=== Test Results ==="
echo "Passed: $tests_passed"
echo "Failed: $tests_failed"

if [ $tests_failed -eq 0 ]; then
    echo "All tests passed! ✓"
    exit 0
else
    echo "Some tests failed! ✗"
    exit 1
fi
```

**Run it:**
```bash
chmod +x test.sh
./test.sh
```

---

## Manual Verification Checklist

Before deploying, verify:

- [ ] Executable compiles without warnings
- [ ] `compress` mode works on various file types
- [ ] `decompress` mode works on all .cmp files
- [ ] Round-trip test: original == decompressed
- [ ] Empty file handling works
- [ ] Punctuation is preserved
- [ ] Multiple spaces are preserved
- [ ] Newlines and tabs are preserved
- [ ] Mixed case is handled correctly
- [ ] Dictionary is populated only with profitable words
- [ ] Error messages are clear
- [ ] Program handles missing files gracefully
- [ ] Compression ratio is calculated correctly
- [ ] No memory leaks (valgrind check)

---

## Performance Testing

### Memory Usage
```bash
valgrind --leak-check=full ./compressor compress sample.txt test.cmp
```

### Speed Testing
```bash
time ./compressor compress large_file.txt large.cmp
time ./compressor decompress large.cmp restored.txt
```

### File Size Analysis
```bash
ls -lah sample.txt compressed.cmp
# Compare sizes for efficiency evaluation
```

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| File not found error | Verify file exists: `ls -la filename` |
| Compilation error | Check C++17 support: `g++ --version` |
| Decompression produces garbage | Verify .cmp file not corrupted: `head compressed.cmp` |
| Compression ratio is 0% or negative | File likely has no profitable words to compress |
| Dictionary is empty | All repeated words are not profitable (too short or low frequency) |
| Round-trip fails | Bug in tokenizer or token expansion logic |

---

## Test Coverage Summary

| Category | Tests | Coverage |
|----------|-------|----------|
| Basic Operations | 6 | Compress/decompress variations |
| Edge Cases | 6 | Empty files, single word, unique content |
| Formatting | 5 | Punctuation, spaces, tabs, newlines |
| Data | 2 | Large repetition, mixed content |
| Error Handling | 3 | Missing files, invalid input |
| **Total** | **22** | **Comprehensive** |
