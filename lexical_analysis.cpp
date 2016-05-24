#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "cstring"
#include "set"
using namespace std;

#define MAX_FORM 200
#define MAX_DFA 200
#define MAX_END_WORD 200

//MAX_FORM为最大生成式个数
//MAX_DFA为DFA矩阵的最大行数,即节点个数
//MAX_END_WORD为最大终结符个数

struct Node
{
	char word;
	int data;
	Node* next;
};

struct NFA
{
	int size;//节点个数,不包含-1终态
	int first_word;//入口
	set<char> end_word;//终结符集合
	Node* head;
};

struct DFA
{
	int w;//矩阵宽度
	int h;//矩阵高度
	set<int> end;
	char word[MAX_END_WORD];
	int x[MAX_DFA][MAX_END_WORD];
};

class grammar
{
	public:
		int size;
		int first_word;
		set<char> start_word;
		set<char> end_word;
		char form[MAX_FORM][5];

		bool is_start(char word)
		{
			if(start_word.find(word)!=start_word.end())
				return true;
			return false;
		}
		bool is_end(char word)
		{
			if(end_word.find(word)!=end_word.end())
				return true;
			return false;
		}
};

//从文件读取一个文法
grammar read_file(char* file_path)
{
	ifstream input_file;
	grammar G;
	G.size = 0;
	char first_word;
	char buf[5];
	input_file.open(file_path);
	input_file>>buf;
	G.start_word.insert(buf[0]);
	first_word=buf[0];
	G.end_word.insert(buf[3]);
	for(int i=0;i<5;i++)
		G.form[G.size][i]=buf[i];
	G.size++;
	while(!input_file.eof())
	{
		input_file>>buf;
		G.start_word.insert(buf[0]);
		G.end_word.insert(buf[3]);
		for(int i=0;i<5;i++)
			G.form[G.size][i]=buf[i];
		G.size++;
	}
	int count=0;
	set<char>::iterator it;
	for(it=G.start_word.begin();it!=G.start_word.end();it++)
	{
		if((*it)==first_word)
		{
			G.first_word=count;
			break;
		}
		count++;
	}
	return G;
}

NFA nfa_builder(grammar G)
{	
	int n = G.start_word.size();
	Node* _list = new Node[n];
	NFA nf;
	nf.size=n;
	nf.first_word=G.first_word;
	nf.end_word=G.end_word;
	nf.head=_list;

	int i=0;
	char buf[5];
	set<char>::iterator it1;
	for(it1=G.start_word.begin();it1!=G.start_word.end();it1++)
	{
		Node* p= &_list[i];
		p->next=NULL;
		(*p).data=0;
		for(int j=0;j<G.size;j++)
		{
			for(int ii=0;ii<5;ii++)
				buf[ii]=G.form[j][ii];
			if(buf[0]!=(*it1))
				continue;
			else
			{
				Node* q = new Node;
				q->next = NULL;
				(*q).data = -1;
				int index=0;
				set<char>::iterator it2;
				for(it2=G.start_word.begin();it2!=G.start_word.end();it2++)
				{
					if((*it2)==buf[4])
					{
						(*q).data = index++;
						break;
					}
					index++;
				}
				(*q).word=buf[3];
				p->next=q;
				p=q;
				_list[i].data++;
			}
		}
		i++;
	}

	// //测试NFA正确性
	// Node* h1=nf.head;
	// cout<<"first word:"<<nf.first_word<<endl;
	// for(int i=0;i<n;i++)	
	// {	
	// 	Node* p1 = h1[i].next;
	// 	for(int j=0;j<h1[i].data;j++)
	// 	{
	// 		cout<<(*p1).word<<":"<<(*p1).data<<" ";
	// 		p1=p1->next;
	// 	}
	// 	cout<<endl;
	// }

	return nf;
}

set<int> move(const NFA nfx, char a, set<int> old, int &flag)
{
	int n=nfx.size;
	//set<int> nt = old;
	set<int> nt ;
	Node* head=nfx.head;
	for(int i=0;i<n;i++)		
		if(old.find(i)!=old.end())//old集中含有i
		{
			Node* p = head[i].next;
			for(int j=0;j<head[i].data;j++)
			{
				if((*p).word==a)
				{
					nt.insert((*p).data);
					flag=1;//可行路径
				}
				p=p->next;
			}
		}
	return nt;
}

set<set<int> >::iterator where(const set<set<int> > &c, const set<set<int> > &mark)
{
	set<set<int> >::iterator it;
	for(it=c.begin();it!=c.end();it++)
		if(mark.find(*it)==mark.end())
			return it;
	return c.end();
}

