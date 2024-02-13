#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <vector>


// use strtok to split input string with given splitter
// strtok_r is reentrant version of strtok
// consecutive splitter will be considered one, empty string between splitter won't be returned
void splitstr0(std::string && input, const char * splitter = " \t") {
    std::cout << __PRETTY_FUNCTION__ << " input is " << input << std::endl;
    char * saveptr = nullptr;
    char * s = &input[0];
    char * start = &input[0];
    std::vector<const char *> res;
    // for (s = &input[0]; ; s = nullptr) {
    for (; ; s = nullptr) {
        const char * token = strtok_r(s, splitter, &saveptr);
        if (token == nullptr) break;
        std::cout << "token loc: " << (token - start) << std::endl;
        res.push_back(token);
    }
    for (auto str : res) {
        std::cout << "'" << str << "'" << std::endl;
    }
}

// use strtok to split input string with given splitter
void splitstr1(std::string && input, const char * splitter = " \t") {
    std::cout << __PRETTY_FUNCTION__ << " input is " << input << std::endl;
    char * s = &input[0];
    const char * pch = nullptr;
    std::vector<const char *> res;
    pch = strtok(s, splitter);
    while (pch != nullptr) {
        res.push_back(pch);
        pch = strtok(nullptr, splitter);
    }
    for (auto str : res) {
        std::cout << "'" << str << "'" << std::endl;
    }
}

void splitstr2(const std::string & input) {
    std::stringstream ss(input);
    // int a, b;
    std::cout << "splitstr2: input is '" << input << "'" << std::endl;
    // std::string a, b, c, d;
    int a = 0, b = 0, c = 0, d = 0;
    ss >> a >> b >> c >> d;
    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "c = " << c << std::endl;
    std::cout << "d = " << d << std::endl;
}

void splitstr3(const std::string & input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::cout << "splitstr3: input is '" << input << "'" << std::endl;
    std::string v;
    int i=0;
    while (ss >> v) {
        // will split on newline or space
        // consecutive spaces are treated as single space, i.e. '' will not be loaded
        // refer to https://en.cppreference.com/w/cpp/locale/ctype_char for taking 
        // other chars as delimiter
        std::cout << i++ << ": '" << v << "'" << std::endl;
        tokens.emplace_back(v);
    }
}

// read file line by line
void readfile1(const char * filename) {
    std::cout << "readfile1: filename is " << filename << std::endl;
    std::ifstream in(filename);

    std::string line;
    while (std::getline(in, line)) {
        // newline, i.e. \n is stripped
        std::cout << line << std::endl;
    }
}

void readfile2(const char * filename, bool stripend=true) {
    std::cout << "readfile2: filename is " << filename << std::endl;
    FILE * fp = fopen(filename, "r");
    if (fp==nullptr) {
        std::cerr << "cannot open file " << filename << " for reading\n";
        return;
    }
    char * line = nullptr;
    size_t len = 0;
    int ret = 0;
    while ((ret = getline(&line, &len, fp)) != -1) {
        // newline, i.e. \n is not stripped
        // line is nullptr, and getline will malloc or remalloc to get enough
        // space to readline, so must free line at the end
        line[ret-1] = '\0'; // remove '\n'
        printf("ret %d %d: %p %s", ret, strlen(line), (void*)line, line);
        printf("\n");
        std::string str(line, ret);
        std::cout << "'" << line << "'" << std::endl;
    }
    fclose(fp);
    if (line) free(line);
}

int main(int argc, const char ** argv) {
    if (argc > 1) {
        readfile1(argv[1]);
        std::cout << "\n";
        readfile2(argv[1]);
    }
    char str0[80] = "This is - www.tutorialspoint.com - \t website";
    splitstr0(str0);
    char str1[80] = "This is - www.tutorialspoint.com - \t website";
    splitstr1(str1);
    splitstr2("100 10");                // a=100, b=10, c & d are zero
    splitstr3("100 10");                // 100, 10
    splitstr2("100 10\n");              // a=100, b=10, c & d are zero
    splitstr3("100 10\n");              // 100, 10
    splitstr2("100 10 abc\n");          // a=100, b=10, c & d are zero
    splitstr3("100 10 abc\n");          // 100, 10, abc
    splitstr2("100 10 125 abc\n");      // a=100, b=10, c=125, d is zero
    splitstr3("100 10 125 abc\n");      // 100, 10, 125, abc
    splitstr2("100 10 abc 125\n");      // a=100, b=10, c & d are zero
    splitstr3("100 10 abc 125\n");      // 100, 10, abc, 125
    splitstr2("100 10 abc 125\nxyz bmw;1  ts ");      // a=100, b=10, c & d are zero
    splitstr3("100 10 abc 125\nxyz bmw;1  ts ");      // 100, 10, abc, 125, xyz, bmw;1, ts
}
