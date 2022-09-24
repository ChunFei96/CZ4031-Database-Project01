#include <iostream>
#include <algorithm>
#include <string>
#include <climits>
#include <vector>
#include "bufferPool.h"

using namespace std;
const int MAX = 5; // size of each node

class Node
{
	bool IS_LEAF;
	int *key, size;
	Location *location;
	LLNode **llPtr;
	Node **ptr;
	friend class BPTree;

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

class BPTree
{
	Node *root;
	int numOfNode = 0, treeLvl = 0, numOfNodeDel = 0;

public:
	BPTree()
	{
		root = NULL;
	}

	void insertInternal(int x, Node *cursor, Node *child)
	{
		if (cursor->size < MAX)
		{
			// cursor not full, find position to insert new key
			int i = 0;
			while (x > cursor->key[i] && i < cursor->size)
				i++;
			// make space for new key
			for (int j = cursor->size; j > i; j--)
				cursor->key[j] = cursor->key[j - 1];
			// make space for new pointer
			for (int j = cursor->size + 1; j > i + 1; j--)
				cursor->ptr[j] = cursor->ptr[j - 1];
			cursor->key[i] = x;
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
			while (x > tempKey[i] && i < MAX)
				i++;
			// make space for new key
			for (int j = MAX + 1; j > i; j--)
				tempKey[j] = tempKey[j - 1];

			tempKey[i] = x;

			for (int j = MAX + 2; j > i + 1; j--)
				tempPtr[j] = tempPtr[j - 1];

			tempPtr[i + 1] = child;
			newInternalNode->IS_LEAF = false;
			// split cursor into two different nodes
			cursor->size = (MAX + 1) / 2;
			newInternalNode->size = MAX - ((MAX + 1) / 2);
			// give elements and pointers to the new node
			for (i = 0, j = cursor->size + 1; i < newInternalNode->size; i++, j++)
				newInternalNode->key[i] = tempKey[j];
			for (i = 0, j = cursor->size + 1; i < newInternalNode->size + 1; i++, j++)
				newInternalNode->ptr[i] = tempPtr[j];

			if (cursor == root)
			{
				// if cursor is a root node, create new root
				Node *newRoot = new Node;
				newRoot->key[0] = cursor->key[cursor->size];
				newRoot->ptr[0] = cursor;
				newRoot->ptr[1] = newInternalNode;
				newRoot->IS_LEAF = false;
				newRoot->size = 1;
				root = newRoot;
				// increase number of node and tree level by 1
				numOfNode += 1;
				treeLvl += 1;
			}
			else
			{
				// find depth first search to find parent of cursor recursively
				insertInternal(cursor->key[cursor->size], findParent(root, cursor), newInternalNode);
			}
		}
	}

