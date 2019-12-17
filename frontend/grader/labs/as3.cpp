#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>

#include "../framework2.h"

static constexpr uint64_t InstLimit = 500000;
static constexpr uint16_t StartAddr = 0x32F0;

struct Node
{
    uint16_t sim_addr;
    uint16_t sim_next, sim_str;

    std::string name;
    Node * next, * prev;

    Node(std::string const & name, uint16_t sim_addr, uint16_t sim_str) :
        sim_addr(sim_addr), sim_next(0), sim_str(sim_str), name(name), next(nullptr), prev(nullptr)
    {}

    Node(Node const * node) :
        sim_addr(node->sim_addr), sim_next(0), sim_str(node->sim_str), name(node->name), next(nullptr),
        prev(nullptr)
    {}

    Node * append(Node * next)
    {
        this->next = next;
        if(next != nullptr) {
            this->sim_next = next->sim_addr;
            next->prev = this;
        } else {
            this->sim_next = 0x0000;
        }
        return next;
    }
};

using solution_t = std::tuple<Node *, Node *, Node *>;
using mem_map_t = std::set<uint16_t>;

std::ostream & operator<<(std::ostream & out, Node const * root) {
    while(root != nullptr) {
        out << root->name << " [";
        out << (root->prev ? root->prev->name : "null");
        out << ", ";
        out << (root->next ? root->next->name : "null");
        out << "] (";
        out << std::hex;
        out << "x" << std::setfill('0') << std::setw(4) << root->sim_addr << " ";
        out << "x" << std::setfill('0') << std::setw(4) << root->sim_next << " ";
        out << "x" << std::setfill('0') << std::setw(4) << root->sim_str;
        out << std::dec;
        out << ")";
        if(root->next) {
            out << " <- ";
        }
        root = root->next;
    }
    return out;
}

void deleteList(Node * list)
{
    Node * list_cur = list;
    while(list_cur != nullptr) {
        Node * temp = list_cur->next;
        delete list_cur;
        list_cur = temp;
    }
}

void deleteLists(solution_t solution)
{
    deleteList(std::get<0>(solution));
    deleteList(std::get<1>(solution));
    deleteList(std::get<2>(solution));
}

void setupMem(lc3::sim & sim, uint16_t sim_start_addr, Node * node)
{
    if(node == nullptr) {
        sim.setMem(sim_start_addr, 0x0000);
        return;
    }

    sim.setMem(sim_start_addr, node->sim_addr);

    while(node != nullptr) {
        sim.setMem(node->sim_addr, node->sim_next);
        sim.setMem(node->sim_addr + 1, node->sim_str);
        sim.setMemString(node->sim_str, node->name);
        node = node->next;
    }
}

solution_t intersect(Node * list1, Node * list2)
{
    Node * list1_head = new Node("ROOT", 0x4000, 0x0000);
    Node * list2_head = new Node("ROOT", 0x4001, 0x0000);
    Node * list3_head = new Node("ROOT", 0x4002, 0x0000);

    list1_head->append(list1);
    list2_head->append(list2);

    Node * list1_cur = list1;
    Node * list2_cur = list2;
    Node * list3_cur = list3_head;

    while(list1_cur != nullptr) {
        bool match = false;

        while(list2_cur != nullptr) {
            if(list1_cur->name == list2_cur->name) {
                list1_cur->prev->append(list1_cur->next);
                list2_cur->prev->append(list2_cur->next);
                list3_cur = list3_cur->append(new Node(list1_cur));

                Node * temp = list1_cur->next;
                delete list1_cur;
                list1_cur = temp;

                delete list2_cur;

                match = true;
                break;
            }

            list2_cur = list2_cur->next;
        }

        if(! match) {
            list1_cur = list1_cur->next;
        }
        list2_cur = list2_head->next;
    }

    return std::make_tuple(list1_head, list2_head, list3_head);
}

enum class ResultStatus {
      FULL_MATCH = 0
    , INVALID_END = 1
    , NO_MATCH = 2
};

std::string createString(lc3::sim & sim, uint16_t addr)
{
    std::stringstream stream;
    uint16_t offset = 0;
    while(sim.getMem(addr + offset) != 0 && offset < 100) {
        stream << static_cast<char>(sim.getMem(addr + offset));
        ++offset;
    }
    return stream.str();
}

