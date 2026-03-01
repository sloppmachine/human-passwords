#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <interface/commons.h>
#include <interface/huffmantree.h>

// creates a new huffman tree from a root node
static struct huffmanTree* newHuffmanTree(struct huffmanTreeNode* _root);

// merges two huffman trees, freeing the original ones
static struct huffmanTree* mergeHuffmanTree(struct huffmanTree* _tree1, struct huffmanTree* _tree2);

// creates a new huffman tree node with a character and a weight
static struct huffmanTreeNode* newHuffmanTreeNode(char _content, int _weight);

static void freeHuffmanTreeNode(struct huffmanTreeNode* _huffmanTreeNode);

// makes a new empty list of huffman trees
static struct huffmanTreeList* newHuffmanTreeList();

// frees a list of huffman trees (including each of its entries structs (not their contents, the trees!))
static void freeHuffmanTreeList(struct huffmanTreeList* _huffmanTreeList);

static int getHuffmanTreeListLength(struct huffmanTreeList* _huffmanTreeList);

// this will insert the tree at the correct position in the list. if we were really serious about optimization, this might better be a heap, but we don't care
static void insertHuffmanTree(struct huffmanTreeList* _list, struct huffmanTree* _tree);

// removes the list entry at a specific index. not its content, the tree!
static void removeHuffmanTreeAtIndex(struct huffmanTreeList* _list, int _index);

static void printHuffmanCodesRecursive(struct huffmanTreeNode* _node, char* _prefix, int _prefixLength);

// this list is meant to be sorted by weights of the nodes
struct huffmanTreeList {
    struct huffmanTreeListEntry* first;
};

// this struct exists only to serve huffmanTreeList, it is not meant to be allocated and freed by anything else than methods on huffman tree lists
struct huffmanTreeListEntry {
    struct huffmanTree* content;
    struct huffmanTreeListEntry* next;
};

static struct huffmanTree* newHuffmanTree(struct huffmanTreeNode* _root) {
    struct huffmanTree* toReturn = saferMalloc(sizeof(struct huffmanTree), "huffmanTree");
    toReturn -> root = _root;
    toReturn -> totalWeight = _root -> weight;
    return toReturn;
}

void freeHuffmanTree(struct huffmanTree* _huffmanTree) {
    freeHuffmanTreeNode(_huffmanTree -> root);
    free(_huffmanTree);
}

static struct huffmanTree* mergeHuffmanTree(struct huffmanTree* _tree1, struct huffmanTree* _tree2) {
    struct huffmanTreeNode* newHuffmanTreeRoot = saferMalloc(sizeof(struct huffmanTreeNode), "huffmanTreeNode");
    newHuffmanTreeRoot -> content = ' '; // irrelevant since this is an inner node
    newHuffmanTreeRoot -> leftChild = _tree1 -> root;
    newHuffmanTreeRoot -> rightChild = _tree2 -> root;
    newHuffmanTreeRoot -> weight = (_tree1 -> totalWeight) + (_tree2 -> totalWeight);

    struct huffmanTree* newHuffmanTree = saferMalloc(sizeof(struct huffmanTree), "huffmanTree");
    newHuffmanTree -> root = newHuffmanTreeRoot;
    newHuffmanTree -> totalWeight = newHuffmanTreeRoot -> weight;
    
    free(_tree1);
    free(_tree2);
    return newHuffmanTree;
}


void freeHuffmanTreeNode(struct huffmanTreeNode* _huffmanTreeNode);struct huffmanTreeNode* newHuffmanTreeNode(char _content, int _weight) {
    struct huffmanTreeNode* toReturn = saferMalloc(sizeof(struct huffmanTreeNode), "huffmanTreeNode");
    toReturn -> content = _content;
    toReturn -> weight = _weight;
    toReturn -> leftChild = NULL;
    toReturn -> rightChild = NULL;
    return toReturn;
}

static void freeHuffmanTreeNode(struct huffmanTreeNode* _huffmanTreeNode) {
    if (_huffmanTreeNode -> leftChild) {
        freeHuffmanTreeNode(_huffmanTreeNode -> leftChild);
    }
    if (_huffmanTreeNode -> rightChild) {
        freeHuffmanTreeNode(_huffmanTreeNode -> rightChild);
    }
    free(_huffmanTreeNode);
}

static struct huffmanTreeList* newHuffmanTreeList() {
    struct huffmanTreeList* toReturn = saferMalloc(sizeof(struct huffmanTreeList), "huffmanTreeList");
    toReturn -> first = NULL;
    return toReturn;
}

