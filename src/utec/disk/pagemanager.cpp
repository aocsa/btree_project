
#include "pagemanager.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

namespace utec {
namespace disk {

pagemanager::pagemanager(std::string file_name, int page_size)
    : std::fstream(file_name.data(),
                   std::ios::in | std::ios::out | std::ios::binary) {
  pageSize = page_size;
  empty = false;
  fileName = file_name;
  if (!good()) {
    empty = true;
    open(file_name.data(),
         std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
  }
}

pagemanager::~pagemanager() { close(); }

} // namespace disk
} // namespace utec