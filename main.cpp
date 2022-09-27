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
     uint blockSize[2] = {200, 500};
     for (int i = 0; i < (sizeof(blockSize) / sizeof(int)); i++)
     {
          cout << "<------------------- Data reading for block size  " << blockSize[i] << ".. ------------------->"
               << "\n";

          fstream file;
          file.open("data/data_12.tsv", ios::in);

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

                    strcpy_s(record.tconst, line.substr(0, line.find('\t')).c_str());
                    stringstream ss(line);
                    getline(ss, token, '\t');
                    ss >> record.avgRating >> record.numVotes;

                    location = bufferPool.insertRecord(sizeof(record));
                    dataset.push_back(location);

                    void *memoryLocation = (uchar *)location.blockLocation + location.offset;
                    memcpy(memoryLocation, &record, sizeof(record));
               }
          }

          cout << endl
               << "------------------------Experiment 1------------------------" << endl
               << endl;
          cout << "Number of blocks = " << bufferPool.getNumOfBlkAlloc() << endl;
          cout << "Size of Database = " << double(bufferPool.getTotalRecordSize()) / (1000 * 1000) << "MB" << endl;

          cout << "\n <------------------- Completed reading for block size  " << blockSize[i] << ".. ------------------->"
               << "\n \n";

          BPlusTree tree;
          bool isTreeEmpty = true;
          cout << endl << "------------------------Experiment 2------------------------" << endl << endl;
          for (int i = 0; i < dataset.size(); ++i) {
              if (i > 0)
                  isTreeEmpty = false;

              void* mainMemoryAddress = (uchar*)dataset[i].blockLocation + dataset[i].offset;
              uint numVotes = (*(Record*)mainMemoryAddress).numVotes;
              tree.Insert(dataset[i], numVotes, isTreeEmpty);
          }
          cout << "Parameter (n) of B+ Tree = " << MAX << endl;
          cout << "Number of Nodes of B+ Tree = " << tree.getNumOfNode() << endl;
          cout << "Height of the B+ Tree = " << tree.getTreeLvl() << endl;
          tree.Display(tree.getRoot(), tree.getNumOfNode(), 0);
     }

     return 0;
}

// int nthOccurrence(const std::string &str, const std::string &findMe, int nth)
// {
//     size_t pos = 0;
//     int cnt = 0;

//     while (cnt != nth)
//     {
//         pos += 1;
//         pos = str.find(findMe, pos);
//         if (pos == std::string::npos)
//             return -1;
//         cnt++;
//     }
//     return pos;
// }