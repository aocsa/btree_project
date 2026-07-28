
// #include <utecdf/column/column.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utec/memory/btree.h>
#include <fmt/core.h>

struct MemoryBasedBtree : public ::testing::Test
{
};

TEST_F(MemoryBasedBtree, TestA) {
    using namespace utec::memory;

    btree<int> bt;
    std::string values = "zxcnmvafjdaqpirue";
    for(auto c : values) {
       bt.insert((int)c);
       bt.print();
    }
    bt.print();
}

TEST_F(MemoryBasedBtree, FindExistingKeys) {
    using namespace utec::memory;
    btree<int> bt;
    for (int v : {10, 20, 30, 40, 50}) {
        bt.insert(v);
    }
    EXPECT_TRUE(bt.find(10));
    EXPECT_TRUE(bt.find(30));
    EXPECT_TRUE(bt.find(50));
}

TEST_F(MemoryBasedBtree, FindMissingKeys) {
    using namespace utec::memory;
    btree<int> bt;
    for (int v : {10, 20, 30}) {
        bt.insert(v);
    }
    EXPECT_FALSE(bt.find(0));
    EXPECT_FALSE(bt.find(15));
    EXPECT_FALSE(bt.find(99));
}
 
 