
// #include <utecdf/column/column.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <utec/disk/btree.h>
#include <utec/disk/pagemanager.h>

#include <fmt/core.h>

// PAGE_SIZE 64 bytes
#define PAGE_SIZE  64 

// Other examples:
// PAGE_SIZE 1024 bytes => 1Kb
// PAGE_SIZE 1024*1024 bytes => 1Mb

// PAGE_SIZE = 2 * sizeof(long) +  (BTREE_ORDER + 1) * sizeof(int) + (BTREE_ORDER + 2) * sizeof(long)  
// PAGE_SIZE = 2 * sizeof(long) +  (BTREE_ORDER) * sizeof(int) + sizeof(int) + (BTREE_ORDER) * sizeof(long) + 2 * sizeof(long)
// PAGE_SIZE = (BTREE_ORDER) * (sizeof(int) + sizeof(long))  + 2 * sizeof(long) + sizeof(int) +  2 * sizeof(long)
//  BTREE_ORDER = PAGE_SIZE - (2 * sizeof(long) + sizeof(int) +  2 * sizeof(long)) /  (sizeof(int) + sizeof(long))

#define BTREE_ORDER   ((PAGE_SIZE - (2 * sizeof(long) + sizeof(int) +  2 * sizeof(long)) ) /  (sizeof(int) + sizeof(long)))


struct DiskBasedBtree : public ::testing::Test
{
};
using namespace utec::disk;

TEST_F(DiskBasedBtree, IndexingRandomElements) {
  bool trunc_file = true;
  std::shared_ptr<pagemanager> pm = std::make_shared<pagemanager>("btree.index", trunc_file);
  std::cout << "PAGE_SIZE: " << PAGE_SIZE << std::endl;
  std::cout << "BTREE_ORDER: " << BTREE_ORDER << std::endl;
  btree<char, BTREE_ORDER> bt(pm);
  std::string values = "zxcnmvfjdaqpirue";
  for(auto c : values) {
    bt.insert(c);
  }
  bt.print_tree();
  std::ostringstream out;
  bt.print(out);
  std::sort(values.begin(), values.end());
  EXPECT_EQ(out.str(), values.c_str());
}
 
TEST_F(DiskBasedBtree, Persistence) {
  std::shared_ptr<pagemanager> pm = std::make_shared<pagemanager>("btree.index");
  btree<char, BTREE_ORDER> bt(pm);
  std::string values = "123456";
  for(auto c : values) {
    bt.insert(c);
  }
  bt.print_tree();

  std::ostringstream out;
  bt.print(out);
  std::string all_values = "zxcnmvfjdaqpirue123456";
  std::sort(all_values.begin(), all_values.end());
  EXPECT_EQ(out.str(), all_values.c_str());
}


TEST_F(DiskBasedBtree, Iterators) {
  std::shared_ptr<pagemanager> pm = std::make_shared<pagemanager>("btree.index");
  using char_btree = btree<char, BTREE_ORDER>;
  char_btree bt(pm);
// TODO:
//  char_btree::iterator iter =  bt.find('a');
//  for( ; iter != bt.end(); iter++) {
//    std::cout << *iter << ", ";
//  }
}


