#include "compressor.h"
#include <iostream>
#include <cstring>

// ============================================================================
// WHAT THIS DOES: Display usage information to user
// WHY AND INTENT: Helps users understand command-line syntax quickly
// ============================================================================
void printUsage(const char* programName) {
    std::cout << "\n=== Dictionary-Based File Compressor/Decompressor ===" << std::endl;
    std::cout << "\nUsage:" << std::endl;
    std::cout << "  " << programName << " compress <input_file> <output_file>" << std::endl;
    std::cout << "  " << programName << " decompress <input_file> <output_file>" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << programName << " compress sample.txt compressed.cmp" << std::endl;
    std::cout << "  " << programName << " decompress compressed.cmp restored.txt" << std::endl;
    std::cout << "\nNotes:" << std::endl;
    std::cout << "  - Compression uses frequency-based dictionary substitution" << std::endl;
    std::cout << "  - Only profitable words are included in dictionary" << std::endl;
    std::cout << "  - Compressed files use .cmp extension" << std::endl;
    std::cout << std::endl;
}

// ============================================================================
// WHAT THIS DOES: Main entry point for compression/decompression program
// WHY AND INTENT: Parses command-line arguments and routes to appropriate
//                  operation (compress or decompress) with error handling
// ============================================================================
int main(int argc, char* argv[]) {
    try {
        // Validate command-line arguments
        // WHY: Clear error messages for incorrect usage improve user experience
        if (argc != 4) {
            printUsage(argv[0]);
            return 1;
        }
        
        // Extract arguments
        std::string mode = argv[1];      // "compress" or "decompress"
        std::string inputFile = argv[2]; // Input filename
        std::string outputFile = argv[3]; // Output filename
        
        // Route to appropriate operation
        if (mode == "compress") {
            // WHAT THIS DOES: Compress input file to .cmp format
            // WHY AND INTENT: Creates compressed version using dictionary
            //                  substitution with profitability analysis
            std::cout << "Starting compression..." << std::endl;
            std::cout << "Input: " << inputFile << std::endl;
            std::cout << "Output: " << outputFile << std::endl;
            
            Compressor compressor;
            compressor.compress(inputFile, outputFile);
            
            std::cout << "Saved to: " << outputFile << std::endl;
            return 0;
        } 
        else if (mode == "decompress") {
            // WHAT THIS DOES: Decompress .cmp file back to original text
            // WHY AND INTENT: Reverses compression by parsing dictionary
            //                  and expanding tokens
            std::cout << "Starting decompression..." << std::endl;
            std::cout << "Input: " << inputFile << std::endl;
            std::cout << "Output: " << outputFile << std::endl;
            
            Decompressor decompressor;
            decompressor.decompress(inputFile, outputFile);
            
            std::cout << "Saved to: " << outputFile << std::endl;
            return 0;
        }
        else {
            // Unknown mode - show help
            std::cerr << "ERROR: Unknown mode '" << mode << "'" << std::endl;
            std::cerr << "Mode must be 'compress' or 'decompress'" << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    } 
    catch (const std::exception& e) {
        // WHAT THIS DOES: Catch and display any errors that occur
        // WHY AND INTENT: Provides clear error messages instead of crashes
        std::cerr << "\nERROR: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        // WHAT THIS DOES: Catch unexpected exceptions
        // WHY AND INTENT: Ensures graceful exit even for unforeseen errors
        std::cerr << "\nERROR: An unexpected error occurred!" << std::endl;
        return 1;
    }
}
