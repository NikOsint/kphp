// Compiler for PHP (aka KPHP)
// Copyright (c) 2023 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#include "runtime/tl/tl_magics_decoding.h"

// body of these functions is generated by KPHP compiler if TL scheme is passed
// see compiler/code-gen/files/tl2cpp.cpp
const char * __attribute__ ((weak)) tl_magic_convert_to_name_impl(unsigned int magic) noexcept;
void __attribute__ ((weak)) tl_magic_fill_all_functions_impl(array<string> &out) noexcept;

const char *tl_magic_convert_to_name(uint32_t magic) noexcept {
  if (!tl_magic_convert_to_name_impl) {
    return "__unknown__";
  }

  return tl_magic_convert_to_name_impl(magic);
}

array<string> tl_magic_get_all_functions() noexcept {
  array<string> out;
  if (tl_magic_fill_all_functions_impl) {
    tl_magic_fill_all_functions_impl(out);
  }
  return out;
}