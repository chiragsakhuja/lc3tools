/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework.h"

static constexpr uint64_t InstLimit = 5000;

struct Node
{
    uint16_t node_addr, str_addr;
    Node * left, * right;
    std::string name;
    uint16_t income;

    Node(uint16_t node_addr, uint16_t str_addr, Node * left, Node * right, std::string const & name, uint16_t income) :
        node_addr(node_addr), str_addr(str_addr), left(left), right(right), name(name), income(income)
    {}
};

void fillMem(lc3::sim & sim, Node * root)
{
    sim.writeMem(root->node_addr + 2, root->str_addr);
    sim.writeStringMem(root->str_addr, root->name);
    sim.writeMem(root->node_addr + 3, root->income);
    if(root->left != nullptr) {
        sim.writeMem(root->node_addr, root->left->node_addr);
        fillMem(sim, root->left);
    } else {
        sim.writeMem(root->node_addr, 0);
    }
    if(root->right != nullptr) {
        sim.writeMem(root->node_addr + 1, root->right->node_addr);
        fillMem(sim, root->right);
    } else {
        sim.writeMem(root->node_addr + 1, 0);
    }
}

std::ostream & operator<<(std::ostream & out, std::vector<char> const & data)
{
    for(char c : data) {
        out << c;
    }
    return out;
}

void OneTest(lc3::sim & sim, Grader & grader, double total_points)
{
    Node ali{0x4009, 0x5000, nullptr, nullptr, "Ali", 18000};
    Node dan{0x400d, 0x5004, nullptr, nullptr, "Dan", 16000};
    Node daniel{0x4005, 0x5008, nullptr, &ali, "Daniel", 24000};
    Node joe{0x4001, 0x500f, &daniel, &dan, "Joe", 20000};
    fillMem(sim, &joe);
    sim.writeMem(0x4000, joe.node_addr);

    bool success = true;
    success &= sim.runUntilInputConsumed();
    bool correct = grader.checkMatch(grader.getOutput(), "Type a professor's name and then press enter:");
    grader.verify("Correct", success && correct, total_points / 5);

    grader.clearOutput();
    grader.setInputString("Dan\n");
    success &= sim.runUntilInputConsumed();
    correct = grader.checkContain(grader.getOutput(), "16000");
    grader.verify("Dan", success && correct, total_points / 5);

    grader.clearOutput();
    grader.setInputString("Dani\n");
    success &= sim.runUntilInputConsumed();
    correct = grader.checkContain(grader.getOutput(), "No Entry");
    grader.verify("Dani", success && correct, total_points / 5);

    grader.clearOutput();
    grader.setInputString("Daniel\n");
    success &= sim.runUntilInputConsumed();
    correct = grader.checkContain(grader.getOutput(), "24000");
    grader.verify("Daniel", success && correct, total_points / 5);

    grader.clearOutput();
    grader.setInputString("dan\n");
    success &= sim.runUntilInputConsumed();
    correct = grader.checkContain(grader.getOutput(), "No Entry");
    grader.verify("dan", success && correct, total_points / 5);

    grader.clearOutput();
    grader.setInputString("d\n");
    success &= sim.runUntilHalt();
    grader.verify("Exit", success && ! sim.didExceedInstLimit(), 0);

}

void testBringup(lc3::sim & sim)
{
    sim.writePC(0x3000);
    sim.setRunInstLimit(InstLimit);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(Grader & grader)
{
    grader.registerTest("One", OneTest, 60, false);
    grader.registerTest("One", OneTest, 40, true);
}

void shutdown(void) {}
