#include "common.h"
int toPackedLayoutCoord(int idx, int rows, int cols) {
    int i = idx / cols;
    int j = idx % cols;
    if (i % 2 == 0) {
        return i * cols + 2 * j;
    }
    else {
        return (i - 1) * cols + 2 * j + 1;
    }
}
// Row et cols correspondent au nombre de rows et cols de la matrice d'origine pas celle en packec layout ( qui est donc cols*2 rows/2)
int fromPackedLayoutCoord(int idx, int rows, int cols) {
    int i = idx / (cols * 2);
    int j = idx % (cols * 2);
    if (j % 2 == 0) {
        return (i * 2) * cols + j / 2;
    }
    else {
        return (i * 2 + 1) * cols + (j - 1) / 2;
    }
}

bool isequalstring(const std::string& str1, const std::string& str2) {
    if (str1.length() != str2.length()) { return false; }
    for (int i = 0; i < str1.length(); i++) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) { return false; }
    }
    return true;
}
