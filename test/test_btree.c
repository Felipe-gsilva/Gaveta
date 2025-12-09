#include "test_btree.h"

bool test_btree_insert_and_search() {
    BTree* tree = create_btree("assets/configs/default_btree.json");
    if (!tree) return false; 

    int values_to_insert[] = {10, 20, 5, 6, 12, 30, 7, 17};

    size_t n = sizeof(values_to_insert) / sizeof(values_to_insert[0]);

    for (size_t i = 0; i < n; i++) {
        b_insert(tree, &values_to_insert[i], 0);
    }

    for (size_t i = 0; i < n; i++) {
        if (!b_search(tree, (char*)values_to_insert[i], 0)) {
            return false;
        }
    }

    clear_btree(tree);
    return true; // All values found
}
