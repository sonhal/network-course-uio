#include "minunit.h"
#include "../src/darray.h"


static DArray *array = NULL;
static int *val1 = NULL;
static int *val2 = NULL;


char *test_create() {
    array = DArray_create(sizeof(int), 100);
    mu_assert(array != NULL, "DArray_create failed.");
    mu_assert(array->contents != NULL, "contents are wrong in DArray.");
    mu_assert(array->end == 0, "end isn't at the right spot.");
    mu_assert(array->element_size == sizeof(int), "element_size is wrong");
    mu_assert(array->max == 100, "wrong max length on initial DArray");

    return NULL;
}


char *test_destroy(){
    DArray_destroy(array);

    return NULL;
}

char *test_new(){
    array = DArray_create(sizeof(int), 100);

    val1 = DArray_new(array);
    mu_assert(val1 != NULL, "failed to create new element");

    val2 = DArray_new(array);
    mu_assert(val2 != NULL, "failed to create new element");


    return NULL;
}

char *test_set(){
    DArray_set(array, 0, val1);
    DArray_set(array, 1, val2);

    return NULL;
}

 char *test_get() {
     mu_assert(DArray_get(array, 0) == val1, "Wrong first value.");
     mu_assert(DArray_get(array, 1) == val2, "Wrong second value.");
 
    return NULL;
}


char *test_remove()
{
    int *val_check = DArray_remove(array, 0);
    mu_assert(val_check != NULL, "Should not get NULL.");
    mu_assert(*val_check == *val1, "Should get the first value.");
    mu_assert(DArray_get(array, 0) == NULL, "Should be gone.");
    DArray_free(val_check);
 
    val_check = DArray_remove(array, 1);
    mu_assert(val_check != NULL, "Should not get NULL.");
    mu_assert(*val_check == *val2, "Should get the first value.");
    mu_assert(DArray_get(array, 1) == NULL, "Should be gone.");
    DArray_free(val_check);
    
    return NULL;
}

char *test_expand_contract(){
    int old_max = array->max;
    int rc = 0;

    DArray_expand(array);

    mu_assert((unsigned int)array->max == old_max + array->expand_rate, "Wrong size after expand");

    rc = DArray_contract(array);
    mu_assert(rc != -1, "Failed to contract.");
    mu_assert((unsigned int)array->max == array->expand_rate + 1, "Should stay at the expand rate at least");

    rc = DArray_contract(array);
    mu_assert(rc != -1, "Failed to contract.");
    mu_assert((unsigned int)array->max == array->expand_rate + 1, "Should stay at the expand rate at least");

    return NULL;
}

char *test_push_pop(){
    int i = 0;
    for(i = 0; i < 1000; i++){
        int *val = DArray_new(array);
        *val = i * 333;
        DArray_push(array, val);
    }

    mu_assert(array->max == 1201, "Wrong max size.");

    for(i = 999; i >= 0; i--){
        int *val = DArray_pop(array);
        mu_assert(val != NULL, "Should not get a NULL.");
        mu_assert(*val == i * 333, "Wrong value.");
        DArray_free(val);
    }

    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_new);
    mu_run_test(test_set);
    mu_run_test(test_get);
    mu_run_test(test_remove);
    mu_run_test(test_expand_contract);
    mu_run_test(test_push_pop);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);