DFA dfa_builder(NFA nfx)
{
	DFA df;
	df.w = nfx.end_word.size();
	for(int i=0;i<MAX_DFA;i++)
		for(int j=0;j<MAX_END_WORD;j++)
			df.x[i][j]=-2;//表示无效路径
	set<set<int> > c;
	set<set<int> > cc;
	set<int> t;
	t.insert(nfx.first_word);
	c.insert(t);
	int sp=0;
	set<int> sign[MAX_DFA];
	sign[sp]=t;
	sp++;
	while(where(c,cc)!=c.end())
	{
		set<set<int> >::iterator it1;
		it1=where(c,cc);
		t=(*it1);
		int cow=0;
		for(int i=0;i<sp;i++)
			if(sign[i]==t)
			{
				cow=i;
				break;
			}
		cc.insert(t);//标记t
		int col=0;
		set<char>::iterator it2;
		for(it2=nfx.end_word.begin();it2!=nfx.end_word.end();it2++)
		{
			df.word[col] = (*it2);
			int flag=0;//环路标记
			set<int> u = move(nfx, (*it2), t, flag);
			if(c.find(u)==c.end())//u是新的
			{
				c.insert(u);//将u加入c
				sign[sp]=u;//记下u的位置
				df.x[cow][col]=sp;//构造DFA
				if(u.find(-1)!=u.end())//判断是否为结束符
					df.end.insert(sp);//记录结束符
				sp++;
			}
			else if(flag==1)//u已存在于c中,且构成可行路径
			{
				for(int isp=0;isp<sp;isp++)
				{
					if(u==sign[isp])//找到u的位置
					{
						df.x[cow][col]=isp;//构造DFA环路
						break;
					}
				}
			}
			col++;
		}
	}
	df.h = sp;

	// //测试DFA正确性
	// for(int i=0;i<sp;i++)
	// {
	// 	for(int j=0;j<df.w;j++)
	// 		cout<<df.x[i][j]<<" ";
	// 	cout<<endl;
	// }
	return df;
}

bool is_dfa(DFA df, char s[])
{
	int p=0;
	int end_index;
	for(int i=0;s[i]!='\0';i++)
	{
		for(int j=0;j<df.w;j++)
		{
			if(s[i]==df.word[j])
			{
				p=df.x[p][j];
				end_index=p;
				if(i==0&&p<0)
					return false;
				break;
			}
			if(p<0||j==df.w-1)
				return false;
		}
	}
	if(df.end.find(end_index)==df.end.end())
		return false;
	return true;
}

bool is_identifier(char s[])
{
	char file_path[] = "input_identifier.txt";
	grammar G = read_file(file_path);
	NFA nfx = nfa_builder(G);
	DFA df = dfa_builder(nfx);
	if(is_dfa(df,s))
		return true;
	else
		return false;
}

bool is_const(char s[])
{
	char file_path[] = "input_const.txt";
	grammar G = read_file(file_path);
	NFA nfx = nfa_builder(G);
	DFA df = dfa_builder(nfx);
	if(is_dfa(df,s))
		return true;
	else
		return false;
}

bool is_key_word(string s)
{
	if(s=="int"||s=="for"||s=="double"||s=="if"||s=="else"||s=="namespace"||s=="std"||s=="return"||s=="main"||s=="void"||s=="endl"||s=="cout"||s=="cin")
		return true;
	return false;
}

bool is_limiter(string s)
{
	if(s=="{")
		return true;
	else if(s=="}")
		return true;
	else if(s=="(")
		return true;
	else if(s==")")
		return true;
	else if(s==";")
		return true;
	else if(s==",")
		return true;
	else
		return false;
}

bool is_operator(string s)
{
	if(s=="+"||s=="-"||s=="++"||s=="<"||s==">"||s=="*"||s=="--"||s==">="||s=="<="||s=="="||s=="\""||s=="+="||s=="<<"||s==">>"||s=="-=")
		return true;
	return false;
}

bool is_empty(string s)
{
	if(s==""||s==" ")
		return true;
	return false;
}

int what_this(string str)
{
	char temp[100];
	char *cr=temp;
	strcpy(cr,str.c_str());
	for(int i=0;cr[i]!='\0';i++)
	{
		if((cr[i]>='a'&&cr[i]<='z')||(cr[i]>='A'&&cr[i]<='Z'))
			cr[i]='l';
		if(cr[i]>='0'&&cr[i]<='9')
			cr[i]='d';
	}
	string s=str;
	if(is_limiter(s))
		return 1;
	if(is_operator(s))
		return 2;
	if(is_key_word(s))
		return 3;
	if(is_empty(s))
		return 4;
	if(is_const(cr))
		return 5;
	if(is_identifier(cr))
		return 6;
	return 0;
}

string read_code(string str)
{
	int token = what_this(str);
	string print_word;
	switch(token)
	{
	case 1:
		print_word="[limiter]";
		break;
	case 2:
		print_word="[operator]";
		break;
	case 3:
		print_word="[key_word]";
		break;
	case 4:
		print_word=" ";
		break;
	case 5:
		print_word="[const]";
		break;
	case 6:
		print_word="[identifier]";
		break;
	default:
		print_word="[error_word]";
	}
	return print_word;
}

void print_token()
{
	ifstream input_file;
	fstream output_file("token_table.txt", ios::out);
	input_file.open("source_code.txt");//读取源代码
	string buf;
	while(!input_file.eof())
	{
		input_file>>buf;
		output_file<<buf<<' '<<read_code(buf)<<endl;
	}
	input_file.close();
	output_file.close();
}

int main()
{
	print_token();
	return 0;
}