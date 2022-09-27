#include <iostream>
#include <algorithm>
#include <string>
#include <climits>
#include <vector>
#include "bufferPool.h"
#include <math.h>

using namespace std;
const int MAX = 3; //size of each node

class Node
{
	bool IS_LEAF;
	int* key, size;
	Location* location;
	LLNode** llPtr;
	Node** ptr;
	friend class BPlusTree;

public:
	Node() {
		//dynamic memory allocation
		key = new int[MAX];
		location = new Location[MAX];
		ptr = new Node * [MAX + 1];
		llPtr = new LLNode * [MAX];
	}
};

class BPlusTree
{
	Node* root;
	int numOfNode = 0, heightOfTree = 0, numOfNodeDel = 0;

public:
	BPlusTree()
	{
		CreateRootNode(root);
	}

#pragma region Insert main function
	void Insert(Location location, int keyToInsert, bool isTreeEmpty)
	{
		// when the B+ Tree is empty
		if (isTreeEmpty)
		{
			root->key[0] = keyToInsert;

			LLNode* listHead = NULL;
			AddToEnd(&listHead, location);
			root->llPtr[0] = listHead;
		}
		else
		{
			Node* cursor = root;
			Node* parent = NULL;
			LLNode* listHead = NULL;

			while (!cursor->IS_LEAF)
			{
				parent = cursor;
				for (int i = 0; i < parent->size; i++)
				{
					if (keyToInsert < parent->key[i])
					{
						cursor = parent->ptr[i];
						break;
					}
					if (i == parent->size - 1)
					{
						cursor = parent->ptr[i + 1];
						break;
					}
				}
			}
			for (int i = 0; i < cursor->size;i++)
			{
				if (keyToInsert == cursor->key[i])
				{
					AddToEnd(&cursor->llPtr[i], location);
					return;
				}
			}

			//now cursor is the leaf node in which we'll insert the new key
			if (cursor->size < MAX)
			{
				//sort(cursor->key, cursor->key + cursor->size);
				insertKey(cursor->key, cursor->llPtr, cursor->size, keyToInsert, listHead, location, false);
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = NULL;
			}
			else
			{
				// Overflow scenario

				// step 1: create a MAX + 1 sorted array for overflow scenario
				int tempKey[MAX + 1];
				LLNode* tempLLPtrList[MAX + 1];
				numOfNode += 1;

				for (int i = 0; i < MAX; i++)
				{
					tempKey[i] = cursor->key[i];
					tempLLPtrList[i] = cursor->llPtr[i];
				}

				/*tempKey[MAX] = keyToInsert;
				sort(tempKey, tempKey + (MAX + 1));*/

				insertKey(tempKey, tempLLPtrList, cursor->size, keyToInsert, listHead, location, true);

				// step 2: create a new leaf node
				Node* newLeaf = CreateLeafNode(cursor->ptr[MAX], tempKey, tempLLPtrList);

				// step 3: reconstruct the current node
				ReconstructCurrentNode(cursor, newLeaf, tempKey, tempLLPtrList);

				// step 4: update the parent nodes
				if (cursor == root)
				{
					//if cursor is a root node, we create a new parent root
					Node* parentRoot = new Node;
					CreateParentNode(parentRoot, cursor, newLeaf);

				}
				else
				{
					//insert new key in parent node
					insertInternal(newLeaf->key[0], parent, newLeaf);
				}
			}
		}
	}
#pragma endregion

#pragma region Display main function
	void Display(Node* cursor, int level, int child)
	{
		if (cursor != NULL and level >= 0)
		{
			if (child == 0)
				cout << "Content of root Node = ";
			else
				cout << "Content of " << child << " Child Node = ";

			for (int i = 0; i < cursor->size; i++)
			{
				cout << cursor->key[i] << " ";
			}
			cout << "\n";
			if (cursor->IS_LEAF != true)
			{
				for (int i = 0; i < cursor->size + 1; i++)
				{
					display(cursor->ptr[i], --level, ++child);
				}
			}
		}
	}
#pragma endregion

	int getNumOfNode()
	{
		return numOfNode;
	}

	int getTreeLvl()
	{
		return heightOfTree;
	}

	int getNumOfNodeDel()
	{
		return numOfNodeDel;
	}

	Node* getRoot()
	{
		return root;
	}

private:

#pragma region Insert sub functions
	void insertInternal(int keyToInsert, Node* cursor, Node* child)
	{
		if (cursor->size < MAX)
		{
			//cursor not full, find position to insert new key
			int i = 0;
			while (keyToInsert > cursor->key[i] && i < cursor->size) i++;
			//make space for new key
			for (int j = cursor->size;j > i; j--)
				cursor->key[j] = cursor->key[j - 1];
			//make space for new pointer
			for (int j = cursor->size + 1; j > i + 1; j--)
				cursor->ptr[j] = cursor->ptr[j - 1];
			cursor->key[i] = keyToInsert;
			cursor->size++;
			cursor->ptr[i + 1] = child;
		}
		else
		{
			//overflow in internal, create new internal node
			Node* newInternalNode = new Node;
			//create virtual internal node to store the key
			int tempKey[MAX + 1];
			Node* tempPtr[MAX + 2];
			//increase number of node by 1
			numOfNode += 1;
			for (int i = 0; i < MAX; i++)
				tempKey[i] = cursor->key[i];
			for (int i = 0; i < MAX + 1; i++)
				tempPtr[i] = cursor->ptr[i];
			int i = 0, j;
			while (keyToInsert > tempKey[i] && i < MAX) i++;
			//make space for new key
			for (int j = MAX + 1;j > i; j--)
				tempKey[j] = tempKey[j - 1];

			tempKey[i] = keyToInsert;

			for (int j = MAX + 2;j > i + 1; j--)
				tempPtr[j] = tempPtr[j - 1];

			tempPtr[i + 1] = child;
			newInternalNode->IS_LEAF = false;
			//split cursor into two different nodes
			cursor->size = (MAX + 1) / 2;
			newInternalNode->size = MAX - ((MAX + 1) / 2);
			//give elements and pointers to the new node
			for (i = 0, j = cursor->size + 1; i < newInternalNode->size; i++, j++)
				newInternalNode->key[i] = tempKey[j];
			for (i = 0, j = cursor->size + 1; i < newInternalNode->size + 1; i++, j++)
				newInternalNode->ptr[i] = tempPtr[j];

			if (cursor == root)
			{
				//if cursor is a root node, create new root
				Node* newRoot = new Node;
				newRoot->key[0] = cursor->key[cursor->size];
				newRoot->ptr[0] = cursor;
				newRoot->ptr[1] = newInternalNode;
				newRoot->IS_LEAF = false;
				newRoot->size = 1;
				root = newRoot;
				//increase number of node and tree level by 1
				numOfNode += 1;
				heightOfTree += 1;
			}
			else
			{
				//find depth first search to find parent of cursor recursively
				insertInternal(cursor->key[cursor->size], findParent(root, cursor), newInternalNode);
			}
		}
	}

