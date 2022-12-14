#include <iostream>
#include <algorithm>
#include <string>
#include <climits>
#include <vector>
#include "diskStorage.h"
#include <math.h>

using namespace std;
const int MAX_200MB = 10;
const int MAX_500MB = 25;

class Node
{
	bool IS_LEAF;
	int *key, size;
	Location *location;
	LLNode **llPtr;
	Node **ptr;
	friend class BPlusTree;

public:
	Node(int maxOfNode)
	{
		if (maxOfNode == MAX_200MB)
		{
			// dynamic memory allocation
			key = new int[MAX_200MB];
			location = new Location[MAX_200MB];
			ptr = new Node *[MAX_200MB + 1];
			llPtr = new LLNode *[MAX_200MB];
		}
		else if (maxOfNode == MAX_500MB)
		{
			// dynamic memory allocation
			key = new int[MAX_500MB];
			location = new Location[MAX_500MB];
			ptr = new Node *[MAX_500MB + 1];
			llPtr = new LLNode *[MAX_500MB];
		}
	}
};

class BPlusTree
{
	Node *root;
	int numOfNode = 0, heightOfTree = 0, numOfNodeDel = 0, MAX = 0;

public:
	BPlusTree(int maxOfNode_)
	{
		MAX = maxOfNode_;
		CreateRootNode(root);
	}

#pragma region Insert main function
	void Insert(Location location, int keyToInsert, bool isTreeEmpty)
	{
		// when the B+ Tree is empty
		if (isTreeEmpty)
		{
			root->key[0] = keyToInsert;
			root->llPtr[0] = createLLNode(location);
		}
		else
		{
			Node *cursor = root;
			Node *parent = NULL;

			// step1: travel to the leaf node while keep track the parent node
			while (!cursor->IS_LEAF)
			{
				bool isFound = false;
				parent = cursor;

				// step1.1: go to the child node if the keyToInsert is smaller than the parent key
				for (int i = 0; i < cursor->size; i++)
				{
					if (keyToInsert < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						isFound = true;
						break;
					}
				}

				// step1.2: else go to the last child node of this parent node
				if (!isFound)
				{
					cursor = cursor->ptr[cursor->size];
				}
			}

			// the cursor reached the leaf nodes level

			// step2: first check if the key already exist, append the key to the linked list
			int dupKeyIndex = isDuplicateKeyFound(cursor, keyToInsert);
			if (dupKeyIndex > -1)
			{
				updateLLNode(&cursor->llPtr[dupKeyIndex], location);
			}
			// step3.1: check if the leaf node is not full, add the key into the leaf node in order
			else if (cursor->size < MAX)
			{
				// sort(cursor->key, cursor->key + cursor->size);
				insertKey(cursor->key, cursor->llPtr, cursor->size, keyToInsert, location, false);
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = NULL;
			}
			// step3.2: else split the leaf node into 2 leaf nodes
			else
			{
				// Overflow scenario

				if (MAX == MAX_200MB)
				{
					// step3.2.1: create a MAX + 1 sorted array for overflow scenario
					int overflowArrKey[MAX_200MB + 1];
					LLNode *overflowArrLLNode[MAX_200MB + 1];
					numOfNode += 1;

					for (int i = 0; i < MAX; i++)
					{
						overflowArrKey[i] = cursor->key[i];
						overflowArrLLNode[i] = cursor->llPtr[i];
					}

					insertKey(overflowArrKey, overflowArrLLNode, cursor->size, keyToInsert, location, true);

					// step3.2.2: create a new leaf node
					Node *newLeaf = CreateLeafNode(cursor->ptr[MAX], overflowArrKey, overflowArrLLNode);

					// step3.2.3: reconstruct the current node
					ReconstructCurrentNode(cursor, newLeaf, overflowArrKey, overflowArrLLNode);

					// step3.2.4: update the parent nodes
					if (cursor == root)
					{
						// if cursor is a root node, we create a new parent root
						Node *parentRoot = new Node(MAX);
						CreateParentNode(parentRoot, cursor, newLeaf);
					}
					else
					{
						// insert new key in parent node
						InsertParent(parent, newLeaf);
					}
				}
				else if (MAX == MAX_500MB)
				{
					// step3.2.1: create a MAX + 1 sorted array for overflow scenario
					int overflowArrKey[MAX_500MB + 1];
					LLNode *overflowArrLLNode[MAX_500MB + 1];
					numOfNode += 1;

					for (int i = 0; i < MAX; i++)
					{
						overflowArrKey[i] = cursor->key[i];
						overflowArrLLNode[i] = cursor->llPtr[i];
					}

					insertKey(overflowArrKey, overflowArrLLNode, cursor->size, keyToInsert, location, true);

					// step3.2.2: create a new leaf node
					Node *newLeaf = CreateLeafNode(cursor->ptr[MAX], overflowArrKey, overflowArrLLNode);

					// step3.2.3: reconstruct the current node
					ReconstructCurrentNode(cursor, newLeaf, overflowArrKey, overflowArrLLNode);

					// step3.2.4: update the parent nodes
					if (cursor == root)
					{
						// if cursor is a root node, we create a new parent root
						Node *parentRoot = new Node(MAX);
						CreateParentNode(parentRoot, cursor, newLeaf);
					}
					else
					{
						// insert new key in parent node
						InsertParent(parent, newLeaf);
					}
				}
			}
		}
	}
#pragma endregion

#pragma region Search main function
	void retrievedetails(int lowlimit, int highlimit, diskStorage *diskStorage)
	{
        vector<int> tIndex;
        vector<string> tData;
        vector<uchar *> tAddress;
        vector<vector<int>> indxNode;
        vector<vector<string>> dataBlk;
        
		int countIndexAccess = 1, countBlkAccess = 1, countMatch = 0;
        float sumRating = 0;
        bool retrievenode = true;
		if (root == NULL)
		{
			cout << "Tree is empty." << endl;
		}
		else
		{
			Node *cursor = root;
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					for (int j = 0; j < cursor->size; j++)
					{
						if (indxNode.size() < 5 and retrievenode == true)
							tIndex.push_back(cursor->key[j]);
					}

					if (lowlimit < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						countIndexAccess += 1;
						break;
					}
					if (i == cursor->size - 1)
					{
						cursor = cursor->ptr[i + 1];
						countIndexAccess += 1;
						break;
					}
					retrievenode = false;
				}
				if (indxNode.size() < 5)
					indxNode.push_back(tIndex);
				tIndex.clear();
				retrievenode = true;
			}
			while (retrievenode)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					if (indxNode.size() < 5)
						tIndex.push_back(cursor->key[i]);

