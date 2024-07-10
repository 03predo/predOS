#include <unity.h>
#include <string.h>

#include "fat.h"

#include "mock_sys_timer.h"

void setUp (void) {}
void tearDown (void) {}

void test(void){
  sys_uptime_IgnoreAndReturn(0);
  fat_directory_entry_t entry;
  memset(&entry, 0, sizeof(fat_directory_entry_t));
  entry.name[0] = 'h';
  entry.name[1] = 'i';
  entry.name[2] = '\0';
  fat_print_entry(entry);
  TEST_ASSERT_EQUAL(0, 0);
}