ResultStatus verifySingleList(lc3::sim & sim, Node * list, Grader & grader)
{
    Node * list_cur = list;

    while(list_cur != nullptr) {
        std::stringstream stream;
        uint16_t actual_next = sim.getMem(list_cur->sim_addr);
        uint16_t expected_next = list_cur->sim_next;

        if(list_cur != list) {
            std::string actual_str = createString(sim, sim.getMem(list_cur->sim_addr + 1));
            std::string const & expected_str = list_cur->name;
            stream << "(expected " << expected_str << ") (actual " << actual_str << ")";

            if(actual_str != expected_str) {
                grader.output(stream.str());
                return ResultStatus::NO_MATCH;
            }
        }

        grader.output(stream.str());
        list_cur = list_cur->next;

        if(expected_next == 0x0000 && actual_next != 0x0000) {
            return ResultStatus::INVALID_END;
        }

/*
 *        std::stringstream stream;
 *        stream << std::hex;
 *
 *        uint16_t actual_next = sim.getMem(list_cur->sim_addr);
 *        uint16_t expected_next = list_cur->sim_next;
 *
 *        stream << "x" << std::setfill('0') << std::setw(4) << list_cur->sim_addr << ": ";
 *        stream << "(expected x" << std::setfill('0') << std::setw(4) << expected_next << ") ";
 *        stream << "(actual x" << std::setfill('0') << std::setw(4) << actual_next << ") ";
 *
 *        if(actual_next == expected_next) {
 *            if(list_cur != list) {
 *                std::string actual_str = createString(sim, sim.getMem(list_cur->sim_addr + 1));
 *                std::string const & expected_str = list_cur->name;
 *                stream << "(expected " << expected_str << ") (actual " << actual_str << ")";
 *
 *                if(actual_str != expected_str) {
 *                    grader.output(stream.str());
 *                    return ResultStatus::NO_MATCH;
 *                }
 *            }
 *        } else {
 *            if(expected_next == 0x0000) {
 *                grader.output(stream.str());
 *                return ResultStatus::INVALID_END;
 *            } else {
 *                grader.output(stream.str());
 *                return ResultStatus::NO_MATCH;
 *            }
 *        }
 *
 *        grader.output(stream.str());
 *        list_cur = list_cur->next;
 */
    }

    return ResultStatus::FULL_MATCH;
}

void verify(bool success, lc3::sim & sim, solution_t solution,
    double points, Grader & grader)
{
    if(! success) { grader.error("Execution hit exception"); }
    if(sim.didExceedInstLimit()) { grader.error("Exceeded instruction limit"); return; }

    auto result = verifySingleList(sim, std::get<0>(solution), grader);
    grader.verify("List 1 correct", result == ResultStatus::FULL_MATCH, points / 3);
    if(result != ResultStatus::FULL_MATCH) {
        grader.verify("List 1 mostly correct", result == ResultStatus::INVALID_END, points / 12);
    }

    result = verifySingleList(sim, std::get<1>(solution), grader);
    grader.verify("List 2 correct", result == ResultStatus::FULL_MATCH, points / 3);
    if(result != ResultStatus::FULL_MATCH) {
        grader.verify("List 2 mostly correct", result == ResultStatus::INVALID_END, points / 12);
    }

    result = verifySingleList(sim, std::get<2>(solution), grader);
    grader.verify("List 3 correct", result == ResultStatus::FULL_MATCH, points / 3);
    if(result != ResultStatus::FULL_MATCH) {
        grader.verify("List 3 mostly correct", result == ResultStatus::INVALID_END, points / 12);
    }
}

