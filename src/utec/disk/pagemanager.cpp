#include "pagemanager.h"

namespace utec {
namespace disk {

pagemanager::pagemanager(std::string file_name, bool trunc)
    : file_name_{std::move(file_name)}, file_{file_name_, open_mode} {
  if (!file_.good() || trunc) {
    empty_ = true;
    file_.close();
    file_.open(file_name_, open_mode | std::ios::trunc);
  }
}

pagemanager::~pagemanager() {
  if (file_.is_open()) {
    file_.close();
  }
}

} // namespace disk
} // namespace utec
