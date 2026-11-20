// Task 06 – The Rules of Operator Overloading
// OOP with C++ | Assignment 3 – CO3

#include <iostream>
#include <cstring>

class Text {
    char* data;
    std::size_t len;
public:
    Text(const char* s = "") {
        len = std::strlen(s);
        data = new char[len + 1];
        std::strcpy(data, s);
    }

    Text(const Text& o) : data(new char[o.len + 1]), len(o.len) {
        std::strcpy(data, o.data);
    }

    Text& operator=(const Text& o) {
        if (this == &o) return *this;
        char* neu = new char[o.len + 1];
        std::strcpy(neu, o.data);
        delete[] data;
        data = neu;
        len = o.len;
        return *this;
    }

    ~Text() { delete[] data; }

    Text operator+(const Text& o) const {
        Text result;
        delete[] result.data;
        result.len = len + o.len;
        result.data = new char[result.len + 1];
        std::strcpy(result.data, data);
        std::strcat(result.data, o.data);
        return result;
    }

    bool operator==(const Text& o) const {
        return std::strcmp(data, o.data) == 0;
    }

    bool operator!=(const Text& o) const { return !(*this == o); }

    friend std::ostream& operator<<(std::ostream& os, const Text& t) {
        return os << t.data;
    }
};

int main() {
    std::cout << "===== Task 06 : Text Class (Rule of Three) =====\n\n";

    Text a("Hello");
    Text b(" World");
    Text c = a + b;

    std::cout << "a + b = " << c << "\n";
    std::cout << "a == b ? " << std::boolalpha << (a == b) << "\n";
    std::cout << "c == \"Hello World\" ? " << (c == Text("Hello World")) << "\n";

    return 0;
}