	Node* findParent(Node* cursor, Node* child)
	{
		//finds parent using depth first traversal and ignores leaf nodes as they cannot be parents
		//also ignores second last level because we will never find parent of a leaf node during insertion using this function
		Node* parent = NULL;
		if (cursor->IS_LEAF || (cursor->ptr[0])->IS_LEAF)
		{
			return NULL;
		}
		for (int i = 0; i < cursor->size + 1; i++)
		{
			if (cursor->ptr[i] == child)
			{
				parent = cursor;
				return parent;
			}
			else
			{
				parent = findParent(cursor->ptr[i], child);
				if (parent != NULL)return parent;
			}
		}
		return parent;
	}

	void AddToEnd(struct LLNode** ppList, Location location)
	{
		/* Temp pointer to store new node */
		struct LLNode* newNode = NULL;
		/* Serves as our runner to find the last node */
		struct LLNode* curr = NULL;

		/* Check ppList is valid */
		if (ppList == NULL)
		{
			cout << "PPList is null." << endl;
			/* Invalid, return don't do anything */
			return;
		}

		/* Allocate the Node memory via malloc */
		//newNode = (struct Node*)malloc(sizeof(struct Node));
		newNode = new LLNode;
		/* Failed allocation, can't do anything else */
		if (newNode == NULL)
			return;

		/* Assign the values to the new Node */
		newNode->location = location;
		newNode->next = NULL;

		/* Find the location to add */
		/* Special case : Head is empty! */
		if (*ppList == NULL)/* *ppList is ListHead*/
		{
			newNode->size = 1;
			*ppList = newNode;
			return;
		}

		/* There are already nodes in the list, need to find the last spot */
		curr = *ppList;
		curr->size += 1;
		int tempSize = curr->size;
		while (curr->next != NULL)
		{
			/* Keep advancing the runner while the next is not empty */
			curr->size = tempSize;
			curr = curr->next;
		}
		/* Here, we have curr at the last position, append the new node */
		curr->next = newNode;
		newNode->size = tempSize;
	}

	void CreateRootNode(Node* &root) {
		root = new Node;
		root->IS_LEAF = true;
		root->size = 1;
		numOfNode = 1;
		heightOfTree = 1;
	}

	void insertKey(int* nodeKeys, LLNode** llNode, int size, int keyToInsert, LLNode* listHead, Location location, bool isOverflow) {
		int i = 0;
		while (keyToInsert > nodeKeys[i] && i < size) i++;

		if (isOverflow) size = size + 1;

		for (int j = size;j > i; j--)
		{
			nodeKeys[j] = nodeKeys[j - 1];
			llNode[j] = llNode[j - 1];
		}
		nodeKeys[i] = keyToInsert;
		AddToEnd(&listHead, location);
		llNode[i] = listHead;
	}

	Node* CreateLeafNode(Node* nextNode, int* tempKeys, LLNode** tempLLPtrList)
	{
		Node* leafNode = new Node();
		leafNode->IS_LEAF = true;
		leafNode->size = floor((MAX + 1) / 2);
		leafNode->ptr[leafNode->size] = nextNode;

		// insert key into the new leaf node
		for (int i = 0,  j = ceil((MAX + 1) / 2); i < leafNode->size; i++, j++)
		{
			leafNode->key[i] = tempKeys[j];
			leafNode->llPtr[i] = tempLLPtrList[j];
		}
		return leafNode;
	}

	void ReconstructCurrentNode(Node* currentNode, Node* newLeafNode, int* tempKeys, LLNode** tempLLPtrList)
	{
		currentNode->size = ceil((MAX + 1) / 2);
		currentNode->ptr[currentNode->size] = newLeafNode;
		currentNode->ptr[MAX] = NULL;

		// insert key into the current node
		for (int i = 0; i < ceil((MAX + 1) / 2); i++)
		{
			currentNode->key[i] = tempKeys[i];
			currentNode->llPtr[i] = tempLLPtrList[i];
		}
	}

	void CreateParentNode(Node* parentRoot, Node* firstLeafNode, Node* secondLeafNode) {
		parentRoot->key[0] = secondLeafNode->key[0];
		parentRoot->ptr[0] = firstLeafNode;
		parentRoot->ptr[1] = secondLeafNode;
		parentRoot->IS_LEAF = false;
		parentRoot->size = 1;

		root = parentRoot;
		numOfNode += 1;
		heightOfTree += 1;
	}
#pragma endregion


};