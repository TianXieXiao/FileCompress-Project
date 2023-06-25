#pragma once

#include <string>
using namespace std;

#include "HuffmanTree.hpp"

typedef unsigned char uch;

struct ByteInfo
{
	uch ch;
	int appearCount;//ch�ֽڳ��ֵĴ���
	string strCode;//ch�ֽڶ�Ӧ�ı���

	ByteInfo(int count = 0)
		:appearCount(count)
	{}

	ByteInfo operator+(const ByteInfo& b)const
	{
		ByteInfo temp;
		temp.appearCount = appearCount + b.appearCount;
		return temp;
	}

	bool operator>(const ByteInfo& b)const
	{
		return appearCount > b.appearCount;
	}

	bool operator!=(const ByteInfo& b)const
	{
		return appearCount != b.appearCount;
	}

	//bool operator==(const ByteInfo& b)const
	//{
	//	return appearCount == b.appearCount;
	//}
};

class FileCompression
{
public:
	FileCompression();
	bool CompressionFile(const string& filepath);
	bool UNCompressionFile(const string& filepath);

private:
	void GetLine(FILE* fi, string& strContent);
	void WriteHead(FILE* fo, const string& filepath);
	void GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root);
	/////////////////////////////////////////////
	//�ļ������ڴ����������ֽڷ�ʽ�洢��
	//�ֽ��ܹ���256��״̬
	//ֻ���һ������256��ByteInfo���͵������������ֽڳ��ֵ�Ƶ����Ϣ
	ByteInfo fileByteInfo[256];
};