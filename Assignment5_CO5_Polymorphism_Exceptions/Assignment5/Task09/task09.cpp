// Task 09 – Final Engineering Decision (B4)
#include <iostream>
int main() {
    std::cout << "===== Task 09 : Final Engineering Decision =====\n\n";
    std::cout << "Production choice for 200 000 bills/day, error rate < 1 %:\n\n";
    std::cout << "  • Polymorphic Bill hierarchy (open for new types)\n";
    std::cout << "  • Abstract Payer hierarchy (open for new payers)\n";
    std::cout << "  • Exceptions for rare error paths\n";
    std::cout << "  • RAII + virtual destructor → leak-free under all paths\n\n";
    std::cout << "Ability to extend without touching working code outweighs\n";
    std::cout << "the micro-benchmark advantage of a closed switch.\n";
    return 0;
}
