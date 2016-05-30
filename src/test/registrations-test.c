
#include <stdlib.h>
#include "CUnit/Basic.h"
#include "registrations.h"
#include "registrations-test.h"

size_t _count_entries(char** component_list) {
  size_t number_of_entries = 0;
  char **current_component;
  for (current_component = component_list; *current_component != NULL; current_component++) {
    number_of_entries++;
  }
  return number_of_entries;
}

void test_count_entries() {
  char **component_list = ambitv_get_component_list();
  CU_ASSERT_EQUAL(_count_entries(component_list), 6);
  ambitv_free_component_list(component_list);
}

void test_entries() {
  char **component_list = ambitv_get_component_list();
  CU_ASSERT_STRING_EQUAL(component_list[0], "v4l2-grab-source");
  CU_ASSERT_STRING_EQUAL(component_list[1], "timer-source");
  CU_ASSERT_STRING_EQUAL(component_list[2], "avg-color-processor");
  CU_ASSERT_STRING_EQUAL(component_list[5], "lpd8806-spidev-sink");
  ambitv_free_component_list(component_list);
}

void _assert_entries(size_t expected_number_of_entries) {
  char **component_list = ambitv_get_component_list();
  CU_ASSERT_EQUAL_FATAL(_count_entries(component_list), expected_number_of_entries);
  ambitv_free_component_list(component_list);
}

void
myfn(const char* a, int b, char** c) {
}

void test_append_entry() {
  _assert_entries(6);

   struct ambitv_component_registration *component1 = (struct ambitv_component_registration *)malloc(sizeof(struct ambitv_component_registration));
   component1->name = "test-source1";
   component1->constructor = (void* (*)(const char*, int, char**))myfn;

  _assert_entries(6);
  char **component_list_initial = ambitv_get_component_list();
  CU_ASSERT_STRING_EQUAL(component_list_initial[0], "v4l2-grab-source");
  CU_ASSERT_STRING_EQUAL(component_list_initial[5], "lpd8806-spidev-sink");
  ambitv_free_component_list(component_list_initial);

  ambitv_append_component(component1);

  _assert_entries(7);

  char **component_list_plus_one = ambitv_get_component_list();
  CU_ASSERT_STRING_EQUAL(component_list_plus_one[0], "v4l2-grab-source");
  CU_ASSERT_STRING_EQUAL(component_list_plus_one[5], "lpd8806-spidev-sink");
  CU_ASSERT_STRING_EQUAL(component_list_plus_one[6], "test-source1");
  ambitv_free_component_list(component_list_plus_one);

   struct ambitv_component_registration *component2 = (struct ambitv_component_registration *)malloc(sizeof(struct ambitv_component_registration));
   component2->name = "test-source2";
   component2->constructor = (void* (*)(const char*, int, char**))myfn;

  ambitv_append_component(component2);

  _assert_entries(8);

  char **component_list_plus_two = ambitv_get_component_list();
  CU_ASSERT_STRING_EQUAL(component_list_plus_two[0], "v4l2-grab-source");
  CU_ASSERT_STRING_EQUAL(component_list_plus_two[7], "test-source2");
  ambitv_free_component_list(component_list_plus_two);
}

int registrations_test_add_suite() {
   CU_pSuite pSuite = NULL;

   /* add a suite to the registry */
   pSuite = CU_add_suite("registrations", NULL, NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test_count_entries", test_count_entries)  ||
       NULL == CU_add_test(pSuite, "test_entries", test_entries) ||
       NULL == CU_add_test(pSuite, "test_append_entry", test_append_entry))
      ) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   return CU_get_error();
}

