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

void OneTest(lc3::sim & sim, Tester & tester, double total_points)
{
    Node ali{0x4009, 0x5000, nullptr, nullptr, "Ali", 18000};
    Node dan{0x400d, 0x5004, nullptr, nullptr, "Dan", 16000};
    Node daniel{0x4005, 0x5008, nullptr, &ali, "Daniel", 24000};
    Node joe{0x4001, 0x500f, &daniel, &dan, "Joe", 20000};
    fillMem(sim, &joe);
    sim.writeMem(0x4000, joe.node_addr);

    bool success = true;
    success &= sim.runUntilInputRequested();
    bool correct = tester.checkMatch(tester.getOutput(), "Type a professor's name and then press enter:");
    tester.verify("Correct", success && correct, total_points / 5);

    tester.clearOutput();
    tester.setInputString("Dan\n");
    success &= sim.runUntilInputRequested();
    correct = tester.checkContain(tester.getOutput(), "16000");
    tester.verify("Dan", success && correct, total_points / 5);

    tester.clearOutput();
    tester.setInputString("Dani\n");
    success &= sim.runUntilInputRequested();
    correct = tester.checkContain(tester.getOutput(), "No Entry");
    tester.verify("Dani", success && correct, total_points / 5);

    tester.clearOutput();
    tester.setInputString("Daniel\n");
    success &= sim.runUntilInputRequested();
    correct = tester.checkContain(tester.getOutput(), "24000");
    tester.verify("Daniel", success && correct, total_points / 5);

    tester.clearOutput();
    tester.setInputString("dan\n");
    success &= sim.runUntilInputRequested();
    correct = tester.checkContain(tester.getOutput(), "No Entry");
    tester.verify("dan", success && correct, total_points / 5);

    tester.clearOutput();
    tester.setInputString("d\n");
    success &= sim.runUntilHalt();
    tester.verify("Exit", success && ! sim.didExceedInstLimit(), 0);
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

void setup(Tester & tester)
{
    tester.registerTest("One", OneTest, 60, false);
    tester.registerTest("One", OneTest, 40, true);
}

void shutdown(void) {}
