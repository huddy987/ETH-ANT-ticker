/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)


/* TODO: Should be able to squeeze out a bit more if needed (0xED000, 0x3F600), were my calculations wrong? */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x00012000, LENGTH = 0xE0000
  RAM (rwx) :  ORIGIN = 0x20000B80, LENGTH = 0x30000
}

SECTIONS
{
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf52_common.ld"
