#pragma once

#include "pagemanager.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>

namespace utec {
namespace disk {

template <class T, int BTREE_ORDER = 3> class btree {
public:
  struct node {
    long page_id{-1};
    long count{0};

    T data[BTREE_ORDER + 1]{};
    long children[BTREE_ORDER + 2]{};

    node(long page_id) : page_id{page_id} {}

    void insert_in_node(int pos, const T &value) {
      const auto key_position = data + pos;
      const auto key_end = data + count;
      std::copy_backward(key_position, key_end, key_end + 1);

      const auto child_position = children + pos;
      const auto child_end = children + count + 1;
      std::copy_backward(child_position, child_end, child_end + 1);

      data[pos] = value;
      ++count;
    }

    [[nodiscard]] bool is_overflow() { return count > BTREE_ORDER; }
  };

  struct Metadata {
    long root_id{1};
    long count{0};
  } header;

  enum state {
    BT_OVERFLOW,
    BT_UNDERFLOW,
    NORMAL,
  };

private:
  std::shared_ptr<pagemanager> pm;

public:
  btree(std::shared_ptr<pagemanager> page_manager)
      : pm{std::move(page_manager)} {
    if (pm->is_empty()) {
      node root{header.root_id};
      pm->save(root.page_id, root);

      ++header.count;

      pm->save(0, header);
    } else {
      static_cast<void>(pm->recover(0, header));
    }
  }

  node new_node() {
    ++header.count;
    node ret{header.count};
    pm->save(0, header);
    return ret;
  }

  node read_node(long page_id) {
    node n{-1};
    static_cast<void>(pm->recover(page_id, n));
    return n;
  }

  bool write_node(long page_id, node n) {
    pm->save(page_id, n);
    return true;
  }

  void insert(const T &value) {
    node root = read_node(header.root_id);
    const int insertion_state = insert(root, value);

    if (insertion_state == BT_OVERFLOW) {
      split_root();
    }
  }

  int insert(node &ptr, const T &value) {
    int pos = 0;
    while (pos < ptr.count && ptr.data[pos] < value) {
      ++pos;
    }

    if (ptr.children[pos] != 0) {
      const long page_id = ptr.children[pos];
      node child = read_node(page_id);
      const int insertion_state = insert(child, value);
      if (insertion_state == BT_OVERFLOW) {
        split(ptr, pos);
      }
    } else {
      ptr.insert_in_node(pos, value);
      static_cast<void>(write_node(ptr.page_id, ptr));
    }

    return ptr.is_overflow() ? BT_OVERFLOW : NORMAL;
  }

  void split(node &parent, int pos) {
    node ptr = read_node(parent.children[pos]);
    node left = new_node();
    node right = new_node();

    int iter = 0;
    int i = 0;
    for (; iter < BTREE_ORDER / 2; ++i, ++iter) {
      left.children[i] = ptr.children[iter];
      left.data[i] = ptr.data[iter];
      ++left.count;
    }
    left.children[i] = ptr.children[iter];

    parent.insert_in_node(pos, ptr.data[iter]);

    ++iter;

    for (i = 0; iter < BTREE_ORDER + 1; ++i, ++iter) {
      right.children[i] = ptr.children[iter];
      right.data[i] = ptr.data[iter];
      ++right.count;
    }
    right.children[i] = ptr.children[iter];

    parent.children[pos] = left.page_id;
    parent.children[pos + 1] = right.page_id;

    static_cast<void>(write_node(parent.page_id, parent));
    static_cast<void>(write_node(left.page_id, left));
    static_cast<void>(write_node(right.page_id, right));
  }

  void split_root() {
    node ptr = read_node(header.root_id);
    node left = new_node();
    node right = new_node();

    int iter = 0;
    int i = 0;
    for (; iter < BTREE_ORDER / 2; ++i, ++iter) {
      left.children[i] = ptr.children[iter];
      left.data[i] = ptr.data[iter];
      ++left.count;
    }
    left.children[i] = ptr.children[iter];

    const T promoted_value = ptr.data[iter];
    ++iter;

    for (i = 0; iter < BTREE_ORDER + 1; ++i, ++iter) {
      right.children[i] = ptr.children[iter];
      right.data[i] = ptr.data[iter];
      ++right.count;
    }
    right.children[i] = ptr.children[iter];

    ptr.children[0] = left.page_id;
    ptr.data[0] = promoted_value;
    ptr.children[1] = right.page_id;
    ptr.count = 1;

    static_cast<void>(write_node(ptr.page_id, ptr));
    static_cast<void>(write_node(left.page_id, left));
    static_cast<void>(write_node(right.page_id, right));
  }

  bool find(const T &value) {
    node root = read_node(header.root_id);
    return find(root, value);
  }

  bool find(node &ptr, const T &value) {
    int pos = 0;
    while (pos < ptr.count && ptr.data[pos] < value) {
      ++pos;
    }

    if (pos < ptr.count && ptr.data[pos] == value) {
      return true;
    }

    if (ptr.children[pos] != 0) {
      node child = read_node(ptr.children[pos]);
      return find(child, value);
    }
    return false;
  }

  void print(std::ostream &out) {
    node root = read_node(header.root_id);
    print(root, 0, out);
  }

  void print(node &ptr, int level, std::ostream &out) {
    int i = 0;
    for (; i < ptr.count; ++i) {
      if (ptr.children[i] != 0) {
        node child = read_node(ptr.children[i]);
        print(child, level + 1, out);
      }
      out << ptr.data[i];
    }

    if (ptr.children[i] != 0) {
      node child = read_node(ptr.children[i]);
      print(child, level + 1, out);
    }
  }

  void print_tree() {
    node root = read_node(header.root_id);
    print_tree(root, 0);
    std::cout << "________________________\n";
  }

  void print_tree(node &ptr, int level) {
    int i = static_cast<int>(ptr.count) - 1;
    for (; i >= 0; --i) {
      if (ptr.children[i + 1] != 0) {
        node child = read_node(ptr.children[i + 1]);
        print_tree(child, level + 1);
      }

      for (int k = 0; k < level; ++k) {
        std::cout << "    ";
      }
      std::cout << ptr.data[i] << '\n';
    }

    if (ptr.children[i + 1] != 0) {
      node child = read_node(ptr.children[i + 1]);
      print_tree(child, level + 1);
    }
  }
};

} // namespace disk
} // namespace utec