					if (cursor->key[i] >= lowlimit and cursor->key[i] <= highlimit)
					{
						tAddress = getAllLLNode(cursor->llPtr[i], tAddress);
						countMatch += cursor->llPtr[i]->size;
					}
					else if (cursor->key[i] > highlimit)
						retrievenode = false;

					if (i == cursor->size - 1 and retrievenode == true)
					{
						cursor = cursor->ptr[i + 1];
						countIndexAccess += 1;
						countBlkAccess += 1;
						break;
					}
				}
				if (indxNode.size() < 5)
					indxNode.push_back(tIndex);
				tIndex.clear();
			}

			countBlkAccess = tAddress.size();
			for (uint i = 0; i < countBlkAccess; i++)
			{
				for (uint j = 20; j <= diskStorage->getBlockSize(); j += 20)
				{
					void *recordAddress = (uchar *)tAddress[i] + j;
					if (dataBlk.size() < 5)
						tData.push_back((*(Record *)recordAddress).tconst);
					if ((*(Record *)recordAddress).numVotes >= lowlimit and (*(Record *)recordAddress).numVotes <= highlimit)
						sumRating += (*(Record *)recordAddress).averageRating;
				}
				if (dataBlk.size() < 5)
					dataBlk.push_back(tData);
				tData.clear();
			}

			cout << "Content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indxNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indxNode[i].size(); j++)
					cout << indxNode[i][j] << " | ";
				cout << endl;
			}
			cout << "Number of Index Nodes accessed = " << countIndexAccess << endl;

			cout << endl
				 << "Content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlk.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlk[i].size(); j++)
					cout << dataBlk[i][j] << " | ";
				cout << endl;
			}
			cout << "Number of Data Blocks accessed = " << countBlkAccess << endl
				 << endl;

			cout << "Total number of matches = " << countMatch << endl;
			if (countMatch > 0)
				cout << "The rating average of 'averageRating' = " << sumRating / countMatch << endl;
			else
				cout << "No records were found due to 0 match results." << endl;
		}
	}

