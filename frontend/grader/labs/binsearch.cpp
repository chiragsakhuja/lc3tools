/*
 * Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
 */
#include "framework2.h"

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

bool compareOutput(std::vector<char> const & source, std::vector<char> const & target, bool full_match)
{
    if(full_match) { return source == target; }

    if(source.size() > target.size()) { return false; }

    uint64_t match_count = 0;
    for(uint64_t i = 0; i < target.size() || match_count != source.size(); ++i) {
        for(uint64_t j = 0; j < source.size(); ++j) {
            if(source[j] == target[i]) {
                ++match_count;
            } else {
                break;
            }
        }
    }

    return match_count == source.size();
}

void OneTest(lc3::sim & sim, Grader & grader, double total_points)
{
    Node ali{0x4009, 0x5000, nullptr, nullptr, "Ali", 18000};
    Node dan{0x400d, 0x5004, nullptr, nullptr, "Dan", 16000};
    Node daniel{0x4005, 0x5008, nullptr, &ali, "Daniel", 24000};
    Node joe{0x4001, 0x500f, &daniel, &dan, "Joe", 20000};
    fillMem(sim, &joe);
    sim.writeMem(0x4000, joe.node_addr);

    // Setup
    sim.runUntilInputPoll();
    std::string expected = "Type a professor's name and then press enter:";
    bool correct = compareOutput(grader.getOutputter().getBuffer(), std::vector<char>(expected.begin(), expected.end()), true);

    grader.getInputter().setString("Dan\n");
    sim.runUntilInputPoll();
    expected = "16000";
    correct &= compareOutput(grader.getOutputter().getBuffer(), std::vector<char>(expected.begin(), expected.end()), false);

    grader.getInputter().setString("Dani\n");
    sim.runUntilInputPoll();
    expected = "No Entry";
    correct &= compareOutput(grader.getOutputter().getBuffer(), std::vector<char>(expected.begin(), expected.end()), false);

    grader.getInputter().setString("Daniel\n");
    sim.runUntilInputPoll();
    expected = "24000";
    correct &= compareOutput(grader.getOutputter().getBuffer(), std::vector<char>(expected.begin(), expected.end()), false);

    grader.getInputter().setString("dan\n");
    sim.runUntilInputPoll();
    expected = "No Entry";
    correct &= compareOutput(grader.getOutputter().getBuffer(), std::vector<char>(expected.begin(), expected.end()), false);

    grader.getInputter().setString("d\n");
    bool success = sim.runUntilHalt();
    grader.verify("OneTest", success && ! sim.didExceedInstLimit() && correct, total_points);
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
