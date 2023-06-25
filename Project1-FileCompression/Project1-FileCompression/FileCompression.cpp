 #pragma warning(disable:4996) 
#include "FileCompression.h"


#include <iostream>


FileCompression::FileCompression()
{
	for (int i = 0; i < 256; ++i)
	{
		fileByteInfo[i].ch = i;
		//fileByteInfo[i].appearCount = 0;
	}
}
bool FileCompression::CompressionFile(const string& filepath)
{
	//1.统计源文件中每个字节出现的次数---保存
	FILE* pf = fopen(filepath.c_str(), "rb");
	if (nullptr == pf)
	{
		cout << "打开文件失败！" << endl;
		return false;
	}

	//文件大小暂时不知道---此处需要采用循环的方式来获取源文件中的内容
	uch readBuff[1024];//一次读取1k的内容
	while (true)
	{
		//本意：想要一次性读取1024个字节，但实际读取到了rdsize个字节
		size_t rdsize = fread(readBuff, 1, 1024, pf);
		if (0 == rdsize)
		{
			//现在已经读取到了文件的末尾
			break;
		}

		//进行统计
		for (size_t i = 0; i < rdsize; ++i)
		{
			//利用到了：直接定址法---以字符的ASCII值作为数组的下标来进行快速的统计
			//这也是数组fileByteInfo给到256个的原因
			fileByteInfo[readBuff[i]].appearCount++;
		}
	}

	//2.根据统计的结果创建huffman树
	//注意：在创建huffman树期间，需要将出现次数为0的字节剔除掉
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreateHuffmanTree(fileByteInfo, 256,invalid);

	//3.借助huffman树获取每个字节的编码
	GenerateHuffmanCode(ht.GetRoot());

	//4.写解压缩时需要用到的信息
	FILE* fo = fopen("2.hzp", "wb");
	WriteHead(fo,filepath);

	//5.使用字节的编码对源文件重新进行改写
	//注意：在后续读取pf文件时，需要将pf文件指针挪动到文件起始位置
	//因为：刚开始在统计文件中字节出现次数的时候已经读取过一遍文件了，pf已经在文件末尾了
	//fseek(pf, 0, SEEK_SET);
	rewind(pf);


	uch ch = 0;
	uch bitCount = 0;
	while (true)
	{
		size_t rdsize = fread(readBuff, 1, 1024, pf);
		if (0 == rdsize)
			break;

		//用编码改写字节---改写的结果需要放到压缩结果文件当中
		for (size_t i = 0; i < rdsize; ++i)
		{
			// readBuff[i]--->'A'--->"100"
			string& strCode = fileByteInfo[readBuff[i]].strCode;

			// 只需要将字符串格式的二进制编码往字节中存放
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1;   //高位丢弃，低位补0
				if ('1' == strCode[j])
					ch |= 1;

				//当ch中的8个比特位填充满之后，需要将该字节写入到压缩文件当中
				bitCount++;
				if (8 == bitCount)
				{
					fputc(ch, fo);
					bitCount = 0;
				}
			}
		}

	}

	//检测：ch不够8个bit位，实际是没有写进去的
	if (bitCount > 0 && bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fo);
	}

	fclose(pf);
	fclose(fo);
	return true;
}


