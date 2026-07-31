#pragma once

#include <fstream>
#include <string>
#include <type_traits>

namespace utec {
namespace disk {

class pagemanager {
public:
  explicit pagemanager(std::string file_name, bool trunc = false);

  pagemanager(const pagemanager &) = delete;
  pagemanager &operator=(const pagemanager &) = delete;
  pagemanager(pagemanager &&) noexcept = default;
  pagemanager &operator=(pagemanager &&) noexcept = default;

  ~pagemanager();

  [[nodiscard]] bool is_empty() const noexcept { return empty_; }

  template <class Register>
  void save(const long &n, Register &reg) {
    static_assert(std::is_trivially_copyable_v<Register>,
                  "Register must be trivially copyable for binary I/O");
    file_.clear();
    file_.seekp(n * static_cast<std::streamoff>(sizeof(Register)),
                std::ios::beg);
    file_.write(reinterpret_cast<const char *>(&reg),
                static_cast<std::streamsize>(sizeof(reg)));
  }

  template <class Register>
  bool recover(const long &n, Register &reg) {
    static_assert(std::is_trivially_copyable_v<Register>,
                  "Register must be trivially copyable for binary I/O");
    file_.clear();
    file_.seekg(n * static_cast<std::streamoff>(sizeof(Register)),
                std::ios::beg);
    file_.read(reinterpret_cast<char *>(&reg),
               static_cast<std::streamsize>(sizeof(reg)));
    return file_.gcount() > 0;
  }

  // Marks the register as deleted:
  template <class Register> void erase(const long &n) {
    file_.clear();
    constexpr char mark = 'N';
    file_.seekg(n * static_cast<std::streamoff>(sizeof(Register)),
                std::ios::beg);
    file_.write(&mark, 1);
  }

private:
  static constexpr auto open_mode =
      std::ios::in | std::ios::out | std::ios::binary;

  std::fstream file_;
  std::string file_name_;
  bool empty_{false};
};

} // namespace disk
} // namespace utec
