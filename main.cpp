#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "bufferPool.h"

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
          file.open("data/data_less.tsv", ios::in);

          bufferPool bufferPool{MEMORY, blockSize[i]};
          vector<Location> dataset;

          if (file.is_open())
          {
               string line;
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