static void freeHuffmanTreeList(struct huffmanTreeList* _huffmanTreeList) {
    struct huffmanTreeListEntry* current = _huffmanTreeList -> first;
    while (current) {
        struct huffmanTreeListEntry* next = current -> next;
        free(current);
        current = next;
    }
    free(_huffmanTreeList);
}

static int getHuffmanTreeListLength(struct huffmanTreeList* _huffmanTreeList) {
    int length = 0;
    struct huffmanTreeListEntry* current = _huffmanTreeList -> first;
    while (current) {
        current = current -> next;
        length++;
    }
    return length;
}

static void insertHuffmanTree(struct huffmanTreeList* _list, struct huffmanTree* _tree) {
    //printf("the length of the list before inserting is %i\n", getHuffmanTreeListLength(_list));
    // since our trees work on 8-bit characters, there's no need to do it this way for performance. we could also just keep them
    // unsorted and pick them out in linear time. i just like this solution

    // create a new list entry object
    struct huffmanTreeListEntry* newEntry = saferMalloc(sizeof(struct huffmanTreeListEntry), "huffmanTreeListEntry");
    newEntry -> content = _tree;
    newEntry -> next = NULL;

    // check if the list is empty
    if (_list -> first) {
        // if list not empty, insert the new tree before the first old tree that has a higher weight than the new tree
        int weightToInsert = newEntry -> content -> totalWeight;

        if (_list -> first -> content -> totalWeight > weightToInsert) {
            // if the weight to insert is smaller than the weight of the first tree, then we must insert it at the beginning
            newEntry -> next = _list -> first;
            _list -> first = newEntry;
            //printf("inserted %c (%i) at the beginning of list\n", newEntry -> content -> root -> content, newEntry -> content -> totalWeight);
        } else {
            // else we can iterate over the tree and always compare it to the entry after the current one.
            struct huffmanTreeListEntry* current = _list -> first;
            while (true) {
                if (current -> next) {
                    if (current -> next -> content -> totalWeight > weightToInsert) {
                        // if the next tree has a higher weight, we need to insert the new tree in between the current one and the next one
                        newEntry -> next = current -> next;
                        current -> next = newEntry;
                        //printf("inserted %c (%i) after %c\n", newEntry -> content -> root -> content, newEntry -> content -> totalWeight, current -> next -> content -> root -> content);
                        break;
                    } else {
                        current = current -> next;
                    }
                } else {
                    // in this case we have reached the end of the list and can just insert the tree.
                    current -> next = newEntry;
                    //printf("inserted %c (%i) at the end of list\n", newEntry -> content -> root -> content, newEntry -> content -> totalWeight);
                    break;
                }
            }
        }
    } else {
        //printf("inserted %c (%i) as the first entry of the list\n", newEntry -> content -> root -> content, newEntry -> content -> totalWeight);
        _list -> first = newEntry;
    }
    //printf("the length of the list after inserting is %i\n", getHuffmanTreeListLength(_list));
}

static void removeHuffmanTreeAtIndex(struct huffmanTreeList* _list, int _index) {
    int currentIndex = 0;
    struct huffmanTreeListEntry* currentListEntry = _list -> first;
    if (!currentListEntry) {
        printf("Fatal error: tried to remove an element at index %i from an empty list of huffman trees\n", _index);
        exit(EXIT_FAILURE);
    }
    struct huffmanTreeListEntry* previousListEntry = NULL;
    while (true) {
        if (currentIndex == _index) {
            // we need to remove the current list entry.
            if (previousListEntry) {
                // if there is a previous entry, we need to connect it to the next entry (if there is one)
                previousListEntry -> next = currentListEntry -> next;
            } else {
                // if there is no previous list entry, we need to update the start of the list
                _list -> first = currentListEntry -> next;
            }

            free(currentListEntry);
            break;
        } else {
            currentIndex++;
            previousListEntry = currentListEntry;
            currentListEntry = currentListEntry -> next;
            if (!currentListEntry) {
                printf("Fatal error: tried to get element at out-of-range index %i from a list of huffman trees\n", _index);
                exit(EXIT_FAILURE);
            }
        }
    }
}

struct huffmanTree* buildHuffmanTreeFromDistribution(char* _alphabet, int _alphabetSize, int* _distribution) {
    // the alphabet needs to have at least 1 member.

    assert(_alphabetSize >= 1, "the alphabet to build a Huffman tree must not be empty.");

    // first, create a list of huffman trees, one each for each symbol of the alphabet

