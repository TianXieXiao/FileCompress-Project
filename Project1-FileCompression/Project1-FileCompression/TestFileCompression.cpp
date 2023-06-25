 #pragma warning(disable:4996) 
#include <iostream>
using namespace std;

//#include "HuffmanTree.hpp"
#include "FileCompression.h"

int main()
{
	//TestHuffmanTree();
	FileCompression fc;
	fc.CompressionFile("4.jpg");
	fc.UNCompressionFile("2.hzp");

	return 0;
}