#pragma endregion

#pragma region Delete main function
	void RemoveEvent(int x, diskStorage *diskStorage)

	{
		if(root != NULL)
		{
			Node* cursor = root;
			Node* parent = NULL;
			int leftChildren, rightChildern;
			bool isUnderFlow = false;

			// travel to the leaf node while keep track the parent node
			while (!cursor->IS_LEAF)
			{
				bool isFound = false;
				parent = cursor;
				for (int i = 0; i < cursor->size; i++)
				{
					leftChildren = i - 1;
					rightChildern = i + 1;

					if (x < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						isFound = true;
						break;
					}
				}
				if (!isFound)
				{
					leftChildren = cursor->size - 1;
					rightChildern = cursor->size - 1 + 2;

					cursor = cursor->ptr[cursor->size - 1 + 1];
				}
			}

			// search for the key if it exists and remove the linked list
			bool found = false;
			int pos;

			for (pos = 0; pos < cursor->size; pos++)
			{
				if (cursor->key[pos] == x)
				{
					found = true;
					deleteLLNode(&cursor->llPtr[pos], diskStorage);
					break;
				}
			}

			if (!found) // if key does not exist in that leaf node
			{
				cout << "No key is found! Deletion failed" << endl;
				return;
			}

			if (cursor == root)
			{
				heightOfTree -= 1;
				for (int i = 0; i < MAX + 1; i++)
				{
					cursor->ptr[i] = NULL;
				}

				if (cursor->size == 0)
				{
					deleteNode(cursor);

					root = NULL;
				}
				return;
			}

			// deleting the key
			for (int i = pos; i < cursor->size; i++)
			{
				cursor->key[i] = cursor->key[i + 1];
				cursor->llPtr[i] = cursor->llPtr[i + 1];
			}

			cursor->size--;

			// underflow
			if (cursor->size < floor((MAX + 1) / 2))
			{
				isUnderFlow = true;
			}

			cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
			cursor->ptr[cursor->size + 1] = NULL;

			if (isUnderFlow)
			{
				// if the node keys is less than minimum, check if sibling able to borrow key
				bool isCanBorrowSiblingKey = canBorrowSiblingKey(parent, leftChildren, rightChildern);

				if (isCanBorrowSiblingKey)
				{
					bool isLeftSibling = true;
					Node *siblingNode = NULL;
					isLeftSibling = isLeftSiblingNode(&siblingNode, parent, leftChildren, rightChildern);

					if (isLeftSibling)
					{
						borrowSiblingKey(cursor, siblingNode, true);

						shiftPointer(cursor);

						shiftPointerSibling(siblingNode, cursor);

						parent->key[leftChildren] = cursor->key[0];
						parent->llPtr[leftChildren] = cursor->llPtr[0];
					}
					else
					{
						borrowSiblingKey(cursor, siblingNode, false);

						shiftPointer(cursor);

						shiftPointerSibling(siblingNode, siblingNode->ptr[siblingNode->size + 1]);

						parent->key[rightChildern - 1] = siblingNode->key[0];
						parent->llPtr[rightChildern - 1] = siblingNode->llPtr[0];
					}
				}
				else
				{
					// else we will need to merge with sibling node and delete the node

					if (leftChildren >= 0)
					{
						Node *leftNode = parent->ptr[leftChildren];

						// merge the current node keys to left sibling node
						for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++)
						{

							leftNode->key[i] = cursor->key[j];
							leftNode->llPtr[i] = cursor->llPtr[j];

							// increase the size of the left sibling node
							leftNode->size += 1;
						}

						// update the pointer
						leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];

						removeParent(parent->key[leftChildren], parent, cursor);

						deleteNode(cursor);
					}
					else if (rightChildern <= parent->size)
					{
						Node *rightNode = parent->ptr[rightChildern];

						// merge the current node keys to left sibling node
						for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++)
						{
							cursor->key[i] = rightNode->key[j];
							cursor->llPtr[i] = rightNode->llPtr[j];

							// increase the size of the current node
							cursor->size += 1;
						}

						// update the pointer
						cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];

						removeParent(parent->key[rightChildern - 1], parent, rightNode);

						deleteNode(rightNode);
					}

					// update the total nodes
					numOfNode -= 1;
					numOfNodeDel += 1;
				}
			}
		}

		return;
	}
