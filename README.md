# GavetaDB - Database Storage Engine

**Gaveta** is a (WIP) database storage engine written in pure C, focusing on raw performance and mainly on my personal desires (the joy of writing C code and learning in the process). 

This project is based on a btree I already did implement in C. The ideia is to allow users to create schemas to interact with the generic data structures in the backend using transactions to ensure data integrity. You can see a plan of the project down below:

- [ ] Generic Data Structures (such as BTree, HashTable, LinkedList, etc)
  - [X] GenericQueue
  - [X] GenericLinkedList
  - [ ] GenericHashTable (with user given hash function)
- [ ] BTree implementation/Refactor from the previous project
- [ ] Schema creation
- [ ] Transactions
- [ ] Query language parser
- [ ] Study how to distribute it later on
- [ ] other things yet to come...

## Compiling

You will need *cmake* in order to run the code:

``` bash

cmake -S . -B target/
cmake --build target/
./target/gaveta

```

## Testing (with CTest)
You can run the tests using CTest, which is included with CMake:

``` bash
cd target/
make test
```
