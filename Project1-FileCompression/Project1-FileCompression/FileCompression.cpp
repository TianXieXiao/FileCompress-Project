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
	//1.ͳ��Դ�ļ���ÿ���ֽڳ��ֵĴ���---����
	FILE* pf = fopen(filepath.c_str(), "rb");
	if (nullptr == pf)
	{
		cout << "���ļ�ʧ�ܣ�" << endl;
		return false;
	}

	//�ļ���С��ʱ��֪��---�˴���Ҫ����ѭ���ķ�ʽ����ȡԴ�ļ��е�����
	uch readBuff[1024];//һ�ζ�ȡ1k������
	while (true)
	{
		//���⣺��Ҫһ���Զ�ȡ1024���ֽڣ���ʵ�ʶ�ȡ����rdsize���ֽ�
		size_t rdsize = fread(readBuff, 1, 1024, pf);
		if (0 == rdsize)
		{
			//�����Ѿ���ȡ�����ļ���ĩβ
			break;
		}

		//����ͳ��
		for (size_t i = 0; i < rdsize; ++i)
		{
			//���õ��ˣ�ֱ�Ӷ�ַ��---���ַ���ASCIIֵ��Ϊ������±������п��ٵ�ͳ��
			//��Ҳ������fileByteInfo����256����ԭ��
			fileByteInfo[readBuff[i]].appearCount++;
		}
	}

	//2.����ͳ�ƵĽ������huffman��
	//ע�⣺�ڴ���huffman���ڼ䣬��Ҫ�����ִ���Ϊ0���ֽ��޳���
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreateHuffmanTree(fileByteInfo, 256,invalid);

	//3.����huffman����ȡÿ���ֽڵı���
	GenerateHuffmanCode(ht.GetRoot());

	//4.д��ѹ��ʱ��Ҫ�õ�����Ϣ
	FILE* fo = fopen("2.hzp", "wb");
	WriteHead(fo,filepath);

	//5.ʹ���ֽڵı����Դ�ļ����½��и�д
	//ע�⣺�ں�����ȡpf�ļ�ʱ����Ҫ��pf�ļ�ָ��Ų�����ļ���ʼλ��
	//��Ϊ���տ�ʼ��ͳ���ļ����ֽڳ��ִ�����ʱ���Ѿ���ȡ��һ���ļ��ˣ�pf�Ѿ����ļ�ĩβ��
	//fseek(pf, 0, SEEK_SET);
	rewind(pf);


	uch ch = 0;
	uch bitCount = 0;
	while (true)
	{
		size_t rdsize = fread(readBuff, 1, 1024, pf);
		if (0 == rdsize)
			break;

		//�ñ����д�ֽ�---��д�Ľ����Ҫ�ŵ�ѹ������ļ�����
		for (size_t i = 0; i < rdsize; ++i)
		{
			// readBuff[i]--->'A'--->"100"
			string& strCode = fileByteInfo[readBuff[i]].strCode;

			// ֻ��Ҫ���ַ�����ʽ�Ķ����Ʊ������ֽ��д��
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				ch <<= 1;   //��λ��������λ��0
				if ('1' == strCode[j])
					ch |= 1;

				//��ch�е�8������λ�����֮����Ҫ�����ֽ�д�뵽ѹ���ļ�����
				bitCount++;
				if (8 == bitCount)
				{
					fputc(ch, fo);
					bitCount = 0;
				}
			}
		}

	}

	//��⣺ch����8��bitλ��ʵ����û��д��ȥ��
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
	//1.��ѹ���ļ���ȡ��ѹ��ʱ��Ҫ�õ�����Ϣ
	FILE* fi = fopen(filepath.c_str(), "rb");
	if (nullptr == fi)
	{
		cout << "��ѹ���ļ�ʧ��" << endl;
		return false;
	}

	//��ȡԴ�ļ���׺
	string postFix;
	GetLine(fi, postFix);

	//Ƶ����Ϣ�ܵ�����
	string strContent;
	GetLine(fi, strContent);
	size_t lineCount = atoi(strContent.c_str());

	//ѭ����ȡlineCount�У���ȡ�ֽ�Ƶ����Ϣ
	strContent = "";
	for (size_t i = 0; i < lineCount; ++i)
	{	
		GetLine(fi, strContent);
		if ("" == strContent)
		{
			//˵���ոն�ȡ������һ������
			strContent += "\n";
			GetLine(fi, strContent);
		}

			//fileByteInfo[strContent[0]].ch = strContent[0];
			fileByteInfo[(uch)strContent[0]].appearCount = atoi(strContent.c_str() + 2);
			strContent = "";	
	}

	//2.�ָ�huffman��
	HuffmanTree<ByteInfo> ht;
	ByteInfo invalid;
	ht.CreateHuffmanTree(fileByteInfo, 256, invalid);

	//3.��ȡѹ�����ݣ����huffman�����н�ѹ��
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
			//����ֽڱ���λ�����н�ѹ��
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
					//����ɹ���ѹ���ֽڵĸ�����Դ�ļ���С��ͬʱ���ѹ������
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
	//1.�Ȼ�ȡԴ�ļ��ĺ�׺
	string postFix = filepath.substr(filepath.rfind('.'));
	postFix += "\n";
	fwrite(postFix.c_str(), 1, postFix.size(), fo);

	//2.�����ֽ�Ƶ����Ϣ�Լ�ͳ����Ч�ֽ��ܵ�����
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

	//3.д�������Լ�Ƶ����Ϣ
	string totalLine = to_string(lineCount);
	totalLine += "\n";
	fwrite(totalLine.c_str(), 1, totalLine.size(), fo);
	fwrite(chAppearCount.c_str(), 1, chAppearCount.size(), fo);
}

void FileCompression::GenerateHuffmanCode(HuffmanTreeNode<ByteInfo>* root)
{
	//�ݹ��˳�����
	if (nullptr == root)
		return;

	//��Ϊ��huffman�����У�������Ч��Ȩֵȫ����Ҷ�ӽڵ��λ��
	//��������Ҷ�ӽڵ��λ��ʱ����Ȩֵ��Ӧ�ı�����õ���
	if (nullptr == root->left && nullptr == root->right)
	{   //��ȡ����ĺ���
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

	GenerateHuffmanCode(root->left);//�����ȡ
	GenerateHuffmanCode(root->right);//���һ�ȡ
}