#pragma endregion

#pragma region Display main function
	void Display(Node *cursor, int level, int child)
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
					Display(cursor->ptr[i], --level, ++child);
				}
			}
		}
	}
#pragma endregion

#pragma region Common main function

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

	Node *getRoot()
	{
		return root;
	}
#pragma endregion

private:
#pragma region Insert sub functions
	void InsertParent(Node *cursor, Node *child)
	{
		Node *lowestBoundLeafNode = findLowestBoundLeafNode(child);
		int keyToInsert = lowestBoundLeafNode->key[0];

		if (cursor->size < MAX)
		{
			// cursor is not full, find the position to insert the new key
			int i = 0;
			while (keyToInsert > cursor->key[i] && i < cursor->size)
				i++;
			// make space for the new key
			for (int j = cursor->size; j > i; j--)
				cursor->key[j] = cursor->key[j - 1];
			// make space for the new pointer
			for (int j = cursor->size + 1; j > i + 1; j--)
				cursor->ptr[j] = cursor->ptr[j - 1];
			cursor->key[i] = keyToInsert;
			cursor->size++;
			cursor->ptr[i + 1] = child;
		}
		else
		{
			if (MAX == MAX_200MB)
			{
				// overflow in internal, create new internal node
				Node *newInternalNode = new Node(MAX);
				// create virtual internal node to store the key
				int tempKey[MAX_200MB + 1];
				Node *tempPtr[MAX_200MB + 2];
				// increase the number of node by 1
				numOfNode += 1;
				for (int i = 0; i < MAX; i++)
					tempKey[i] = cursor->key[i];
				for (int i = 0; i < MAX + 1; i++)
					tempPtr[i] = cursor->ptr[i];
				int i = 0, j;
				while (keyToInsert > tempKey[i] && i < MAX)
					i++;
				// make space for the new key
				for (int j = MAX + 1; j > i; j--)
					tempKey[j] = tempKey[j - 1];

				tempKey[i] = keyToInsert;

				for (int j = MAX + 2; j > i + 1; j--)
					tempPtr[j] = tempPtr[j - 1];

				tempPtr[i + 1] = child;
				newInternalNode->IS_LEAF = false;
				// split the cursor into two different nodes
				cursor->size = (MAX + 1) / 2;
				// update the current internal node
				for (int h = 0; h < cursor->size; h++)
					cursor->key[h] = tempKey[h];

				for (int k = 0; k < cursor->size + 1; k++)
					cursor->ptr[k] = tempPtr[k];

				newInternalNode->size = MAX - ((MAX + 1) / 2);
				// give elements and pointers to the new node
				for (i = 0, j = cursor->size + 1; i < newInternalNode->size; i++, j++)
					newInternalNode->key[i] = tempKey[j];
				for (i = 0, j = cursor->size + 1; i < newInternalNode->size + 1; i++, j++)
					newInternalNode->ptr[i] = tempPtr[j];

				if (cursor == root)
				{
					// if the cursor is a root node, create new parent root
					Node *parentRoot = new Node(MAX);
					CreateParentNode(parentRoot, cursor, newInternalNode);
				}
				else
				{
					// use depth first search to find the parent of the cursor recursively
					InsertParent(findParent(root, cursor), newInternalNode);
				}
			}
			else if (MAX == MAX_500MB)
			{
				// overflow in internal, create new internal node
				Node *newInternalNode = new Node(MAX);
				// create virtual internal node to store the key
				int tempKey[MAX_500MB + 1];
				Node *tempPtr[MAX_500MB + 2];
				// increase the number of node by 1
				numOfNode += 1;
				for (int i = 0; i < MAX; i++)
					tempKey[i] = cursor->key[i];
				for (int i = 0; i < MAX + 1; i++)
					tempPtr[i] = cursor->ptr[i];
				int i = 0, j;
				while (keyToInsert > tempKey[i] && i < MAX)
					i++;
				// make space for the new key
				for (int j = MAX + 1; j > i; j--)
					tempKey[j] = tempKey[j - 1];

				tempKey[i] = keyToInsert;

				for (int j = MAX + 2; j > i + 1; j--)
					tempPtr[j] = tempPtr[j - 1];

				tempPtr[i + 1] = child;
				newInternalNode->IS_LEAF = false;
				// split the cursor into two different nodes
				cursor->size = (MAX + 1) / 2;
				// update the current internal node
				for (int h = 0; h < cursor->size; h++)
					cursor->key[h] = tempKey[h];

				for (int k = 0; k < cursor->size + 1; k++)
					cursor->ptr[k] = tempPtr[k];

				newInternalNode->size = MAX - ((MAX + 1) / 2);
				// give elements and pointers to the new node
				for (i = 0, j = cursor->size + 1; i < newInternalNode->size; i++, j++)
					newInternalNode->key[i] = tempKey[j];
				for (i = 0, j = cursor->size + 1; i < newInternalNode->size + 1; i++, j++)
					newInternalNode->ptr[i] = tempPtr[j];

				if (cursor == root)
				{
					// if the cursor is a root node, create new parent root
					Node *parentRoot = new Node(MAX);
					CreateParentNode(parentRoot, cursor, newInternalNode);
				}
				else
				{
					// use depth first search to find the parent of the cursor recursively
					InsertParent(findParent(root, cursor), newInternalNode);
				}
			}
		}
	}

	LLNode *createLLNode(Location location)
	{
		LLNode *newLLNode = new LLNode;

		newLLNode->location = location;
		newLLNode->size = 1;
		newLLNode->next = NULL;

		return newLLNode;
	}

	void updateLLNode(struct LLNode **llNode, Location location)
	{
		struct LLNode *dupKeyNode = new LLNode;
		dupKeyNode->location = location;
		dupKeyNode->next = NULL;

		// step1: find the last node of the linked list
		while ((*llNode)->next != NULL)
		{
			(*llNode) = (*llNode)->next;
		}

		// step2: append the new node to the last node
		(*llNode)->size += 1;
		(*llNode)->next = dupKeyNode;
		dupKeyNode->size = (*llNode)->size;
	}

	void CreateRootNode(Node *&root)
	{
		root = new Node(MAX);
		root->IS_LEAF = true;
		root->size = 1;
		numOfNode = 1;
		heightOfTree = 1;
	}

	void insertKey(int *nodeKeys, LLNode **llNode, int size, int keyToInsert, Location location, bool isOverflow)
	{
		int i = 0;
		while (keyToInsert > nodeKeys[i] && i < size)
			i++;

		if (isOverflow)
			size = size + 1;

		for (int j = size; j > i; j--)
		{
			nodeKeys[j] = nodeKeys[j - 1];
			llNode[j] = llNode[j - 1];
		}
		nodeKeys[i] = keyToInsert;
		llNode[i] = createLLNode(location);
	}

	Node *CreateLeafNode(Node *nextNode, int *overflowArrKey, LLNode **overflowArrLLNode)
	{
		Node *leafNode = new Node(MAX);
		leafNode->IS_LEAF = true;
		leafNode->size = floor((MAX + 1) / 2);
		leafNode->ptr[leafNode->size] = nextNode;

		// insert key into the new leaf node
		for (int i = 0, j = ceil((MAX + 1) / 2); i < leafNode->size; i++, j++)
		{
			leafNode->key[i] = overflowArrKey[j];
			leafNode->llPtr[i] = overflowArrLLNode[j];
		}
		return leafNode;
	}

	void ReconstructCurrentNode(Node *currentNode, Node *newLeafNode, int *overflowArrKey, LLNode **overflowArrLLNode)
	{
		currentNode->size = ceil((MAX + 1) / 2);
		currentNode->ptr[currentNode->size] = newLeafNode;
		currentNode->ptr[MAX] = NULL;

		// insert key into the current node
		for (int i = 0; i < ceil((MAX + 1) / 2); i++)
		{
			currentNode->key[i] = overflowArrKey[i];
			currentNode->llPtr[i] = overflowArrLLNode[i];
		}
	}

	void CreateParentNode(Node *parentRoot, Node *firstLeafNode, Node *secondLeafNode)
	{
		Node *lowestBoundLeafNode = findLowestBoundLeafNode(secondLeafNode);
		parentRoot->key[0] = lowestBoundLeafNode->key[0];
		parentRoot->ptr[0] = firstLeafNode;
		parentRoot->ptr[1] = secondLeafNode;
		parentRoot->IS_LEAF = false;
		parentRoot->size = 1;

		root = parentRoot;
		numOfNode += 1;
		heightOfTree += 1;
	}

	Node *findLowestBoundLeafNode(Node *cursor)
	{
		Node *lowestBoundLeafNode = NULL;
		while (!cursor->IS_LEAF)
		{
			cursor = cursor->ptr[0];
		}
		lowestBoundLeafNode = cursor;
		return lowestBoundLeafNode;
	}

	int isDuplicateKeyFound(Node *leafNode, int keyToInsert)
	{
		for (int i = 0; i < leafNode->size; i++)
		{
			if (keyToInsert == leafNode->key[i])
				return i;
		}
		return -1;
	}