    struct huffmanTreeList* list = newHuffmanTreeList();
    for (int characterIndex = 0; characterIndex < _alphabetSize; characterIndex++) {
        // ignore the string terminator
        if (_alphabet[characterIndex] != '\0') {
            //printf("trying to insert %c into list...\n", _alphabet[characterIndex]);
            insertHuffmanTree(
                list,
                newHuffmanTree(
                    newHuffmanTreeNode(
                        _alphabet[characterIndex],
                        _distribution[characterIndex]
                    )
                )
            );
        }
    }

    printf("these are the huffman trees currently in the list \n");
    struct huffmanTreeListEntry* currentEntry = list -> first;
    while (currentEntry) {
        struct huffmanTreeNode* root = currentEntry -> content -> root;
        printf(" character %c, weight %i; ", root -> content, root -> weight);
        currentEntry = currentEntry -> next;
    }
    printf("\n");

    // merge the tow trees with the least weight together until only one tree is left.
    while (getHuffmanTreeListLength(list) >= 2) {
        struct huffmanTreeListEntry* firstTreeListEntry = list -> first;
        struct huffmanTreeListEntry* secondTreeListEntry = firstTreeListEntry -> next;
        
        //printf("the list length is currently %i, will now remove an entry\n", getHuffmanTreeListLength(list));
        if (secondTreeListEntry) {
            // extract the trees from the first two list entries, then remove the first two list entries from the list.
            struct huffmanTree* firstTree = firstTreeListEntry -> content;
            struct huffmanTree* secondTree = secondTreeListEntry -> content;
            removeHuffmanTreeAtIndex(list, 1);
            removeHuffmanTreeAtIndex(list, 0);

            // we make a new huffman tree. the root content is irrelevant.
            struct huffmanTree* newTree = mergeHuffmanTree(firstTree, secondTree);
            insertHuffmanTree(list, newTree);
        } else {
            // there is only one tree left and we can break.
            break;
        }
    }
    
    // extract the final tree and then clean up
    struct huffmanTree* toReturn = list -> first -> content;
    freeHuffmanTreeList(list);
    return toReturn;
}

struct huffmanTree* getEmptyRootHuffmanTree() {
    struct huffmanTree* toReturn = newHuffmanTree(
        newHuffmanTreeNode(' ', 0)
    );
    return toReturn;
}

void addEncodedNodeToHuffmanTree(struct huffmanTree* _tree, char _content, char* _prefix, int _prefixLength) {
    struct huffmanTreeNode* current = _tree -> root;
    assert(current != NULL, "can't add an encoded node to a rootless tree\n"); // the !=NULL is for the compiler

    // we now iteratively trace down the tree
    int depth = 0;
    while (true) {
        if (depth == _prefixLength) {
            // this is where the node needs to be inserted
            current -> content = _content;
            break;
        } else {
            // in this case we need to trace further down the tree and add children if necessary
            if (_prefix[depth] == '0') {
                if (!current -> leftChild) {
                    current -> leftChild = newHuffmanTreeNode(_content, 0);
                }
                current = current -> leftChild;
            } else {
                if (!current -> rightChild) {
                    current -> rightChild = newHuffmanTreeNode(_content, 0);
                }
                current = current -> rightChild;
            }
            depth++;
        }
    }
}

void printHuffmanCodes(struct huffmanTree* _tree) {
    printf("Huffman codes:\n");
    printf("(Begin)\n");
    // use a dummy string (consisting only of the terminator) as a prefix for the root
    printHuffmanCodesRecursive(_tree -> root, "", 0);
    printf("(End)\n");
}

static void printHuffmanCodesRecursive(struct huffmanTreeNode* _node, char* _prefix, int _prefixLength) {
    bool isInner = false;
    // if the node has children, call this method on them. their huffman tree prefix derives from the current one.

    // technically, if a node has one child then it must have two, but having 2 seperate ifs is more intuitive. my code my rules
    if (_node -> leftChild) {
        isInner = true;
        char* leftChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
        memcpy(leftChildPrefix, _prefix, _prefixLength);
        leftChildPrefix[_prefixLength] = '0';
        printHuffmanCodesRecursive(_node -> leftChild, leftChildPrefix, _prefixLength + 1);
        free(leftChildPrefix);
    }
    if (_node -> rightChild) {
        isInner = true;
        char* rightChildPrefix = saferMalloc(sizeof(char) * _prefixLength + 1, "huffman tree prefix");
        memcpy(rightChildPrefix, _prefix, _prefixLength);
        rightChildPrefix[_prefixLength] = '1';
        printHuffmanCodesRecursive(_node -> rightChild, rightChildPrefix, _prefixLength + 1);
        free(rightChildPrefix);
    }
    if (!isInner) {
        printFromCharArray(_prefix, _prefixLength);
        printf(" : %c\n", _node -> content);
    }
}