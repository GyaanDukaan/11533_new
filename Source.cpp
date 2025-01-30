#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <mutex>
#include <cassert>
#include <thread>

using namespace std;

// Prime numbers for the hash function
const int PRIME1 = 101;
const int PRIME2 = 103;

// Rolling hash implementation for Rabin-Karp Algorithm
class RabinKarp {
private:
    int prime;
    int base;
    vector<int> prefixHashes;
    mutable mutex mtx;  // Mutex for thread safety

public:
    RabinKarp(int prime = PRIME1, int base = 256) : prime(prime), base(base) {}

    // Hashing function to compute hash of a string
    int computeHash(const string& s) {
  //      unique_lock<mutex> lock(mtx);
        int hashValue = 0;
        for (char c : s) {
            hashValue = (hashValue * base + c) % prime;
        }
        return hashValue;
    }

    // Rolling hash calculation for substring (using base and prime)
    vector<int> precomputeHashes(const string& text, int length) {
        unique_lock<mutex> lock(mtx);
        prefixHashes.resize(text.length() + 1, 0);
        int hashValue = 0;
        int basePower = 1;

        // Compute the hash for the first substring of given length
        for (int i = 0; i < length; ++i) {
            hashValue = (hashValue * base + text[i]) % prime;
            if (i < length - 1) {
                basePower = (basePower * base) % prime;
            }
        }
        prefixHashes[length] = hashValue;

        // Precompute hashes for remaining substrings
        for (int i = length; i < text.length(); ++i) {
            hashValue = (hashValue - text[i - length] * basePower % prime + prime) % prime;
            hashValue = (hashValue * base + text[i]) % prime;
            prefixHashes[i + 1] = hashValue;
        }

        return prefixHashes;
    }

    // Collision detection: Check if the hashes match and substrings are equal
    bool detectCollision(const string& text, int start, int end, int hashValue, int length) {
        unique_lock<mutex> lock(mtx);
        int computedHash = computeHash(text.substr(start, length));
        return computedHash == hashValue;
    }
};

// Function to detect plagiarism between two papers
/*
void detectPlagiarism(const string& paper1, const string& paper2, int substringLength) {
    RabinKarp rk(PRIME1);

    vector<int> paper1Hashes = rk.precomputeHashes(paper1, substringLength);
    vector<int> paper2Hashes = rk.precomputeHashes(paper2, substringLength);

    unordered_map<int, vector<int>> hashMap;

    // Store hashes of paper1
    for (int i = 0; i < paper1.size() - substringLength + 1; ++i) {
        hashMap[paper1Hashes[i + substringLength]].push_back(i);
    }

    // Compare with paper2
    for (int i = 0; i < paper2.size() - substringLength + 1; ++i) {
        int hashValue = paper2Hashes[i + substringLength];
        if (hashMap.find(hashValue) != hashMap.end()) {
            for (int startIdx : hashMap[hashValue]) {
                if (rk.detectCollision(paper2, i, i + substringLength, hashValue, substringLength)) {
                    cout << "Plagiarized content detected between Paper 1 and Paper 2 at position " << i << endl;
                }
            }
        }
    }
}*/


// Function to detect plagiarism between two papers
void detectPlagiarism(const string& paper1, const string& paper2, int substringLength) {
    RabinKarp rk(PRIME1);

    // Precompute hashes for both papers
    vector<int> paper1Hashes = rk.precomputeHashes(paper1, substringLength);
    vector<int> paper2Hashes = rk.precomputeHashes(paper2, substringLength);

    unordered_map<int, vector<int>> hashMap;

    // Store hashes of paper1 in the hash map
    for (int i = 0; i <= paper1.size() - substringLength; ++i) {
        // Store the starting index of the substring for each hash
        hashMap[paper1Hashes[i]] = { i };
    }

    // Compare paper2 with paper1
    for (int i = 0; i <= paper2.size() - substringLength; ++i) {
        int hashValue = paper2Hashes[i];  // Correct hash value for comparison
        if (hashMap.find(hashValue) != hashMap.end()) {
            for (int startIdx : hashMap[hashValue]) {
                // If hashes match, check if the substrings are indeed equal
                if (rk.detectCollision(paper2, i, i + substringLength, hashValue, substringLength)) {
                    cout << "Plagiarized content detected between Paper 1 and Paper 2 at position " << i << endl;
                }
            }
        }
    }
}


// Test cases
void testComputeHash() {
    RabinKarp rk;
    assert(rk.computeHash("abc") == 90);
    assert(rk.computeHash("abcd") == 11);
    cout << "testComputeHash passed" << endl;
}

void testPrecomputeHashes() {
    RabinKarp rk;
    string text = "abcdabcd";
    vector<int> hashes = rk.precomputeHashes(text, 4);
    assert(hashes[4] == 11); // Hash of "abcd"
    assert(hashes[5] == 54); // Hash of "bcda"
    cout << "testPrecomputeHashes passed : " << hashes[5] << endl;
}

void testDetectCollision() {
    RabinKarp rk;
    string text = "abcdabcd";
    assert(rk.detectCollision(text, 0, 4, 50, 4));
    assert(!rk.detectCollision(text, 1, 5, 23, 4));
    cout << "testDetectCollision passed" << endl;
}

void testDetectPlagiarism() {
    string paper1 = "This is a simple test. The quick brown fox jumps over the lazy dog.";
    string paper2 = "This is a simple test. The quick brown fox jumps over a lazy dog.";
    detectPlagiarism(paper1, paper2, 15);
    // Expected output: "Plagiarized content detected between Paper 1 and Paper 2 at position 28"
    cout << "testDetectPlagiarism passed" << endl;
}

void testScalability() {
    string largeText1(100000, 'a');
    string largeText2 = largeText1;
    largeText2[50000] = 'b';
    detectPlagiarism(largeText1, largeText2, 100);
    // Expected output: Should not detect plagiarism
    cout << "testScalability passed" << endl;
}

void testHashCollision() {
    RabinKarp rk;
    string text1 = "abcd";
    string text2 = "efgh";
    assert(rk.computeHash(text1) != rk.computeHash(text2)); // Different hashes expected
    cout << "testHashCollision passed" << endl;
}

void testThreadSafety() {
    RabinKarp rk;
    thread t1([&rk]() {
        rk.computeHash("abc");
        });
    thread t2([&rk]() {
        rk.computeHash("def");
        });
    t1.join();
    t2.join();
    cout << "testThreadSafety passed" << endl;
}

int main() {
    testComputeHash();
    testPrecomputeHashes();
    testDetectCollision();
    testDetectPlagiarism();
    testScalability();
    testHashCollision();
    testThreadSafety();
    return 0;
}