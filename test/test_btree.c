#include "../src/modules/ds/BTree.h"

bool test_btree_insert_and_search() {
    BTree* tree = create_btree("assets/default_btree.conf");

    size_t n = sizeof(values_to_insert) / sizeof(values_to_insert[0]);

    for (size_t i = 0; i < n; i++) {
        b_insert(tree, values_to_insert[i], NULL);
    }

    for (size_t i = 0; i < n; i++) {
        if (!b_search(tree, &values_to_insert[i], NULL)) {
            return false;
        }
    }

    b(tree);
    return true; // All values found
}
