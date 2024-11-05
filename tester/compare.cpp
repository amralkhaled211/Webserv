#include <iostream>
#include <fstream>
#include <string>

void compareFiles(const std::string &file1, const std::string &file2) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (!f1.is_open() || !f2.is_open()) {
        std::cerr << "Error opening files." << std::endl;
        return;
    }

    std::string line1, line2;
    int lineNumber = 1;
    bool differencesFound = false;

    while (std::getline(f1, line1) && std::getline(f2, line2)) {
        if (line1 != line2) {
            std::cout << "Difference found at line " << lineNumber << ":\n";
            std::cout << "File 1: " << line1 << "\n";
            std::cout << "File 2: " << line2 << "\n";
            differencesFound = true;
        }
        ++lineNumber;
    }

    // Check if any file has extra lines
    while (std::getline(f1, line1)) {
        std::cout << "Extra line in " << file1 << " at line " << lineNumber << ": " << line1 << "\n";
        differencesFound = true;
        ++lineNumber;
    }

    while (std::getline(f2, line2)) {
        std::cout << "Extra line in " << file2 << " at line " << lineNumber << ": " << line2 << "\n";
        differencesFound = true;
        ++lineNumber;
    }

    if (!differencesFound) {
        std::cout << "Files are identical." << std::endl;
    }

    f1.close();
    f2.close();
}

int main() {
    std::string file1 = "ones.txt";
    std::string file2 = "test/ones.txt";

    compareFiles(file1, file2);

    return 0;
}