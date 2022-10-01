#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "bufferPool.h"
#include "BPlusTree.cpp"
#include "table_printer.h"
using namespace std;


#pragma region Printing functions
size_t get_longest_text(const std::vector<std::string>& lines)
{
    size_t ret = 0;
    for (auto& line : lines)
    {
        if (line.size() > ret)
        {
            ret = line.size();
        }
    }
    return ret;
}

void print_border_top_or_bottom_line(size_t longest_text)
{
    for (size_t i = 0; i < longest_text + 4; ++i)
    {
        std::cout << "*";
    }
    std::cout << "\n";
}

void print_second_top_or_second_bottom_line(size_t longest_text)
{
    std::cout << "*";
    for (size_t i = 0; i < longest_text + 2; ++i)
    {
        std::cout << " ";
    }
    std::cout << "*";
    std::cout << "\n";
}

void print_line(const std::string& text, size_t longest_text)
{
    std::cout << "*";
    std::cout << " ";
    std::cout << text;
    std::cout << std::string(longest_text + 1 - text.size(), ' ');
    std::cout << "*";
    std::cout << "\n";
}

void print_lines_with_border(const std::vector<std::string>& lines)
{
    size_t longest_text = get_longest_text(lines);
    print_border_top_or_bottom_line(longest_text);
    print_second_top_or_second_bottom_line(longest_text);
    for (auto& line : lines)
    {
        print_line(line, longest_text);
    }
    print_second_top_or_second_bottom_line(longest_text);
    print_border_top_or_bottom_line(longest_text);
}
#pragma endregion

using bprinter::TablePrinter;

int main()
{
     const int MEMORY = 300000000; // 300 MB of memory allocated

     uint blockSize[2] = {200, 500};

     int maxOfNode = 0;
     for (int i = 0; i < (sizeof(blockSize) / sizeof(int)); i++)
     {
         vector<string> title = { "-------------------------------------------------------------------------- Block size " + to_string(blockSize[i]) + "--------------------------------------------------------------------------" };
         print_lines_with_border(title);

          fstream file;
          file.open("data/data.tsv", ios::in);

          bufferPool bufferPool{MEMORY, blockSize[i]};
          vector<Location> dataset;

          cout << endl
              << "Reading dataset..." << endl
              << endl;
          if (file.is_open())
          {

               string line;
               int recordSize= 0;
               // skip title
               getline(file, line);
               while (getline(file, line))
               {

                    Record record;
                    Location location;
                    string token;

                    strcpy(record.tconst, line.substr(0, line.find('\t')).c_str());
                    stringstream ss(line);
                    getline(ss, token, '\t');
                    ss >> record.averageRating >> record.numVotes;

                    recordSize = sizeof(record);
                    location = bufferPool.insertRecord(recordSize);
                    dataset.push_back(location);

                    void *memoryLocation = (uchar *)location.blockLocation + location.offset;
                    memcpy(memoryLocation, &record, sizeof(record));
               }
               maxOfNode = blockSize[i] / recordSize;
          }
          cout << endl
              << "Done!" << endl
              << endl;

          vector<string> exp1 = { "------------------------Experiment 1------------------------" };
          print_lines_with_border(exp1);
            
          TablePrinter exp1_table(&std::cout);
          exp1_table.AddColumn("Number of blocks", 25);
          exp1_table.AddColumn("Size of Database (MB)", 25);

          exp1_table.PrintHeader();
          exp1_table << bufferPool.getNumOfBlockAlloc() << double(bufferPool.getTotalRecordSize()) / (1000 * 1000);
          exp1_table.PrintFooter();


          BPlusTree tree = BPlusTree(maxOfNode);
          bool isTreeEmpty = true;

          vector<string> exp2 = { "------------------------Experiment 2------------------------" };
          print_lines_with_border(exp2);

          for (int i = 0; i < dataset.size(); ++i)
          {
               if (i > 0)
                    isTreeEmpty = false;

               void *mainMemoryAddress = (uchar *)dataset[i].blockLocation + dataset[i].offset;
               uint numVotes = (*(Record *)mainMemoryAddress).numVotes;
               tree.Insert(dataset[i], numVotes, isTreeEmpty);
          }
          TablePrinter exp2_table(&std::cout);
          exp2_table.AddColumn("Maximum key for each node", 25);
          exp2_table.AddColumn("Total number of nodes", 20);
          exp2_table.AddColumn("Height of the Tree", 20);

          exp2_table.PrintHeader();
          exp2_table << maxOfNode << tree.getNumOfNode() << tree.getTreeLvl();
          exp2_table.PrintFooter();
          tree.Display(tree.getRoot(), 1, 0);


          vector<string> exp3 = { "------------------------Experiment 3------------------------" };
          print_lines_with_border(exp3);
          tree.retrievedetails(500, 500, &bufferPool);


          vector<string> exp4 = { "------------------------Experiment 4------------------------" };
          print_lines_with_border(exp4);
          tree.retrievedetails(30000, 40000, &bufferPool);

          vector<string> exp5 = { "------------------------Experiment 5------------------------" };
          print_lines_with_border(exp5);
          tree.RemoveEvent(1000, &bufferPool);

          TablePrinter exp5_table(&std::cout);
          exp5_table.AddColumn("Total number of nodes left", 30);
          exp5_table.AddColumn("Total number of nodes deleted", 35);
          exp5_table.AddColumn("Height of the Tree", 20);

          exp5_table.PrintHeader();
          exp5_table << tree.getNumOfNodeDel() << tree.getNumOfNode() << tree.getTreeLvl();
          exp5_table.PrintFooter();
         
     }
     return 0;
}
