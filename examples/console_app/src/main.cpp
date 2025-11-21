#include <iostream>
#include <memory>
#include <vector>
#include "../../Unit.h"

int main() {
    using UnitPtr = std::shared_ptr<Unit>;

    // Create three units. Constructor is Unit(int x, int y, double initialValue)
    UnitPtr u1 = std::make_shared<Unit>(0, 0, 0.0);
    UnitPtr u2 = std::make_shared<Unit>(1, 0, 0.0);
    UnitPtr u3 = std::make_shared<Unit>(0, 1, 0.0);

    // Wire up connections (u1 pushes to u2 and u3)
    u1->connections.push_back(u2);
    u1->connections.push_back(u3);

    // Basic simulation loop: set a source value, then update/push and print
    for (int step = 0; step < 10; ++step) {
        std::cout << "=== Step " << step << " ===\n";
        if (step == 0) {
            u1->set_value(5.0); // initial stimulus
        }

        // Call update on each unit (order as needed by your model)
        u1->update();
        u2->update();
        u3->update();

        // Call push to propagate deltas to connections
        u1->push();
        u2->push();
        u3->push();

        // Print each unit using existing operator<<
        std::cout << "* u1: " << *u1 << '\n';
        std::cout << "* u2: " << *u2 << '\n';
        std::cout << "* u3: " << *u3 << '\n';
        std::cout << '\n';
    }

    return 0;
}