void Example(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Chen", 0x4040, 0x4025);
    Node * tail = list1->append(new Node("Grey", 0x4140, 0x4077));
    tail = tail->append(new Node("Patel", 0x4550, 0x5525));

    Node * list2 = new Node("Brown", 0x4090, 0x5650);
    tail = list2->append(new Node("Chen", 0x5000, 0x5600));
    tail = tail->append(new Node("Patel", 0x454E, 0x5500));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void buildMemMapSingleList(mem_map_t & mem_map, Node * list)
{
    Node * list_cur = list;
    while(list_cur != nullptr) {
        mem_map.insert(list_cur->sim_addr);
        mem_map.insert(list_cur->sim_addr + 1);

        for(uint16_t i = 0; i < list_cur->name.size(); i += 1) {
            mem_map.insert(list_cur->sim_str + i);
        }

        list_cur = list_cur->next;
    }
}

void Simple(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Arjun", 0x8000, 0x8010);
    Node * tail = list1->append(new Node("Chirag", 0x8100, 0x8110));
    tail = tail->append(new Node("Joseph", 0x8200, 0x8210));
    tail = tail->append(new Node("Meiling", 0x8300, 0x8310));

    Node * list2 = new Node("Grace", 0x8400, 0x8410);
    tail = list2->append(new Node("Joseph", 0x8500, 0x8510));
    tail = tail->append(new Node("Sabee", 0x8600, 0x8610));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    mem_map_t mem_map;
    buildMemMapSingleList(mem_map, list1);
    buildMemMapSingleList(mem_map, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);

    bool allocated_mem = false;
    for(uint16_t i = 0x4003; i < 0xFE00; i += 1) {
        if(sim.getMem(i) != 0x0000) {
            if(mem_map.find(i) == mem_map.end()) {
                allocated_mem = true;
                break;
            }
        }
    }

    /*
     *if(allocated_mem) {
     *    grader.verify("Did not duplicate nodes", false, total_points);
     *} else {
     */
        verify(success, sim, solution, total_points, grader);
    /*
     *}
     */
    deleteLists(solution);
}

template<int EmptyListN>
void EmptyList(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list = new Node("Chirag", 0x8000, 0x8010);
    Node * tail = list->append(new Node("Evil", 0x8100, 0x8110));

    solution_t solution;
    if(EmptyListN == 0) {
        setupMem(sim, 0x4000, nullptr);
        setupMem(sim, 0x4001, list);
        solution = intersect(nullptr, list);
    } else {
        setupMem(sim, 0x4000, list);
        setupMem(sim, 0x4001, nullptr);
        solution = intersect(list, nullptr);
    }

    bool success = sim.runUntilHalt();

    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void EmptyIntersection(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Arjun", 0x8000, 0x8010);
    Node * tail = list1->append(new Node("Chirag", 0x8100, 0x8110));
    tail = tail->append(new Node("Grace", 0x8200, 0x8210));

    Node * list2 = new Node("Meiling", 0x8300, 0x8310);
    tail = list2->append(new Node("Joseph", 0x8400, 0x8410));
    tail = tail->append(new Node("Sabee", 0x8500, 0x8510));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void EmptyAll(lc3::sim & sim, Grader & grader, double total_points)
{
    setupMem(sim, 0x4000, nullptr);
    setupMem(sim, 0x4001, nullptr);

    bool success = sim.runUntilHalt();

    auto solution = intersect(nullptr, nullptr);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void MoveAll(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Chirag", 0x8000, 0x8010);
    Node * list2 = new Node("Chirag", 0x8300, 0x8310);

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void SimilarStrings(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Arjun", 0x8000, 0x8010);
    Node * tail = list1->append(new Node("Chirag", 0x8100, 0x8110));
    tail = tail->append(new Node("Joseph", 0x8200, 0x8210));
    tail = tail->append(new Node("Meiling", 0x8300, 0x8310));

    Node * list2 = new Node("Grace", 0x8400, 0x8410);
    tail = list2->append(new Node("Joseph", 0x8500, 0x8510));
    tail = tail->append(new Node("MeilingTang", 0x8600, 0x8610));
    tail = tail->append(new Node("Sabee", 0x8700, 0x8710));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void SameStringAddr(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Arjun", 0x8000, 0x8010);
    Node * tail = list1->append(new Node("Chirag", 0x8100, 0x8110));
    tail = tail->append(new Node("Joseph", 0x8200, 0x8210));
    tail = tail->append(new Node("Meiling", 0x8300, 0x8310));

    Node * list2 = new Node("Grace", 0x8400, 0x8410);
    tail = list2->append(new Node("Joseph", 0x8500, 0x8210));
    tail = tail->append(new Node("Sabee", 0x8600, 0x8610));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
}

void EmptyString(lc3::sim & sim, Grader & grader, double total_points)
{
    Node * list1 = new Node("Arjun", 0x8000, 0x8010);
    Node * tail = list1->append(new Node("Chirag", 0x8100, 0x8110));
    tail = tail->append(new Node("", 0x8200, 0x8210));
    tail = tail->append(new Node("Meiling", 0x8300, 0x8310));

    Node * list2 = new Node("Grace", 0x8400, 0x8410);
    tail = list2->append(new Node("", 0x8500, 0x8510));
    tail = tail->append(new Node("Sabee", 0x8600, 0x8610));

    setupMem(sim, 0x4000, list1);
    setupMem(sim, 0x4001, list2);

    bool success = sim.runUntilHalt();

    auto solution = intersect(list1, list2);
    verify(success, sim, solution, total_points, grader);
    deleteLists(solution);
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

void setup(Grader & grader)
{
    grader.registerTest("Example", Example, 10, false);
    grader.registerTest("Example", Example, 10, true);
    grader.registerTest("Simple", Simple, 10, false);
    grader.registerTest("Empty List1", EmptyList<0>, 5, true);
    grader.registerTest("Empty List2", EmptyList<1>, 5, true);
    grader.registerTest("Empty Intersection", EmptyIntersection, 10, true);
    grader.registerTest("Empty All", EmptyAll, 10, true);
    grader.registerTest("Move All", MoveAll, 10, false);
    grader.registerTest("Similar Strings", SimilarStrings, 10, false);
    grader.registerTest("Same String Address", SameStringAddr, 10, false);
    grader.registerTest("Empty String", EmptyString, 10, false);
}

void shutdown(void) {}
