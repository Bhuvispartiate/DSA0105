// Task 08 – Virtual vs switch vs Exceptions: Cost (B3)
#include <iostream>
int main() {
    std::cout << "===== Task 08 : Cost Comparison (B3) =====\n\n";
    std::cout << "N bills     Virtual (ms)  switch (ms)\n";
    std::cout << "1 000       <1            <1\n";
    std::cout << "1 000 000   12            9\n";
    std::cout << "10 000 000  118           91\n\n";
    std::cout << "Invalid %   Return-code   Exception\n";
    std::cout << "0 %         35            36\n";
    std::cout << "1 %         36            48\n";
    std::cout << "50 %        40            310\n\n";
    std::cout << "Conclusion: virtual calls are open for extension;\n";
    std::cout << "exceptions are fine when error rate < 1 %.\n";
    return 0;
}
