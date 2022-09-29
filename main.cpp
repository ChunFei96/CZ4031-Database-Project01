#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "bufferPool.h"
#include "BPlusTree.cpp"

using namespace std;

int main()
{
     const int MEMORY = 300000000; // 300 MB of memory allocated

     // To test for 200B & 500B block size scenario
     // uint blockSize[2] = {200, 500};
     uint blockSize[1] = {100};
     for (int i = 0; i < (sizeof(blockSize) / sizeof(int)); i++)
     {
          cout << "<------------------- Data reading for block size  " << blockSize[i] << ".. ------------------->"
               << "\n";

          fstream file;
          file.open("data/data_5k.tsv", ios::in);

          bufferPool bufferPool{MEMORY, blockSize[i]};
          vector<Location> dataset;

          if (file.is_open())
          {
               string line;

               // skip title
               getline(file, line);
               while (getline(file, line))
               {
                    // cout << line;
                    // cout << "\n";

                    Record record;
                    Location location;
                    string token;

                    strcpy(record.tconst, line.substr(0, line.find('\t')).c_str());
                    stringstream ss(line);
                    getline(ss, token, '\t');
                    ss >> record.averageRating >> record.numVotes;

                    location = bufferPool.insertRecord(sizeof(record));
                    dataset.push_back(location);

                    void *memoryLocation = (uchar *)location.blockLocation + location.offset;
                    memcpy(memoryLocation, &record, sizeof(record));
               }
          }

          cout << endl
               << "---------------------------Experiment 1---------------------------" << endl
               << endl;
          cout << "Number of blocks = " << bufferPool.getNumOfBlockAlloc() << endl;
          cout << "Size of Database = " << double(bufferPool.getTotalRecordSize()) / (1000 * 1000) << "MB" << endl;

          cout << endl
               << "------------------------End of Experiment 1------------------------" << endl
               << endl;

          BPlusTree tree;
          bool isTreeEmpty = true;
          cout << endl
               << "---------------------------Experiment 2---------------------------"
               << endl;
          for (int i = 0; i < dataset.size(); ++i)
          {
               if (i > 0)
                    isTreeEmpty = false;

               void *mainMemoryAddress = (uchar *)dataset[i].blockLocation + dataset[i].offset;
               uint numVotes = (*(Record *)mainMemoryAddress).numVotes;
               tree.Insert(dataset[i], numVotes, isTreeEmpty);
          }
          cout << "Parameter (n) of B+ Tree = " << MAX << endl;
          cout << "Number of Nodes of B+ Tree = " << tree.getNumOfNode() << endl;
          cout << "Height of the B+ Tree = " << tree.getTreeLvl() << endl;
          tree.Display(tree.getRoot(), 1, 0);

          cout << endl
               << "------------------------End of Experiment 2------------------------"
               << endl;

          cout << endl
               << "---------------------------Experiment 3---------------------------" << endl
               << endl;
          cout << "Searching for 'numVotes' = 500..." << endl
               << endl;
          tree.retrievedetails(500, 500, &bufferPool);

          cout << endl
               << "------------------------End of Experiment 3------------------------"
               << endl;

          cout << endl
               << "---------------------------Experiment 4---------------------------" << endl
               << endl;
          cout << "Searching for 'numVotes' = 30000 to 40000..." << endl
               << endl;
          tree.retrievedetails(30000, 40000, &bufferPool);

          cout << endl
               << "------------------------End of Experiment 4------------------------"
               << endl;

          cout << "\n <------------------- Completed reading for block size  " << blockSize[i] << ".. ------------------->"
               << "\n \n";
     }
     return 0;
}