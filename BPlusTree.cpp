#include <iostream>
#include <algorithm>
#include <string>
#include <climits>
#include <vector>
#include "bufferPool.h"
#include <math.h>

using namespace std;
const int MAX = 3; // size of each node

class Node
{
	bool IS_LEAF;
	int *key, size;
	Location *location;
	LLNode **llPtr;
	Node **ptr;
	friend class BPlusTree;

public:
	Node()
	{
		// dynamic memory allocation
		key = new int[MAX];
		location = new Location[MAX];
		ptr = new Node *[MAX + 1];
		llPtr = new LLNode *[MAX];
	}
};

class BPlusTree
{
	Node *root;
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

				// step3.2.1: create a MAX + 1 sorted array for overflow scenario
				int overflowArrKey[MAX + 1];
				LLNode *overflowArrLLNode[MAX + 1];
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
					Node *parentRoot = new Node;
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
#pragma endregion

#pragma region Search main function
	void retrievedetails(int lowlimit, int highlimit, bufferPool *bufferPool)
	{
		bool retrievenode = true;
		int numOfIndexAccess = 1, numOfBlkAccess = 1, numOfMatch = 0;
		float totalRating = 0, avgRating = 0;
		vector<int> tempIndex;
		vector<string> tempData;
		vector<uchar *> tempAddress;
		vector<vector<int>> indexNode;
		vector<vector<string>> dataBlock;

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
						if (indexNode.size() < 5 and retrievenode == true)
							tempIndex.push_back(cursor->key[j]);
					}

					if (lowlimit < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						numOfIndexAccess += 1;
						break;
					}
					if (i == cursor->size - 1)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						break;
					}
					retrievenode = false;
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
				retrievenode = true;
			}
			while (retrievenode)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					if (indexNode.size() < 5)
						tempIndex.push_back(cursor->key[i]);

					if (cursor->key[i] >= lowlimit and cursor->key[i] <= highlimit)
					{
						tempAddress = getAllLLNode(cursor->llPtr[i], tempAddress);
						numOfMatch += cursor->llPtr[i]->size;
					}
					else if (cursor->key[i] > highlimit)
						retrievenode = false;

					if (i == cursor->size - 1 and retrievenode == true)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						numOfBlkAccess += 1;
						break;
					}
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
			}

			numOfBlkAccess = tempAddress.size();
			for (uint i = 0; i < numOfBlkAccess; i++)
			{
				for (uint j = 20; j <= bufferPool->getBlockSize(); j += 20)
				{
					void *recordAddress = (uchar *)tempAddress[i] + j;
					if (dataBlock.size() < 5)
						tempData.push_back((*(Record *)recordAddress).tconst);
					if ((*(Record *)recordAddress).numVotes >= lowlimit and (*(Record *)recordAddress).numVotes <= highlimit)
						totalRating += (*(Record *)recordAddress).averageRating;
				}
				if (dataBlock.size() < 5)
					dataBlock.push_back(tempData);
				tempData.clear();
			}

			cout << "Content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indexNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indexNode[i].size(); j++)
					cout << indexNode[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "Number of Index Nodes accessed = " << numOfIndexAccess << endl;

			cout << endl
				 << "Content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlock.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlock[i].size(); j++)
					cout << dataBlock[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "Number of Data Blocks accessed = " << numOfBlkAccess << endl
				 << endl;

			cout << "Total number of matches = " << numOfMatch << endl;
			if (numOfMatch > 0)
				cout << "The rating average of 'averageRating' = " << totalRating / numOfMatch << endl;
			else
				cout << "No records were found due to 0 match results." << endl;
		}
	}

#pragma endregion

