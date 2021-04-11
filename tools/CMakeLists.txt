# Aseprite PSD Library
# Copyright (C) 2019 Igara Studio S.A.

function(add_psd_tool name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} psd)
endfunction()

add_psd_tool(print_psd_content)
