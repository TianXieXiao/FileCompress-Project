 #pragma warning(disable:4996) 
#include <iostream>
using namespace std;
#include "FileCompress.hpp"


int main()
{
	FileCompress fc;
	fc.CompressFile("1.png");
	fc.UnCompressFile("2.hzp");
	return 0;
}