#pragma endregion

#pragma region Search sub functions
	vector<uchar *> getAllLLNode(LLNode *list, vector<uchar *> &tAddress)
	{
		LLNode *curr = list;
		if (list == NULL)
		{
			cout << ("List is Empty") << endl;
			return tAddress;
		}
		while (curr != NULL)
		{
			if (find(tAddress.begin(), tAddress.end(), curr->location.blockLocation) == tAddress.end())
				tAddress.push_back(curr->location.blockLocation);
			curr = curr->next;
		}
		return tAddress;
	}
	Record getRecords(Location location)
	{
		void *mainMemoryAddress = (uchar *)location.blockLocation + location.offset;
		return (*(Record *)mainMemoryAddress);
	}
#pragma endregion

#pragma region Delete sub functions
	void removeParent(int x, Node *cursor, Node *child)
	{
		// deleting the key x first
		// checking if key from root is to be deleted
		if (cursor == root && cursor->size == 1)
		{
			heightOfTree -= 1;
			numOfNode -= 1;
			numOfNodeDel += 1;
			// if only one key is left, change root
			if (cursor->ptr[1] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				delete child;
				root = cursor->ptr[0];
				delete[] cursor->key;
				delete[] cursor->ptr;
				delete cursor;
				return;
			}
			else if (cursor->ptr[0] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				root = cursor->ptr[1];
				delete[] cursor->key;
				delete[] cursor->ptr;
				return;
			}
		}
		int keyToDeleteIndex;
		for (keyToDeleteIndex = 0; keyToDeleteIndex < cursor->size; keyToDeleteIndex++)
		{
			if (cursor->key[keyToDeleteIndex] == x)
				break;
		}
		for (int i = keyToDeleteIndex; i < cursor->size; i++)
			cursor->key[i] = cursor->key[i + 1];

		// now deleting the pointer child
		for (keyToDeleteIndex = 0; keyToDeleteIndex < cursor->size + 1; keyToDeleteIndex++)
		{
			if (cursor->ptr[keyToDeleteIndex] == child)
				break;
		}
		for (int i = keyToDeleteIndex; i < cursor->size + 1; i++)
			cursor->ptr[i] = cursor->ptr[i + 1];
		cursor->size--;
		if (cursor->size >= (MAX + 1) / 2 - 1) // no underflow
			return;

		if (cursor == root)
			return;
		Node *parent = findParent(root, cursor);
		int leftSibling, rightSibling = -1;
		// finding left n right sibling of cursor
		for (keyToDeleteIndex = 0; keyToDeleteIndex < parent->size + 1; keyToDeleteIndex++)
		{
			if (parent->ptr[keyToDeleteIndex] == cursor)
			{
				leftSibling = keyToDeleteIndex - 1;
				rightSibling = keyToDeleteIndex + 1;
				break;
			}
		}
		// borrow key from left internal sibling node
		if (leftSibling >= 0) // if left sibling exists
		{
			Node *leftNode = parent->ptr[leftSibling];
			// check if the left sibling node has enough key to borrow
			if (leftNode->size >= (MAX + 1) / 2)
			{
				// shift key arr to insert the key
				for (int i = cursor->size; i > 0; i--)
					cursor->key[i] = cursor->key[i - 1];

				// shift child nodes arr to insert the child nodes
				for (int i = cursor->size + 1; i > 0; i--)
					cursor->ptr[i] = cursor->ptr[i - 1];

				// borrow key
				cursor->key[0] = parent->key[leftSibling];
				parent->key[leftSibling] = leftNode->key[leftNode->size - 1];

				// update child node
				cursor->ptr[0] = leftNode->ptr[leftNode->size];

				// update size for both nodes
				cursor->size++;
				leftNode->size--;

				return;
			}
		}

		// borrow key from right internal sibling node
		if (rightSibling <= parent->size) // check if right sibling exist
		{
			Node *rightNode = parent->ptr[rightSibling];
			// check if the right sibling node has enough key to borrow
			if (rightNode->size >= (MAX + 1) / 2)
			{
				// borrow key
				cursor->key[cursor->size] = parent->key[keyToDeleteIndex];
				parent->key[keyToDeleteIndex] = rightNode->key[0];

				// update child node
				cursor->ptr[cursor->size + 1] = rightNode->ptr[0];

				// remove the borrowed key
				for (int i = 0; i < rightNode->size - 1; i++)
					rightNode->key[i] = rightNode->key[i + 1];

				// remove the child node
				for (int i = 0; i < rightNode->size; ++i)
					rightNode->ptr[i] = rightNode->ptr[i + 1];

				// update size for both nodes
				cursor->size++;
				rightNode->size--;

				return;
			}
		}

		// merge with other internal sibling
		if (leftSibling >= 0)
		{
			// leftnode + parent key + cursor
			Node *leftNode = parent->ptr[leftSibling];
			leftNode->key[leftNode->size] = parent->key[leftSibling];
			for (int i = leftNode->size + 1, j = 0; j < cursor->size; j++)
				leftNode->key[i] = cursor->key[j];
			for (int i = leftNode->size + 1, j = 0; j < cursor->size; j++)
			{
				leftNode->ptr[i] = cursor->ptr[j];
				cursor->ptr[j] = NULL;
			}
			leftNode->size += cursor->size + 1;
			cursor->size = 0;
			// delete cursor
			removeParent(parent->key[leftSibling], parent, cursor);
			numOfNode -= 1;
			numOfNodeDel += 1;
		}
		else if (rightSibling <= parent->size)
		{
			// cursor + parent key + rightnode
			Node *rightNode = parent->ptr[rightSibling];
			cursor->key[cursor->size] = parent->key[rightSibling - 1];
			for (int i = cursor->size + 1, j = 0; j < rightNode->size; j++)
				cursor->key[i] = rightNode->key[j];
			for (int i = cursor->size + 1, j = 0; j < rightNode->size; j++)
			{
				cursor->ptr[i] = rightNode->ptr[j];
				rightNode->ptr[j] = NULL;
			}
			cursor->size += rightNode->size + 1;
			rightNode->size = 0;
			// delete cursor
			removeParent(parent->key[rightSibling - 1], parent, rightNode);
			numOfNode -= 1;
			numOfNodeDel += 1;
		}
	}

	void deleteLLNode(LLNode **pList, diskStorage *diskStorage)
	{
		LLNode *temp = NULL;
		if (pList == NULL)
			return;

		while (*pList != NULL)
		{
			temp = *pList;
			diskStorage->deleteRecord(temp->location);
			*pList = (*pList)->next;
			free(temp);
		}
	}

	void shiftPointer(Node *cursor)
	{

		cursor->size++;
		cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
		cursor->ptr[cursor->size - 1] = NULL;
	}

	void shiftPointerSibling(Node *siblingNode, Node *nextNode)
	{
		siblingNode->size--;
		siblingNode->ptr[siblingNode->size] = nextNode;
		siblingNode->ptr[siblingNode->size + 1] = NULL;
	}

	void shiftArr(Node *node, bool isLeftSibling)
	{
		if (isLeftSibling)
		{
			for (int i = node->size; i > 0; i--)
			{
				node->key[i] = node->key[i - 1];
				node->llPtr[i] = node->llPtr[i - 1];
			}
		}
		else
		{
			for (int i = 0; i < node->size; i++)
			{
				node->key[i] = node->key[i + 1];
				node->llPtr[i] = node->llPtr[i + 1];
			}
		}
	}

	void borrowSiblingKey(Node *cursor, Node *siblingNode, bool isLeftSibling)
	{
		if (isLeftSibling)
		{
			shiftArr(cursor, isLeftSibling);

			// transfer
			cursor->key[0] = siblingNode->key[siblingNode->size - 1];
			cursor->llPtr[0] = siblingNode->llPtr[siblingNode->size - 1];
		}
		else
		{
			cursor->key[cursor->size - 1] = siblingNode->key[0];
			cursor->llPtr[cursor->size - 1] = siblingNode->llPtr[0];

			shiftArr(siblingNode, isLeftSibling);
		}
	}

	bool isLeftSiblingNode(Node **siblingNode, Node *parent, int leftChildIndex, int rightChildIndex)
	{
		bool isLeftSibling = true;
		// Determine to borrow from left or right sibling node
		if (leftChildIndex >= 0)
		{
			(*siblingNode) = parent->ptr[leftChildIndex];

			if ((*siblingNode)->size >= ceil((MAX + 1) / 2))
			{
				isLeftSibling = true;
			}
		}
		else if (rightChildIndex <= parent->size)
		{
			(*siblingNode) = parent->ptr[rightChildIndex];

			if ((*siblingNode)->size >= ceil((MAX + 1) / 2))
			{
				isLeftSibling = false;
			}
		}
		return isLeftSibling;
	}

	bool canBorrowSiblingKey(Node *parent, int leftChildIndex, int rightChildIndex)
	{
		bool canBorrow = false;
		Node *siblingNode = NULL;
		// Determine to borrow from left or right sibling node
		if (leftChildIndex >= 0)
		{
			siblingNode = parent->ptr[leftChildIndex];

			if (siblingNode->size - 1 >= ceil((MAX + 1) / 2))
			{
				canBorrow = true;
			}
		}
		else if (rightChildIndex <= parent->size)
		{
			siblingNode = parent->ptr[rightChildIndex];

			if (siblingNode->size - 1 >= ceil((MAX + 1) / 2))
			{
				canBorrow = true;
			}
		}
		return canBorrow;
	}

	void deleteNode(Node *node)
	{
		delete[] node->key;
		delete[] node->ptr;
		delete[] node->location;
		delete node;
	}
#pragma endregion

#pragma region Common functions
	Node *findParent(Node *cursor, Node *child)
	{
		// finds parent using depth first traversal and ignores leaf nodes as they cannot be parents
		// also ignores second last level because we will never find parent of a leaf node during insertion using this function
		Node *parent = NULL;
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
				if (parent != NULL)
					return parent;
			}
		}
		return parent;
	}
#pragma endregion
};