	void removeInternal(int x, Node *cursor, Node *child)
	{
		// deleting the key x first
		// checking if key from root is to be deleted
		if (cursor == root)
		{
			if (cursor->size == 1) // if only one key is left, change root
			{
				treeLvl -= 1;
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

	/*void search(int lowerBound, int upperBound)
	{
		bool search = true;
		int numOfIndexAccess = 1, numOfBlkAccess = 1, numOfMatch = 0;
		float totalRating = 0, avgRating = 0;
		vector <int> tempIndex;
		vector <string> tempData;
		vector <Record> tempRecord;
		vector <vector <int>> indexNode;
		vector <vector <string>> dataBlock;
		//search logic
		if (root == NULL)
		{
			//empty
			cout << "Tree is empty." << endl;
		}
		else
		{
			Node* cursor = root;
			//in the following while loop, cursor will travel to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					for (int j = 0; j < cursor->size; j++)
					{
						if (indexNode.size() < 5 and search == true)
							tempIndex.push_back(cursor->key[j]);
					}

					if (lowerBound < cursor->key[i])
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
					search = false;
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
				search = true;
			}
			//Record record;
			//in the following for loop, we search for the key if it exists
			while (search) {
				for (int i = 0; i < cursor->size; i++)
				{
					//cout << cursor->llPtr[i]->size << endl;
					//record = getRecords(cursor->address[i]);
					tempRecord = getAllLLNode(cursor->llPtr[i]);
					for (int j = 0; j < tempRecord.size(); j++)
						tempData.push_back(tempRecord[j].tconst);


					if (indexNode.size() < 5)
						tempIndex.push_back(cursor->key[i]);
					if (dataBlock.size() < 5)
						dataBlock.push_back(tempData);

					if (cursor->key[i] >= lowerBound and cursor->key[i] <= upperBound)
					{
						for (int k = 0; k < tempRecord.size(); k++)
							totalRating += tempRecord[k].avgRating;
						numOfMatch += cursor->llPtr[i]->size;
					}
					else if (cursor->key[i] > upperBound)
					{
						search = false;
					}

					if (i == cursor->size - 1 and search == true)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						numOfBlkAccess += 1;
						break;
					}
					tempRecord.clear();
					tempData.clear();
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
			}
			cout << "The content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indexNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indexNode[i].size(); j++)
					cout << indexNode[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Index Nodes accessed = " << numOfIndexAccess << endl;

			cout << endl << "The content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlock.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlock[i].size(); j++)
					cout << dataBlock[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Data Blocks accessed = " << numOfBlkAccess << endl << endl;

			cout << "The total number of matches = " << numOfMatch << endl;
			if (numOfMatch > 0)
				cout << "The average of 'averageRating' = " << totalRating / numOfMatch << endl;
			else
				cout << "No records found." << endl;
		}
	}*/

	void search(int lowerBound, int upperBound, bufferPool *bufferPool)
	{
		bool search = true;
		int numOfIndexAccess = 1, numOfBlkAccess = 1, numOfMatch = 0;
		float totalRating = 0, avgRating = 0;
		vector<int> tempIndex;
		vector<string> tempData;
		vector<uchar *> tempAddress;
		vector<vector<int>> indexNode;
		vector<vector<string>> dataBlock;
		// search logic
		if (root == NULL)
		{
			// empty
			cout << "Tree is empty." << endl;
		}
		else
		{
			Node *cursor = root;
			// in the following while loop, cursor will travel to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					for (int j = 0; j < cursor->size; j++)
					{
						if (indexNode.size() < 5 and search == true)
							tempIndex.push_back(cursor->key[j]);
					}

					if (lowerBound < cursor->key[i])
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
					search = false;
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
				search = true;
			}
			// Record record;
			// in the following for loop, we search for the key if it exists
			while (search)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					if (indexNode.size() < 5)
						tempIndex.push_back(cursor->key[i]);

					if (cursor->key[i] >= lowerBound and cursor->key[i] <= upperBound)
					{
						tempAddress = getAllLLNode(cursor->llPtr[i], tempAddress);
						numOfMatch += cursor->llPtr[i]->size;
					}
					else if (cursor->key[i] > upperBound)
						search = false;

					if (i == cursor->size - 1 and search == true)
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
				for (uint j = 20; j <= bufferPool->getBlkSize(); j += 20)
				{
					void *recordAddress = (uchar *)tempAddress[i] + j;
					if (dataBlock.size() < 5)
						tempData.push_back((*(Record *)recordAddress).tconst);
					if ((*(Record *)recordAddress).numVotes >= lowerBound and (*(Record *)recordAddress).numVotes <= upperBound)
						totalRating += (*(Record *)recordAddress).avgRating;
				}
				if (dataBlock.size() < 5)
					dataBlock.push_back(tempData);
				tempData.clear();
			}

			cout << "The content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indexNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indexNode[i].size(); j++)
					cout << indexNode[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Index Nodes accessed = " << numOfIndexAccess << endl;

			cout << endl
				 << "The content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlock.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlock[i].size(); j++)
					cout << dataBlock[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Data Blocks accessed = " << numOfBlkAccess << endl
				 << endl;

			cout << "The total number of matches = " << numOfMatch << endl;
			if (numOfMatch > 0)
				cout << "The average of 'averageRating' = " << totalRating / numOfMatch << endl;
			else
				cout << "No records found." << endl;
		}
	}

	/*void search(int lowerBound, int upperBound)
	{
		bool search = true;
		int numOfIndexAccess = 1, numOfBlkAccess = 1, numOfMatch = 0;
		float totalRating = 0, avgRating = 0;
		vector <int> tempIndex;
		vector <string> tempData;
		vector <vector <int>> indexNode;
		vector <vector <string>> dataBlock;
		//search logic
		if (root == NULL)
		{
			//empty
			cout << "Tree is empty." << endl;
		}
		else
		{
			Node* cursor = root;
			//in the following while loop, cursor will travel to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					for (int j = 0; j < cursor->size; j++)
					{
						if (indexNode.size() < 5 and search == true)
							tempIndex.push_back(getRecords(cursor->address[j]).numVotes);
					}

					if (lowerBound <= cursor->key[i])
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
					search = false;
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				tempIndex.clear();
				search = true;
			}
			Record record;
			//in the following for loop, we search for the key if it exists
			while (search) {
				for (int i = 0; i < cursor->size; i++)
				{
					record = getRecords(cursor->address[i]);
					if (indexNode.size() < 5)
						tempIndex.push_back(record.numVotes);
					if (dataBlock.size() < 5)
						tempData.push_back(record.tconst);
					if (cursor->key[i] >= lowerBound and cursor->key[i] <= upperBound)
					{
						numOfMatch += 1;
						totalRating += record.avgRating;
					}
					else if (cursor->key[i] > upperBound)
					{
						search = false;
					}

					if (i == cursor->size - 1 and search == true)
					{
						cursor = cursor->ptr[i + 1];
						numOfIndexAccess += 1;
						numOfBlkAccess += 1;
						break;
					}
				}
				if (indexNode.size() < 5)
					indexNode.push_back(tempIndex);
				if (dataBlock.size() < 5)
					dataBlock.push_back(tempData);
				tempIndex.clear();
				tempData.clear();
			}
			cout << "The content of the first 5 Index Nodes =" << endl;
			for (uint i = 0; i < indexNode.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < indexNode[i].size(); j++)
					cout << indexNode[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Index Nodes accessed = " << numOfIndexAccess << endl;

			cout << endl << "The content of the first 5 Data Blocks = " << endl;
			for (uint i = 0; i < dataBlock.size(); i++)
			{
				cout << "| ";
				for (uint j = 0; j < dataBlock[i].size(); j++)
					cout << dataBlock[i][j] << " ";
				cout << "| " << endl;
			}
			cout << "The number of Data Blocks accessed = " << numOfBlkAccess << endl << endl;

			cout << "The total number of matches = " << numOfMatch << endl;
			if (numOfMatch > 0)
				cout << "The average of 'averageRating' = " << totalRating / numOfMatch << endl;
			else
				cout << "No records found." << endl;
		}
	}*/

	vector<uchar *> getAllLLNode(LLNode *list, vector<uchar *> &tempAddress)
	{
		/* Declare variable to iterate through the list*/
		LLNode *curr = list;

		/* Check if the list is empty */
		if (list == NULL)
		{
			cout << ("List is Empty") << endl;
			return tempAddress;
		}

		/* Loop while current is not NULL */
		while (curr != NULL)
		{
			if (find(tempAddress.begin(), tempAddress.end(), curr->location.blockLocation) == tempAddress.end()) // address does not exist in vector
				tempAddress.push_back(curr->location.blockLocation);
			curr = curr->next;
		}
		return tempAddress;
	}

	// vector<Record> getAllLLNode(LLNode* list)
	//{
	//	/* Declare variable to iterate through the list*/
	//	LLNode* curr = list;
	//	vector<Record> tempVector;

	//	/* Check if the list is empty */
	//	if (list == NULL)
	//	{
	//		cout << ("List is Empty") << endl;
	//		return tempVector;
	//	}

	//	Record record;
	//	/* Loop while current is not NULL */
	//	while (curr != NULL)
	//	{
	//		record = getRecords(curr->address);
	//		tempVector.push_back(record);
	//		curr = curr->next;
	//	}
	//	return tempVector;
	//}

	// Record getRecords(uchar* blockAddress, int offset) {
	//	void* mainMemoryAddress = (uchar*)blockAddress + offset;
	//	return (*(Record*)mainMemoryAddress);

	Record getRecords(Location location)
	{
		void *mainMemoryAddress = (uchar *)location.blockLocation + location.offset;
		return (*(Record *)mainMemoryAddress);
	}
	void insert(Location location, int x)
	{
		// insert logic
		if (root == NULL)
		{
			root = new Node;
			LLNode *listHead = NULL;
			root->key[0] = x;
			root->IS_LEAF = true;
			root->size = 1;
			AddToEnd(&listHead, location);
			root->llPtr[0] = listHead;
			// increase number of node and tree level by 1
			numOfNode += 1;
			treeLvl += 1;
		}
		else
		{
			Node *cursor = root;
			Node *parent = NULL;
			LLNode *listHead = NULL;
			// cursor traverse to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				parent = cursor;
				for (int i = 0; i < cursor->size; i++)
				{
					if (x < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						break;
					}
					if (i == cursor->size - 1)
					{
						cursor = cursor->ptr[i + 1];
						break;
					}
				}
			}
			for (int i = 0; i < cursor->size; i++)
			{
				if (x == cursor->key[i])
				{
					AddToEnd(&cursor->llPtr[i], location);
					return;
				}
			}

			// now cursor is the leaf node in which we'll insert the new key
			if (cursor->size < MAX)
			{
				// if cursor is not full
				// find the correct position for new key
				int i = 0;
				while (x > cursor->key[i] && i < cursor->size)
					i++;
				// make space for new key
				for (int j = cursor->size; j > i; j--)
				{
					cursor->key[j] = cursor->key[j - 1];
					cursor->llPtr[j] = cursor->llPtr[j - 1];
				}
				cursor->key[i] = x;
				AddToEnd(&listHead, location);
				cursor->llPtr[i] = listHead;
				;
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = NULL;
			}
			else
			{
				// overflow condition
				// create new leaf node
				Node *newLeaf = new Node;
				// create a virtual node and insert x into it
				int tempKey[MAX + 1];
				Location tempAddressList[MAX + 1];
				LLNode *tempLLPtrList[MAX + 1];
				numOfNode += 1;

				for (int i = 0; i < MAX; i++)
				{
					tempKey[i] = cursor->key[i];
					tempLLPtrList[i] = cursor->llPtr[i];
				}
				int i = 0, j;
				while (x > tempKey[i] && i < MAX)
					i++;
				// make space for new key
				for (int j = MAX + 1; j > i; j--)
				{
					tempKey[j] = tempKey[j - 1];
					tempLLPtrList[j] = tempLLPtrList[j - 1];
				}
				tempKey[i] = x;
				AddToEnd(&listHead, location);
				tempLLPtrList[i] = listHead;
				newLeaf->IS_LEAF = true;
				// split the cursor into two leaf nodes
				cursor->size = (MAX + 1) / 2;
				newLeaf->size = MAX + 1 - ((MAX + 1) / 2);
				// make cursor point to new leaf node
				cursor->ptr[cursor->size] = newLeaf;
				// make new leaf node point to the next leaf node
				newLeaf->ptr[newLeaf->size] = cursor->ptr[MAX];
				cursor->ptr[MAX] = NULL;
				// now give elements to new leaf nodes
				for (i = 0; i < cursor->size; i++)
				{
					cursor->key[i] = tempKey[i];
					cursor->llPtr[i] = tempLLPtrList[i];
				}
				for (i = 0, j = cursor->size; i < newLeaf->size; i++, j++)
				{
					newLeaf->key[i] = tempKey[j];
					newLeaf->llPtr[i] = tempLLPtrList[j];
				}
				// modify the parent
				if (cursor == root)
				{
					// if cursor is a root node, we create a new root
					Node *newRoot = new Node;
					newRoot->key[0] = newLeaf->key[0];
					newRoot->ptr[0] = cursor;
					newRoot->ptr[1] = newLeaf;
					newRoot->IS_LEAF = false;
					newRoot->size = 1;
					root = newRoot;
					// increase number of node and tree level by 1
					numOfNode += 1;
					treeLvl += 1;
				}
				else
				{
					// insert new key in parent node
					insertInternal(newLeaf->key[0], parent, newLeaf);
				}
			}
		}
	}

	void AddToEnd(struct LLNode **ppList, Location location)
	{
		/* Temp pointer to store new node */
		struct LLNode *newNode = NULL;
		/* Serves as our runner to find the last node */
		struct LLNode *curr = NULL;

		/* Check ppList is valid */
		if (ppList == NULL)
		{
			cout << "PPList is null." << endl;
			/* Invalid, return don't do anything */
			return;
		}

		/* Allocate the Node memory via malloc */
		// newNode = (struct Node*)malloc(sizeof(struct Node));
		newNode = new LLNode;
		/* Failed allocation, can't do anything else */
		if (newNode == NULL)
			return;

		/* Assign the values to the new Node */
		newNode->location = location;
		newNode->next = NULL;

		/* Find the location to add */
		/* Special case : Head is empty! */
		if (*ppList == NULL) /* *ppList is ListHead*/
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

	/* Frees (deletes) all nodes in the list */
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

	void remove(int x, bufferPool *bufferPool)
	{
		// delete logic
		if (root == NULL)
		{
			cout << "Tree empty\n";
		}
		else
		{
			Node *cursor = root;
			Node *parent = NULL;
			int leftSibling, rightSibling;
			// cursor will traverse to the leaf node possibly consisting the key
			while (!cursor->IS_LEAF)
			{
				for (int i = 0; i < cursor->size; i++)
				{
					parent = cursor;
					leftSibling = i - 1;  // leftSibling is the index of left sibling in the parent node
					rightSibling = i + 1; // rightSibling is the index of right sibling in the parent node
					if (x < cursor->key[i])
					{
						cursor = cursor->ptr[i];
						break;
					}
					if (i == cursor->size - 1)
					{
						leftSibling = i;
						rightSibling = i + 2;
						cursor = cursor->ptr[i + 1];
						break;
					}
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
					freeList(&cursor->llPtr[pos], bufferPool);
					break;
				}
			}
			if (!found) // if key does not exist in that leaf node
			{
				cout << "Key is node found in B+ tree." << endl;
				return;
			}
			// deleting the key
			for (int i = pos; i < cursor->size; i++)
			{
				cursor->key[i] = cursor->key[i + 1];
				cursor->llPtr[i] = cursor->llPtr[i + 1];
			}
			cursor->size--;
			if (cursor == root) // if it is root node, then make all pointers NULL
			{
				treeLvl -= 1;
				for (int i = 0; i < MAX + 1; i++)
				{
					cursor->ptr[i] = NULL;
				}
				if (cursor->size == 0) // if all keys are deleted
				{
					delete[] cursor->key;
					delete[] cursor->llPtr;
					delete[] cursor->ptr;
					delete cursor;
					root = NULL;
				}
				return;
			}
			cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
			cursor->ptr[cursor->size + 1] = NULL;
			if (cursor->size >= (MAX + 1) / 2) // no underflow
			{
				return;
			}
			// underflow condition
			// first we try to transfer a key from sibling node
			// check if left sibling exists
			if (leftSibling >= 0)
			{
				Node *leftNode = parent->ptr[leftSibling];
				// check if it is possible to transfer
				if (leftNode->size >= (MAX + 1) / 2 + 1)
				{
					// make space for transfer
					for (int i = cursor->size; i > 0; i--)
					{
						cursor->key[i] = cursor->key[i - 1];
						cursor->llPtr[i] = cursor->llPtr[i - 1];
						// cursor->address[i] = cursor->address[i - 1];
					}
					// shift pointer to next leaf
					cursor->size++;
					cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
					cursor->ptr[cursor->size - 1] = NULL;
					// transfer
					cursor->key[0] = leftNode->key[leftNode->size - 1];
					cursor->llPtr[0] = leftNode->llPtr[leftNode->size - 1];
					// shift pointer of leftsibling
					leftNode->size--;
					leftNode->ptr[leftNode->size] = cursor;
					leftNode->ptr[leftNode->size + 1] = NULL;
					// update parent
					parent->key[leftSibling] = cursor->key[0];
					parent->llPtr[leftSibling] = cursor->llPtr[0];
					return;
				}
			}
			if (rightSibling <= parent->size) // check if right sibling exist
			{
				Node *rightNode = parent->ptr[rightSibling];
				// check if it is possible to transfer
				if (rightNode->size >= (MAX + 1) / 2 + 1)
				{
					// shift pointer to next leaf
					cursor->size++;
					cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
					cursor->ptr[cursor->size - 1] = NULL;
					// transfer
					cursor->key[cursor->size - 1] = rightNode->key[0];
					cursor->llPtr[cursor->size - 1] = rightNode->llPtr[0];
					// shift pointer of rightsibling
					rightNode->size--;
					rightNode->ptr[rightNode->size] = rightNode->ptr[rightNode->size + 1];
					rightNode->ptr[rightNode->size + 1] = NULL;
					// shift conent of right sibling
					for (int i = 0; i < rightNode->size; i++)
					{
						rightNode->key[i] = rightNode->key[i + 1];
						rightNode->llPtr[i] = rightNode->llPtr[i + 1];
					}
					// update parent
					parent->key[rightSibling - 1] = rightNode->key[0];
					parent->llPtr[rightSibling - 1] = rightNode->llPtr[0];
					return;
				}
			}
			// merge and delete the node
			if (leftSibling >= 0) // if left sibling exist
			{
				Node *leftNode = parent->ptr[leftSibling];
				// transfer all keys to leftsibling and then transfer pointer to next leaf node
				for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++)
				{
					leftNode->key[i] = cursor->key[j];
					leftNode->llPtr[i] = cursor->llPtr[j];
				}
				// leftNode->ptr[leftNode->size] = NULL;
				leftNode->size += cursor->size;
				leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];
				removeInternal(parent->key[leftSibling], parent, cursor); // delete parent node key
				delete[] cursor->key;
				delete[] cursor->ptr;
				delete[] cursor->location;
				delete cursor;
				numOfNode -= 1;
				numOfNodeDel += 1;
			}
			else if (rightSibling <= parent->size) // if right sibling exist
			{
				Node *rightNode = parent->ptr[rightSibling];
				// transfer all keys to cursor and then transfer pointer to next leaf node
				for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++)
				{
					cursor->key[i] = rightNode->key[j];
					cursor->llPtr[i] = rightNode->llPtr[j];
				}
				// cursor->ptr[cursor->size] = NULL;
				cursor->size += rightNode->size;
				cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];
				removeInternal(parent->key[rightSibling - 1], parent, rightNode); // delete parent node key
				delete[] rightNode->key;
				delete[] rightNode->ptr;
				delete[] rightNode->location;
				delete rightNode;
				numOfNode -= 1;
				numOfNodeDel += 1;
			}
		}
		return;
	}

	void display(Node *cursor, int level, int child)
	{
		if (cursor != NULL and level >= 0)
		{
			if (child == 0)
				cout << "Content of Root Node = ";
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

		// depth first display
		/*if (cursor != NULL)
		{
			for (int i = 0; i < cursor->size; i++)
			{
				cout << cursor->key[i] << " ";
			}
			cout << "\n";
			if (cursor->IS_LEAF != true)
			{
				for (int i = 0; i < cursor->size + 1; i++)
				{
					display(cursor->ptr[i], level, child);
				}
			}
		}*/
	}

	/*void displayLeaf(Node* cursor)
	{
		while (!cursor->IS_LEAF)
		{
			cursor = cursor->ptr[0];
		}
		while (cursor != NULL)
		{
			for (int i =0; i < cursor->size; i++)
				cout << cursor->key[i] << " ";

			cursor = cursor->ptr[cursor->size];
		}
	}*/

	int getNumOfNode()
	{
		return numOfNode;
	}

	int getTreeLvl()
	{
		return treeLvl;
	}

	int getNumOfNodeDel()
	{
		return numOfNodeDel;
	}

	Node *getRoot()
	{
		return root;
	}
};