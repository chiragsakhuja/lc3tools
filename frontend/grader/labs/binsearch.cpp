#include "../framework.h"

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
    sim.setMem(root->node_addr + 2, root->str_addr);
    sim.setMemString(root->str_addr, root->name);
    sim.setMem(root->node_addr + 3, root->income);
    if(root->left != nullptr) {
        sim.setMem(root->node_addr, root->left->node_addr);
        fillMem(sim, root->left);
    } else {
        sim.setMem(root->node_addr, 0);
    }
    if(root->right != nullptr) {
        sim.setMem(root->node_addr + 1, root->right->node_addr);
        fillMem(sim, root->right);
    } else {
        sim.setMem(root->node_addr + 1, 0);
    }
}

void OneTest(lc3::sim & sim, StringInputter & inputter)
{
    Node ali{0x4009, 0x5000, nullptr, nullptr, "Ali", 18000};
    Node dan{0x400d, 0x5004, nullptr, nullptr, "Dan", 16000};
    Node daniel{0x4005, 0x5008, nullptr, &ali, "Daniel", 24000};
    Node joe{0x4001, 0x500f, &daniel, &dan, "Joe", 20000};
    fillMem(sim, &joe);
    sim.setMem(0x4000, joe.node_addr);

    // Setup
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_NAMED("Correct prompt", "Type a professor's name and then press enter:");

    inputter.setString("Dan\n");
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_HAD_NAMED("Dan", "16000");

    inputter.setString("Dani\n");
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_HAD_NAMED("Dani", "No Entry");

    inputter.setString("Daniel\n");
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_HAD_NAMED("Daniel", "24000");

    inputter.setString("dan\n");
    sim.runUntilInputPoll();
    VERIFY_OUTPUT_HAD_NAMED("dan", "No Entry");

    inputter.setString("d\n");
    bool success = sim.runUntilHalt();
    VERIFY(success && ! sim.didExceedInstLimit());
}

void testBringup(lc3::sim & sim)
{
    sim.setPC(0x3000);
    sim.setRunInstLimit(InstLimit);
}

void testTeardown(lc3::sim & sim)
{
    (void) sim;
}

void setup(void)
{
    REGISTER_TEST(One, OneTest, 60);
    REGISTER_RANDOM_TEST(One, OneTest, 40);
}

void shutdown(void) {}