#pragma region Delete main function
	void RmoveEvent(int x, bufferPool* bufferPool)

	{
		//delete logic
		if (root == NULL)

		{
			cout << "Tree is Empty!!\n";
		}

		else

		{
			Node* cursor = root;
			Node* parent = NULL;
			int leftChildren, rightChildern;
			bool isUnderFlow = false;

			//cursor will traverse to the leaf node possibly consisting the key
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
				if (!isFound) {
					leftChildren = cursor->size - 1;
					rightChildern = cursor->size - 1 + 2;

					cursor = cursor->ptr[cursor->size - 1 + 1];
				}

			}

			//search for the key if it exists and remove the linked list
			bool found = false;
			int pos;

			for (pos = 0; pos < cursor->size; pos++)
			{
				if (cursor->key[pos] == x)
				{
					found = true;
					freeList(&cursor->llPtr[pos], bufferPool);
					break;
				}
			}

			if (!found)//if key does not exist in that leaf node
			{
				cout << "Key is node found in B+ tree." << endl;
				return;
			}

			if (cursor == root)//if it is root node, then make all pointers NULL
			{
				heightOfTree -= 1;
				for (int i = 0; i < MAX + 1; i++)
				{
					cursor->ptr[i] = NULL;
				}

				if (cursor->size == 0)//if all keys are deleted
				{
					deleteNode(cursor);

					root = NULL;
				}
				return;

			}

			//deleting the key
			for (int i = pos; i < cursor->size; i++)
			{
				cursor->key[i] = cursor->key[i + 1];
				cursor->llPtr[i] = cursor->llPtr[i + 1];
			}

			cursor->size--;

			//underflow
			if (cursor->size < floor((MAX + 1) / 2))
			{
				isUnderFlow = true;
			}

			

			cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
			cursor->ptr[cursor->size + 1] = NULL;

			if (isUnderFlow)
			{
#pragma region MY CODE 
				// if the node keys is less than minimum, check if sibling able to borrow key
				bool isCanBorrowSiblingKey = canBorrowSiblingKey(parent, leftChildren, rightChildern);

				if (isCanBorrowSiblingKey) {
					bool isLeftSibling = true;
					Node* siblingNode = NULL;
					isLeftSibling = isLeftSiblingNode(&siblingNode, parent, leftChildren, rightChildern);

					if (isLeftSibling) {
						borrowSiblingKey(cursor, siblingNode, true);

						shiftPointer(cursor);

						shiftPointerSibling(siblingNode, cursor);

						parent->key[leftChildren] = cursor->key[0];
						parent->llPtr[leftChildren] = cursor->llPtr[0];
					}
					else {
						borrowSiblingKey(cursor, siblingNode, false);

						shiftPointer(cursor);

						shiftPointerSibling(siblingNode, siblingNode->ptr[siblingNode->size + 1]);

						parent->key[rightChildern - 1] = siblingNode->key[0];
						parent->llPtr[rightChildern - 1] = siblingNode->llPtr[0];
					}
				}
				else {
					// else we will need to merge with sibling node and delete the node

					if (leftChildren >= 0) {
						Node* leftNode = parent->ptr[leftChildren];

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

						removeInternal(parent->key[leftChildren], parent, cursor);

						deleteNode(cursor);
					}
					else if (rightChildern <= parent->size) {
						Node* rightNode = parent->ptr[rightChildern];

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

						removeInternal(parent->key[rightChildern - 1], parent, rightNode);

						deleteNode(rightNode);
					}

					// update the total nodes
					numOfNode -= 1;
					numOfNodeDel += 1;
				}
#pragma endregion
//
//#pragma region Borrow key from sibling OLD
//				//underflow condition
//				//first we try to transfer a key from sibling node
//				//check if left sibling exists
//				if (leftChildren >= 0)
//				{
//					Node* leftNode = parent->ptr[leftChildren];
//					//check if it is possible to transfer
//
//					if (leftNode->size >= (MAX + 1) / 2 + 1)
//					{
//						//make space for transfer
//						for (int i = cursor->size; i > 0; i--)
//						{
//							cursor->key[i] = cursor->key[i - 1];
//							cursor->llPtr[i] = cursor->llPtr[i - 1];
//
//						}
//						//transfer
//						cursor->key[0] = leftNode->key[leftNode->size - 1];
//						cursor->llPtr[0] = leftNode->llPtr[leftNode->size - 1];
//
//						//shift pointer to next leaf
//						cursor->size++;
//						cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
//						cursor->ptr[cursor->size - 1] = NULL;
//
//						shiftPointer(cursor);
//
//						//shift pointer of leftsibling
//						leftNode->size--;
//						leftNode->ptr[leftNode->size] = cursor;
//						leftNode->ptr[leftNode->size + 1] = NULL;
//
//						shiftPointerSibling(leftNode, cursor);
//						//update parent
//						parent->key[leftChildren] = cursor->key[0];
//						parent->llPtr[leftChildren] = cursor->llPtr[0];
//
//						return;
//
//					}
//
//				}
//
//				if (rightChildern <= parent->size)//check if right sibling exist
//				{
//					Node* rightNode = parent->ptr[rightChildern];
//
//					//check if it is possible to transfer
//					if (rightNode->size >= (MAX + 1) / 2 + 1)
//					{
//						//transfer
//						cursor->key[cursor->size - 1] = rightNode->key[0];
//						cursor->llPtr[cursor->size - 1] = rightNode->llPtr[0];
//
//						//shift pointer to next leaf
//						cursor->size++;
//						cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
//						cursor->ptr[cursor->size - 1] = NULL;
//
//						//shift pointer of rightsibling
//						rightNode->size--;
//						rightNode->ptr[rightNode->size] = rightNode->ptr[rightNode->size + 1];
//						rightNode->ptr[rightNode->size + 1] = NULL;
//
//						//shift conent of right sibling
//						for (int i = 0; i < rightNode->size; i++)
//						{
//							rightNode->key[i] = rightNode->key[i + 1];
//							rightNode->llPtr[i] = rightNode->llPtr[i + 1];
//						}
//
//						//update parent
//						parent->key[rightChildern - 1] = rightNode->key[0];
//						parent->llPtr[rightChildern - 1] = rightNode->llPtr[0];
//
//						return;
//
//					}
//
//				}
//#pragma endregion
//
//
//#pragma region Merge and delete the node OLD
//				//merge and delete the node
//
//				if (leftChildren >= 0)//if left sibling exist
//				{
//					Node* leftNode = parent->ptr[leftChildren];
//
//					// transfer all keys to leftsibling and then transfer pointer to next leaf node
//					for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++)
//					{
//						leftNode->key[i] = cursor->key[j];
//						leftNode->llPtr[i] = cursor->llPtr[j];
//					}
//
//					//leftNode->ptr[leftNode->size] = NULL;
//					leftNode->size += cursor->size;
//					leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];
//					removeInternal(parent->key[leftChildren], parent, cursor);// delete parent node key
//
//					//delete[] cursor->key;
//					//delete[] cursor->ptr;
//					//delete[] cursor->location;
//					//delete cursor;
//					deleteNode(cursor);
//
//					numOfNode -= 1;
//					numOfNodeDel += 1;
//				}
//
//				else if (rightChildern <= parent->size)//if right sibling exist
//
//				{
//
//					Node* rightNode = parent->ptr[rightChildern];
//
//					// transfer all keys to cursor and then transfer pointer to next leaf node
//					for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++)
//					{
//						cursor->key[i] = rightNode->key[j];
//						cursor->llPtr[i] = rightNode->llPtr[j];
//					}
//
//					//cursor->ptr[cursor->size] = NULL;
//					cursor->size += rightNode->size;
//					cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];
//					removeInternal(parent->key[rightChildern - 1], parent, rightNode);// delete parent node key
//
//					/*delete[] rightNode->key;
//					delete[] rightNode->ptr;
//					delete[] rightNode->location;
//					delete rightNode;*/
//					deleteNode(rightNode);
//
//					numOfNode -= 1;
//					numOfNodeDel += 1;
//				}
//#pragma endregion

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
			// cursor not full, find position to insert new key
			int i = 0;
			while (keyToInsert > cursor->key[i] && i < cursor->size)
				i++;
			// make space for new key
			for (int j = cursor->size; j > i; j--)
				cursor->key[j] = cursor->key[j - 1];
			// make space for new pointer
			for (int j = cursor->size + 1; j > i + 1; j--)
				cursor->ptr[j] = cursor->ptr[j - 1];
			cursor->key[i] = keyToInsert;
			cursor->size++;
			cursor->ptr[i + 1] = child;
		}
		else
		{
			// overflow in internal, create new internal node
			Node *newInternalNode = new Node;
			// create virtual internal node to store the key
			int tempKey[MAX + 1];
			Node *tempPtr[MAX + 2];
			// increase number of node by 1
			numOfNode += 1;
			for (int i = 0; i < MAX; i++)
				tempKey[i] = cursor->key[i];
			for (int i = 0; i < MAX + 1; i++)
				tempPtr[i] = cursor->ptr[i];
			int i = 0, j;
			while (keyToInsert > tempKey[i] && i < MAX)
				i++;
			// make space for new key
			for (int j = MAX + 1; j > i; j--)
				tempKey[j] = tempKey[j - 1];

			tempKey[i] = keyToInsert;

			for (int j = MAX + 2; j > i + 1; j--)
				tempPtr[j] = tempPtr[j - 1];

			tempPtr[i + 1] = child;
			newInternalNode->IS_LEAF = false;
			// split cursor into two different nodes
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
				// if cursor is a root node, create new parent root
				Node *parentRoot = new Node;
				CreateParentNode(parentRoot, cursor, newInternalNode);
			}
			else
			{
				// find depth first search to find parent of cursor recursively
				InsertParent(findParent(root, cursor), newInternalNode);
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
		root = new Node;
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
		Node *leafNode = new Node();
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
	vector<uchar *> getAllLLNode(LLNode *list, vector<uchar *> &tempAddress)
	{
		LLNode *curr = list;
		if (list == NULL)
		{
			cout << ("List is Empty") << endl;
			return tempAddress;
		}
		while (curr != NULL)
		{
			if (find(tempAddress.begin(), tempAddress.end(), curr->location.blockLocation) == tempAddress.end())
				tempAddress.push_back(curr->location.blockLocation);
			curr = curr->next;
		}
		return tempAddress;
	}
	Record getRecords(Location location)
	{
		void *mainMemoryAddress = (uchar *)location.blockLocation + location.offset;
		return (*(Record *)mainMemoryAddress);
	}
#pragma endregion

#pragma region Delete sub functions
	void removeInternal(int x, Node *cursor, Node *child)
	{
		// deleting the key x first
		// checking if key from root is to be deleted
		if (cursor == root)
		{
			if (cursor->size == 1) // if only one key is left, change root
			{
				heightOfTree -= 1;
				numOfNode -= 1;
				numOfNodeDel += 1;
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
		}
		int pos;
		for (pos = 0; pos < cursor->size; pos++)
		{
			if (cursor->key[pos] == x)
				break;
		}
		for (int i = pos; i < cursor->size; i++)
			cursor->key[i] = cursor->key[i + 1];

		// now deleting the pointer child
		for (pos = 0; pos < cursor->size + 1; pos++)
		{
			if (cursor->ptr[pos] == child)
				break;
		}
		for (int i = pos; i < cursor->size + 1; i++)
			cursor->ptr[i] = cursor->ptr[i + 1];
		cursor->size--;
		if (cursor->size >= (MAX + 1) / 2 - 1) // no underflow
			return;
		// underflow, try to transfer first
		if (cursor == root)
			return;
		Node *parent = findParent(root, cursor);
		int leftSibling, rightSibling;
		// finding left n right sibling of cursor
		for (pos = 0; pos < parent->size + 1; pos++)
		{
			if (parent->ptr[pos] == cursor)
			{
				leftSibling = pos - 1;
				rightSibling = pos + 1;
				break;
			}
		}
		// try to transfer
		if (leftSibling >= 0) // if left sibling exists
		{
			Node *leftNode = parent->ptr[leftSibling];
			// check if it is possible to transfer
			if (leftNode->size >= (MAX + 1) / 2)
			{
				// make space for key transfer
				for (int i = cursor->size; i > 0; i--)
					cursor->key[i] = cursor->key[i - 1];
				// transfer key from left sibling through parent
				cursor->key[0] = parent->key[leftSibling];
				parent->key[leftSibling] = leftNode->key[leftNode->size - 1];
				// transfer last pointer from leftnode to cursor
				// make space for transfer of ptr
				for (int i = cursor->size + 1; i > 0; i--)
					cursor->ptr[i] = cursor->ptr[i - 1];
				// transfer ptr
				cursor->ptr[0] = leftNode->ptr[leftNode->size];
				cursor->size++;
				leftNode->size--;
				return;
			}
		}
		if (rightSibling <= parent->size) // check if right sibling exist
		{
			Node *rightNode = parent->ptr[rightSibling];
			// check if it is possible to transfer
			if (rightNode->size >= (MAX + 1) / 2)
			{
				// transfer key from right sibling through parent
				cursor->key[cursor->size] = parent->key[pos];
				parent->key[pos] = rightNode->key[0];
				for (int i = 0; i < rightNode->size - 1; i++)
					rightNode->key[i] = rightNode->key[i + 1];
				// transfer first pointer from rightnode to cursor
				// transfer ptr
				cursor->ptr[cursor->size + 1] = rightNode->ptr[0];
				for (int i = 0; i < rightNode->size; ++i)
					rightNode->ptr[i] = rightNode->ptr[i + 1];
				cursor->size++;
				rightNode->size--;
				return;
			}
		}
		// transfer wasnt posssible hence do merging
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
			removeInternal(parent->key[leftSibling], parent, cursor);
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
			removeInternal(parent->key[rightSibling - 1], parent, rightNode);
			numOfNode -= 1;
			numOfNodeDel += 1;
		}
	}

	void freeList(LLNode **pList, bufferPool *bufferPool)
	{
		LLNode *temp = NULL;
		if (pList == NULL)
			return;

		while (*pList != NULL)
		{
			temp = *pList;
			bufferPool->deleteRecord(temp->location);
			*pList = (*pList)->next;
			free(temp);
		}
	}

	void shiftPointer(Node* cursor) {

		cursor->size++;
		cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
		cursor->ptr[cursor->size - 1] = NULL;
	}

	void shiftPointerSibling(Node* siblingNode, Node* nextNode) {
		siblingNode->size--;
		siblingNode->ptr[siblingNode->size] = nextNode;
		siblingNode->ptr[siblingNode->size + 1] = NULL;
	}

	void shiftArr(Node* node, bool isLeftSibling) {
		if (isLeftSibling) {
			for (int i = node->size; i > 0; i--)
			{
				node->key[i] = node->key[i - 1];
				node->llPtr[i] = node->llPtr[i - 1];

			}
		}
		else {
			for (int i = 0; i < node->size; i++)
			{
				node->key[i] = node->key[i + 1];
				node->llPtr[i] = node->llPtr[i + 1];
			}
		}
	}

	void borrowSiblingKey(Node* cursor, Node* siblingNode, bool isLeftSibling) {
		if (isLeftSibling) {
			shiftArr(cursor, isLeftSibling);
			
			//transfer
			cursor->key[0] = siblingNode->key[siblingNode->size - 1];
			cursor->llPtr[0] = siblingNode->llPtr[siblingNode->size - 1];
		}
		else {
			cursor->key[cursor->size - 1] = siblingNode->key[0];
			cursor->llPtr[cursor->size - 1] = siblingNode->llPtr[0];

			shiftArr(siblingNode, isLeftSibling);
		}
	}

	bool isLeftSiblingNode(Node** siblingNode, Node* parent, int leftChildIndex, int rightChildIndex) {
		bool isLeftSibling = true;
		//Determine to borrow from left or right sibling node
		if (leftChildIndex >= 0) {
			(*siblingNode) = parent->ptr[leftChildIndex];

			if ((*siblingNode)->size >= ceil((MAX + 1) / 2)) {
				isLeftSibling = true;
			}
		}
		else if (rightChildIndex <= parent->size) {
			(*siblingNode) = parent->ptr[rightChildIndex];

			if ((*siblingNode)->size >= ceil((MAX + 1) / 2)) {
				isLeftSibling = false;
			}
		}
		return isLeftSibling;
	}

	bool canBorrowSiblingKey(Node* parent, int leftChildIndex, int rightChildIndex) {
		bool canBorrow = false;
		Node* siblingNode = NULL;
		//Determine to borrow from left or right sibling node
		if (leftChildIndex >= 0) {
			siblingNode = parent->ptr[leftChildIndex];

			if (siblingNode->size - 1 >= ceil((MAX + 1) / 2)) {
				canBorrow = true;
			}
		}
		else if (rightChildIndex <= parent->size) {
			siblingNode = parent->ptr[rightChildIndex];

			if (siblingNode->size - 1 >= ceil((MAX + 1) / 2)) {
				canBorrow = true;
			}
		}
		return canBorrow;
	}

	void deleteNode(Node* node) {
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
