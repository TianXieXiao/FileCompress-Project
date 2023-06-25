#pragma once


#include <queue>
#include <vector>
//#include <functional>
using namespace std;


template<class W>//Ȩֵ
struct HuffmanTreeNode
{
	HuffmanTreeNode<W>* left;
	HuffmanTreeNode<W>* right;
	HuffmanTreeNode<W>* parent;
	W weight;

	HuffmanTreeNode(const W& w = W())
		:left(nullptr)
		,right(nullptr)
		,parent(nullptr)
		,weight(w)
	{}
};//�������ڵ�ķ�ʽ


template<class W>
struct Com
{
	typedef HuffmanTreeNode<W> Node;
	bool operator()(const Node* left, const Node* right)
	{
		return left->weight > right->weight;
	}
};

template<class W>
class HuffmanTree
{
	typedef HuffmanTreeNode<W> Node;
public:
	HuffmanTree()
		:root(nullptr)
	{}

	~HuffmanTree()
	{
		Destroy(root);
	}

	void CreateHuffmanTree(const W array[], size_t size,const W& invalid)
	{
		//С��---���ȼ�����Ĭ��������Ǵ�ѣ�ֻ��Ҫ�޸���ȽϹ���
		std::priority_queue<Node*, vector<Node*>, Com<W>> q;
		//std::priority_queue<Node*, vector<Node*>, greater<Node*>> q;

		//1.��ʹ��������Ȩֵ����ֻ�и��ڵ�Ķ�����ɭ��
		for (size_t i = 0; i < size; ++i)
		{
			if (array[i] != invalid)
				q.push(new Node(array[i]));
		}

		//2.ѭ���������²��裬ֱ��������ɭ����ֻʣ��һ�ö�����Ϊֹ
		while (q.size() > 1)
		{
			//�Ӷ�����ɭ������ȡȨֵ��С�����ö�����
			Node* left = q.top();
			q.pop();

			Node* right = q.top();
			q.pop();

			//��left��right��Ϊĳ���½ڵ����������������һ���µĶ�����
			//�µĶ��������ڵ��Ȩֵ���������Һ��ӵ�Ȩֵ֮��
			Node* parent = new Node(left->weight + right->weight);
			parent->left = left;
			parent->right = right;
			left->parent = parent;
			right->parent = parent;

			//���µĶ��������뵽������ɭ����
			q.push(parent);
		}

		root = q.top();
	}

	Node* GetRoot()
	{
		return root;
	}

	void Destroy(Node*& proot)
	{
		if (proot)
		{
			Destroy(proot->left);
			Destroy(proot->right);
			delete proot;
			proot = nullptr;
		}
	}
private:
	Node* root;
};


//void TestHuffmanTree()
//{
//	int array[] = { 3, 1, 7, 5 };
//	HuffmanTree<int> t;
//	t.CreateHuffmanTree(array, sizeof(array) / sizeof(array[0]));
//}