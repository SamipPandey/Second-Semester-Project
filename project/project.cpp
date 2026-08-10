#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

struct WordStats {
    string word;
    size_t count;
};

class CompressorEngine {
private:
    const string HEADER_DELIMITER = "===END_DICT===";

    unordered_map<string, string> buildDictionary(const string& text) {
        unordered_map<string, size_t> freqs;
        stringstream ss(text);
        string word;

        while (ss >> word) {
            freqs[word]++;
        }

        vector<WordStats> candidates;
        for (const auto& [w, count] : freqs) {
            if (count > 1 && w.length() > 3) {
                candidates.push_back({w, count});
            }
        }

        sort(candidates.begin(), candidates.end(), [](const WordStats& a, const WordStats& b) {
            return (a.word.length() * a.count) > (b.word.length() * b.count);
        });

        unordered_map<string, string> wordToToken;
        int tokenID = 0;

        for (const auto& item : candidates) {
            string token = "#" + to_string(tokenID);
            size_t originalSize = item.word.length() * item.count;
            size_t compressedSize = (token.length() * item.count) + item.word.length() + token.length() + 2;

            if (originalSize > compressedSize) {
                wordToToken[item.word] = token;
                tokenID++;
            }
        }

        return wordToToken;
    }

public:
    bool compress(const string& inputFile, const string& outputFile) {
        ifstream inFile(inputFile);
        if (!inFile) {
            cerr << "Error: Could not open input file: " << inputFile << "\n";
            return false;
        }

        stringstream buffer;
        buffer << inFile.rdbuf();
        string content = buffer.str();
        inFile.close();

        auto wordToToken = buildDictionary(content);

        ofstream outFile(outputFile, ios::binary);
        if (!outFile) {
            cerr << "Error: Could not create output file: " << outputFile << "\n";
            return false;
        }

        // Output header dictionary
        for (const auto& [word, token] : wordToToken) {
            outFile << token << ":" << word << "\n";
        }
        outFile << HEADER_DELIMITER << "\n";

        // Output compressed content body
        stringstream ss(content);
        string word;
        bool first = true;

        while (ss >> word) {
            if (!first) outFile << " ";
            first = false;

            if (wordToToken.count(word)) {
                outFile << wordToToken[word];
            } else {
                outFile << word;
            }
        }

        outFile.close();
        cout << "Successfully compressed: '" << inputFile << "' -> '" << outputFile << "'\n";
        return true;
    }

    bool decompress(const string& inputFile, const string& outputFile) {
        ifstream inFile(inputFile, ios::binary);
        if (!inFile) {
            cerr << "Error: Could not open compressed file: " << inputFile << "\n";
            return false;
        }

        unordered_map<string, string> tokenToWord;
        string line;

        // Parse header dictionary
        while (getline(inFile, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line == HEADER_DELIMITER) {
                break;
            }

            size_t pos = line.find(':');
            if (pos != string::npos) {
                string token = line.substr(0, pos);
                string word = line.substr(pos + 1);
                tokenToWord[token] = word;
            }
        }

        ofstream outFile(outputFile);
        if (!outFile) {
            cerr << "Error: Could not create target file: " << outputFile << "\n";
            return false;
        }

        // Reconstruct original file content
        string token;
        bool first = true;

        while (inFile >> token) {
            if (!first) outFile << " ";
            first = false;

            if (tokenToWord.count(token)) {
                outFile << tokenToWord[token];
            } else {
                outFile << token;
            }
        }

        inFile.close();
        outFile.close();
        cout << "Successfully decompressed: '" << inputFile << "' -> '" << outputFile << "'\n";
        return true;
    }
};

int main() {
    CompressorEngine engine;
    int choice = 0;

    cout << "====================================\n";
    cout << "    File Compressor & Decompressor  \n";
    cout << "====================================\n";
    cout << "1. Compress a text file\n";
    cout << "2. Decompress a file\n";
    cout << "3. Exit\n";
    cout << "Enter your choice (1-3): ";
    cin >> choice;

    string inputFile, outputFile;

    switch (choice) {
        case 1:
            cout << "Enter source input filename (e.g., input.txt): ";
            cin >> inputFile;
            cout << "Enter target output filename (e.g., compressed.cmp): ";
            cin >> outputFile;
            engine.compress(inputFile, outputFile);
            break;

        case 2:
            cout << "Enter compressed filename (e.g., compressed.cmp): ";
            cin >> inputFile;
            cout << "Enter target output filename (e.g., restored.txt): ";
            cin >> outputFile;
            engine.decompress(inputFile, outputFile);
            break;

        case 3:
            cout << "Exiting program. Goodbye!\n";
            break;

        default:
            cout << "Invalid choice! Please select 1, 2, or 3.\n";
            break;
    }

    return 0;
}