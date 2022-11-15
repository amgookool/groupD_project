file(REMOVE_RECURSE
  "bootloader/bootloader.bin"
  "bootloader/bootloader.elf"
  "bootloader/bootloader.map"
  "config/sdkconfig.cmake"
  "config/sdkconfig.h"
  "group-project.bin"
  "group-project.map"
  "project_elf_src.c"
  "CMakeFiles/group-project.elf.dir/project_elf_src.c.obj"
  "group-project.elf"
  "group-project.elf.pdb"
  "project_elf_src.c"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/group-project.elf.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