bool FileCompression::UNCompressionFile(const string& filepath)
{
	//1.从压缩文件读取解压缩时需要用到的信息
	FILE* fi = fopen(filepath.c_str(), "rb");
	if (nullptr == fi)
	{
		cout << "打开压缩文件失败" << endl;
		return false;
	}

	//读取源文件后缀
	string postFix;
	GetLine(fi, postFix);

	//频次信息总的行数
	string strContent;
	GetLine(fi, strContent);
	size_t lineCount = atoi(strContent.c_str());

	//循环读取lineCount行：获取字节频次信息
	strContent = "";
	for (size_t i = 0; i < lineCount; ++i)
	{	
		GetLine(fi, strContent);
		if ("" == strContent)
		{
			//说明刚刚读取到的是一个换行
			strContent += "\n";
			GetLine(fi, strContent);
		}

			//fileByteInfo[strContent[0]].ch = strContent[0];
			fileByteInfo[(uch)strContent[0]].appearCount = atoi(strContent.c_str() + 2);
			strContent = "";	
	}

	//2.恢复huffman树
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreateHuffmanTree(fileByteInfo, 256, invalid);

	//3.读取压缩数据，结合huffman树进行解压缩
	string filename("5");
	filename += postFix;
	FILE* fo = fopen(filename.c_str(), "wb");

	uch readBuff[1024];
	uch bitCount = 0;
	HuffmanTreeNode<ByteInfo>* cur = ht.GetRoot();
	const int fileSize = cur->weight.appearCount;
	int compressSize = 0;
	while (true)
	{
		size_t rdsize = fread(readBuff, 1, 1024, fi);
		if (0 == rdsize)
			break;

		for (size_t i = 0; i < rdsize; ++i)
		{
			//逐个字节比特位来进行解压缩
			uch ch = readBuff[i];
			bitCount = 0;
			while (bitCount < 8)
			{
				if (ch & 0x80)
					cur = cur->right;
				else
					cur = cur->left;

				
				if (nullptr == cur->left && nullptr == cur->right)
				{
					fputc(cur->weight.ch, fo);
					cur = ht.GetRoot();
					compressSize++;
					//如果成功解压缩字节的个数与源文件大小相同时则解压缩结束
					if (compressSize == fileSize)
						break;
				}

				bitCount++;
				ch <<= 1;

			}
		}
	}

	fclose(fi);
	fclose(fo);
	return true;
}

void FileCompression::GetLine(FILE* fi, string& strContent)
{
	uch ch;
	while (!feof(fi))
	{
		ch = fgetc(fi);
		if (ch == '\n')
			break;

		strContent += ch;
	}
}

//1.txt
void FileCompression::WriteHead(FILE* fo, const string& filepath)
{
	//1.先获取源文件的后缀
	string postFix = filepath.substr(filepath.rfind('.'));
	postFix += "\n";
	fwrite(postFix.c_str(), 1, postFix.size(), fo);

	//2.构造字节频次信息以及统计有效字节总的行数
	string chAppearCount;
	size_t lineCount = 0;
	for (size_t i = 0; i < 256; ++i)
	{
		if (fileByteInfo[i].appearCount > 0)
		{
			chAppearCount += fileByteInfo[i].ch;
			chAppearCount += ":";
			chAppearCount += to_string(fileByteInfo[i].appearCount);
			chAppearCount += "\n";
			lineCount++;
		}
	}

	//3.写总行数以及频次信息
	string totalLine = to_string(lineCount);
	totalLine += "\n";
	fwrite(totalLine.c_str(), 1, totalLine.size(), fo);
	fwrite(chAppearCount.c_str(), 1, chAppearCount.size(), fo);
}

void FileCompression::GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root)
{
	//递归退出条件
	if (nullptr == root)
		return;

	//因为在huffman树当中，所有有效的权值全部在叶子节点的位置
	//当遍历到叶子节点的位置时，该权值对应的编码就拿到了
	if (nullptr == root->left && nullptr == root->right)
	{   //获取编码的核心
		HuffmanTreeNode<ByteInfo>* cur = root;
		HuffmanTreeNode<ByteInfo>* parent = root->parent;
		string& strCode = fileByteInfo[cur->weight.ch].strCode;
		while (parent)
		{
			if (cur == parent->left)
				strCode += '0';
			else
				strCode += '1';
			cur = parent;
			parent = cur->parent;
		}

		reverse(strCode.begin(), strCode.end());
	}

	GenerateHuffmanCode(root->left);//往左获取
	GenerateHuffmanCode(root->right);//往右获取
}