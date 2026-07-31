#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <utility>

namespace utec {
namespace memory {

template <class T, int BTREE_ORDER = 3> class btree {
  enum state {
    BT_OVERFLOW,
    BT_UNDERFLOW,
    NORMAL,
  };

  struct node {
    int count{};
    std::array<T, BTREE_ORDER + 1> data{};
    std::array<std::unique_ptr<node>, BTREE_ORDER + 2> children{};

    void insert_in_node(int pos, const T &value) {
      for (int j = count; j > pos; --j) {
        data[j] = data[j - 1];
        children[j + 1] = std::move(children[j]);
      }
      data[pos] = value;
      ++count;
    }

    [[nodiscard]] bool is_overflow() const noexcept {
      return count > BTREE_ORDER;
    }
  };

  std::unique_ptr<node> root{std::make_unique<node>()};

  [[nodiscard]] static std::unique_ptr<node> clone(const node *source) {
    if (source == nullptr) {
      return nullptr;
    }

    auto result = std::make_unique<node>();
    result->count = source->count;
    result->data = source->data;
    for (int i = 0; i <= source->count; ++i) {
      result->children[i] = clone(source->children[i].get());
    }
    return result;
  }

public:
  btree() = default;

  btree(const btree &other) : root{clone(other.root.get())} {}

  btree &operator=(const btree &other) {
    if (this != &other) {
      auto copy = clone(other.root.get());
      root = std::move(copy);
    }
    return *this;
  }

  btree(btree &&) noexcept = default;
  btree &operator=(btree &&) noexcept = default;

  void insert(const T &value) {
    if (root == nullptr) {
      root = std::make_unique<node>();
    }

    const int insertion_state = insert(root.get(), value);
    if (insertion_state == BT_OVERFLOW) {
      split_root(root.get(), value);
    }
  }

  int insert(node *ptr, const T &value) {
    int pos = 0;
    while (pos < ptr->count && ptr->data[pos] < value) {
      ++pos;
    }

    if (ptr->children[pos] != nullptr) {
      const int insertion_state = insert(ptr->children[pos].get(), value);
      if (insertion_state == BT_OVERFLOW) {
        split(ptr, pos);
      }
    } else {
      ptr->insert_in_node(pos, value);
    }

    return ptr->is_overflow() ? BT_OVERFLOW : NORMAL;
  }

  void split(node *ptr, int pos) {
    auto node_in_overflow = std::move(ptr->children[pos]);
    auto child1 = std::make_unique<node>();
    auto child2 = std::make_unique<node>();

    int iter = 0;
    int i = 0;
    for (; iter < BTREE_ORDER / 2; ++i, ++iter) {
      child1->children[i] =
          std::move(node_in_overflow->children[iter]);
      child1->data[i] = node_in_overflow->data[iter];
      ++child1->count;
    }
    child1->children[i] = std::move(node_in_overflow->children[iter]);

    const T promoted_value = node_in_overflow->data[iter];
    ++iter;

    for (i = 0; iter < BTREE_ORDER + 1; ++i, ++iter) {
      child2->children[i] =
          std::move(node_in_overflow->children[iter]);
      child2->data[i] = node_in_overflow->data[iter];
      ++child2->count;
    }
    child2->children[i] = std::move(node_in_overflow->children[iter]);

    for (int j = ptr->count; j > pos; --j) {
      ptr->data[j] = std::move(ptr->data[j - 1]);
    }
    for (int j = ptr->count + 1; j > pos + 1; --j) {
      ptr->children[j] = std::move(ptr->children[j - 1]);
    }

    ptr->data[pos] = promoted_value;
    ptr->children[pos] = std::move(child1);
    ptr->children[pos + 1] = std::move(child2);
    ++ptr->count;
  }

  void split_root(node *ptr, [[maybe_unused]] const T &value) {
    auto child1 = std::make_unique<node>();
    auto child2 = std::make_unique<node>();

    int iter = 0;
    int i = 0;
    for (; iter < BTREE_ORDER / 2; ++i, ++iter) {
      child1->children[i] = std::move(ptr->children[iter]);
      child1->data[i] = ptr->data[iter];
      ++child1->count;
    }
    child1->children[i] = std::move(ptr->children[iter]);

    const T promoted_value = ptr->data[iter];
    ++iter;

    for (i = 0; iter < BTREE_ORDER + 1; ++i, ++iter) {
      child2->children[i] = std::move(ptr->children[iter]);
      child2->data[i] = ptr->data[iter];
      ++child2->count;
    }
    child2->children[i] = std::move(ptr->children[iter]);

    ptr->data[0] = promoted_value;
    ptr->children[0] = std::move(child1);
    ptr->children[1] = std::move(child2);
    ptr->count = 1;
  }

  bool find(const T &value) { return find(root.get(), value); }

  bool find(node *ptr, const T &value) {
    if (ptr == nullptr) {
      return false;
    }

    int pos = 0;
    while (pos < ptr->count && ptr->data[pos] < value) {
      ++pos;
    }
    if (pos < ptr->count && ptr->data[pos] == value) {
      return true;
    }
    return find(ptr->children[pos].get(), value);
  }

  void print() {
    print(root.get(), 0);
    std::cout << "________________________\n";
  }

  void print(node *ptr, int level) {
    if (ptr == nullptr) {
      return;
    }

    int i = ptr->count - 1;
    for (; i >= 0; --i) {
      print(ptr->children[i + 1].get(), level + 1);

      for (int k = 0; k < level; ++k) {
        std::cout << "    ";
      }
      std::cout << ptr->data[i] << '\n';
    }
    print(ptr->children[i + 1].get(), level + 1);
  }
};

} // namespace memory
} // namespace utec
