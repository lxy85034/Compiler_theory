#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "cstring"
#include "set"
#include "vector"
#include "map"
#include "stack"
#include "iomanip"
using namespace std;

#define MAX_W 100
#define MAX_H 1000

struct word//token值
{
	int type;
	string value;
};

struct form//语法产生式
{
	string start;//产生式左部
	string end;//产生式右部
};

struct point
{
	string type;//类型:ACTION GOTO 归约 接受
	int value;
	string start;
	string end;
};

struct table
{
	int w;//宽度
	int h;//高度
	point fx[MAX_H][MAX_W];
	string head[MAX_W];
	set<string> vv;//符号表
};



struct project//单个项目
{
	string start;
	string end;
	int dot;//表示项目中点的位置,初始为0
	string symbol;
	bool operator<(const project  &b)const //重载<运算符
	{
		string str1,str2;
		str1 = this->start + this->end + this->symbol;
		str2 = b.start + b.end + b.symbol;
        if(str1 < str2)
        	return true;
        else if(str1 > str2)
        	return false;
        else if(str1 == str2)
        {
        	if(this->dot < b.dot)
        		return true;
        	else
        		return false;
        }
    }
    bool operator==(const project  &b)const //重载==运算符
	{
        if(this->end == b.end && this->start == b.start && this->dot == b.dot && this->symbol == b.symbol)
        	return true;
        else
        	return false;
    } 
};

vector<word> read_token(char* file_path)
{
	ifstream token_file;
	token_file.open(file_path);
	string buf_1,buf_2;
	vector<word> v;
	while(!token_file.eof())
	{
		word new_word;
		token_file>>buf_1>>buf_2;
		new_word.value = buf_1;
		if(buf_2=="[key_word]")
			new_word.type = 1;
		else if(buf_2=="[identifier]")
			new_word.type = 2;
		else if(buf_2=="[operator]")
			new_word.type = 3;
		else if(buf_2=="[limiter]")
			new_word.type = 4;
		else if(buf_2=="[const]")
			new_word.type = 5;
		else
			new_word.type = 0;
		v.push_back(new_word);
	}
	return v;
}

vector<form> read_grammar(char* file_path)
{
	ifstream grammar_file;
	grammar_file.open(file_path);
	string str;
	vector<form> v;
	while(!grammar_file.eof())
	{
		grammar_file>>str;
		form buf;
		buf.start = str.substr(0,1);
		buf.end = str.substr(3,str.npos);
		v.push_back(buf);
	}
	return v;
}

vector<string> split(const string str)
{
	vector<string> result;
	for(int i=0;i<str.size();i++)
	{
		string temp = str.substr((string::size_type)i,(string::size_type)(1));
		result.push_back(temp);
	}
	return result;
}

stack<string> str_stack(const string str)
{
	stack<string> result;
	for(int i=str.size()-1;i>=0;i--)
	{
		string temp = str.substr((string::size_type)i,(string::size_type)(1));
		result.push(temp);
	}
	return result;
}

set<string> find_vn(const vector<form> f)
{
	set<string> vn;
	int n = f.size();
	for(int i=0;i<n;i++)
	{
		string buf = f[i].start;
		vn.insert(buf);
	}
	return vn;
}

set<string> find_vt(const vector<form> f)
{
	set<string> vt;
	set<string> vn;
	vn = find_vn(f);
	int n = f.size();
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<f[i].end.size();j++)
		{
			string temp = f[i].end.substr((string::size_type)j,(string::size_type)(1));
			if(vn.find(temp)==vn.end())
				vt.insert(temp);
		}
	}
	if(vt.find("$")!=vt.end())
		vt.erase("$");
	return vt;
}

map<string,int>  get_empty(const vector<form> v_value)//求能推出空的非终结符
{
	vector<form> v;
    v = v_value;
    map<string,int> empty;
    set<string> vt = find_vt(v);
    set<string> vn = find_vn(v);
    set<string>::iterator it1;
    for(it1=vn.begin();it1!=vn.end();it1++)
        empty.insert(pair<string,int>((*it1),-1));
    vector<form>::iterator it2;
    for(it2=v.begin();it2!=v.end();it2++)//删除所有右部含有终结符的产生式
    {
        vector<string> right = split((*it2).end);//拆分产生式右部
        int right_size = right.size();
        int flag = 0;
        for(int j=0;j<right_size;j++)
        {
        	if((string)right.at(j)==(*it2).start)//出现了递归情况
			{
				v.erase(it2);
				it2--;
				flag=2;
				break;
			}
            if(vt.find(right.at(j))!=vt.end())//产生式右部含有终结符
            {
                flag = 1;
                break;
            }
        }
        if(flag==2)
        	continue;
        if(flag==1)
        {
            int count = 0;
            vector<form>::iterator it4;
            for(it4=v.begin();it4!=v.end();it4++)
                if((*it4).start==(*it2).start)
                    count++;
            if(count == 1)
                empty[(string)(*it2).start] = 0;
            v.erase(it2);
            it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
        }
    }
    for(it2=v.begin();it2!=v.end();it2++)
    {
        if((*it2).end=="$")
        {
            string temp = (*it2).start;
            empty[(string)(*it2).start] = 1;//能推出空
            vector<form>::iterator it3;
            for(it3=v.begin();it3!=v.end();it3++)
            {
                if((*it3).start==temp)
                {
                    v.erase(it3);//删除该终结符的所有产生式
                    it3=v.begin();//it3--
                    if(it2!=v.begin())
                        it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
                }
            }
        }
    }
    while(v.size()>0)
    {
        for(it2=v.begin();it2!=v.end();it2++)
        {
            vector<string> right = split((*it2).end);//拆分产生式右部
            int right_size = right.size();
            int flag = 0;
            for(int j=0;j<right_size;j++)
            {
                if(empty[(string)right.at(j)]==0)//产生式右部不能推出空
                {
                    flag = 1;
                    break;
                }
                else if(empty[(string)right.at(j)]==1)
                {
                    right.at(j)="";
                }
            }
            if(flag==1)//此产生式不能推出空
            {
                int count = 0;
                for(int k=0;k<v.size();k++)
                    if(v.at(k).start==(*it2).start)
                        count++;
                if(count == 1)
                    empty[(string)(*it2).start] = 0;
                v.erase(it2);
                it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
                continue;
            }
            for(int j=right_size-1;j>0;j--)
                right.at(j-1) += right.at(j);
            (*it2).end = right.at(0);
            if((*it2).end=="")
            {
                string temp = (*it2).start;
                empty[(string)(*it2).start] = 1;//能推出空
                vector<form>::iterator it3;
                for(it3=v.begin();it3!=v.end();it3++)
                {
                    if((*it3).start==temp)
                    {
                        v.erase(it3);//删除该终结符的所有产生式
                        it3=v.begin();//it3--
                        if(it2!=v.begin())
                            it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
                    }
                }
            }
        }
    }
    // //测试empty表是否正确
    // map<string,int>::iterator it;
    // for(it=empty.begin();it!=empty.end();it++)
    // cout<<it->first<<":"<<it->second<<endl;
    return empty;
}

set<string> _union(const set<string > a , const set<string> b)//求并集
{
	set<string> result;
	result = a;
	set<string>::iterator it;
	for(it=b.begin();it!=b.end();it++)
		result.insert(*it);
	return result;
}

set<string> get_first(string symbol , set<string > vt , set<string > vn , vector<form > f , map<string,int > empty)
{
	set<string> result;
	if(vt.find(symbol)!=vt.end())
	{
		result.insert(symbol);
		return result;
	}
	int flag=0;
	vector<form>::iterator it;
	for(it=f.begin();it!=f.end();it++)
	{
		if((*it).start==symbol)
		{
			if((*it).end=="$")
			{
				flag = 1;
				continue;
			}
			vector<string> right = split((*it).end);
			if(symbol==(string)right.at(0))//递归,跳过
				continue;
			int right_size = right.size();
			for(int i=0;i<right_size;i++)
			{
				if(result.find("$")!=result.end()&&flag==0)
					result.erase("$");
				if(symbol!=(string)right.at(i))
					result = _union(result , get_first(right.at(i) , vt , vn , f , empty));
				else
					break;
				if(empty[(string)right.at(i)]==0||vt.find(right.at(i))!=vt.end())
					break;
				else if(i==right_size-1)
					flag = 1;
			}
		}
		if(flag==1)
			result.insert("$");
	}
	return result;
}

map<string,set<string> > make_first(const set<string> vt , const set<string> vn , const vector<form> f , const map<string,int> empty)
{
	map<string,set<string> > result;
	set<string>::iterator it1;
	for(it1=vt.begin();it1!=vt.end();it1++)
	{
		set<string> buf;
		buf.insert((*it1));
		result.insert(pair<string,set<string> >((*it1),buf));
	}
	for(it1=vn.begin();it1!=vn.end();it1++)
	{
		set<string> buf;
		buf = get_first((*it1),vt,vn,f,empty);
		result.insert(pair<string,set<string> >((*it1),buf));
	}
	return result;
}

set<string> str_first(string str ,string symbol, map<string,set<string> > first , map<string,int> empty)
{
	set<string> result;
	if(str == "")
	{
		result.insert(symbol);
		return result;
	}
	vector<string> buf;
	buf = split(str);
	vector<string>::iterator it;
	for(it=buf.begin();it!=buf.end();it++)
	{
		result = _union(result,(first[(string)(*it)]));
		if(empty[(string)(*it)]==0)
			break;
	}
	if(it==buf.end() && empty[(string)(*(it-1))]==1)
		result.insert(symbol);
	if(result.find("$")!=result.end())
	{
		result.erase("$");
		result.insert("#");
	}
	return result;
}

string dot_right(string str,int dot)//获得右边第一个字符之后的串
{
	dot++;
	if(dot>=str.size())
		return "";
	return str.substr((string::size_type)dot,str.npos);
}

string dot_right_one(string str,int dot)//获得点右边第一个字符
{
	if(dot>=str.size())
		return "";
	return str.substr((string::size_type)dot,(string::size_type)(1));
}

set<project> make_closure(set<project> CI , vector<form> f , map<string,set<string> > first , map<string,int> empty)
{
	set<project> result;
	result = CI;
	if(result.size()==0)
		return result;
	set<project>::iterator pi;
	bool flag = false;
	for(pi=result.begin();pi!=result.end();pi++)
	{
		if(flag)
			pi=result.begin();
		int _size = result.size();
		for(int i=0;i<f.size();i++)
		{
			//cout<<dot_right_one((*pi).end,(*pi).dot)<<endl;
			if(f.at(i).start==dot_right_one((*pi).end,(*pi).dot))
			{
				set<string> buf = str_first(dot_right((*pi).end,(*pi).dot),(*pi).symbol,first,empty);
				set<string>::iterator it;
				for(it=buf.begin();it!=buf.end();it++)
				{
					project J;
					J.start = dot_right_one((*pi).end,(*pi).dot);
					J.end = f.at(i).end;
					J.dot = 0;
					J.symbol = (*it);
					result.insert(J);
					// cout<<J.start<<"->"<<J.end<<endl;
					// cout<<"size:"<<result.size()<<endl;;
				}
			}
		}
		if(result.size()== _size)
			break;
		else
			flag = true;
	}
	return result;
}

set<project> go_closure(string X , set<project> CI , vector<form> f , map<string,set<string> > first , map<string,int> empty)
{
	if(X=="")
		return make_closure(CI,f,first,empty);
	set<project> result;
	set<project>::iterator it;
	for(it=CI.begin();it!=CI.end();it++)
	{
		if(dot_right_one((*it).end,(*it).dot)==X)
		{
			project J;
			J.start = (*it).start;
			J.end = (*it).end;
			J.dot = (*it).dot+1;
			J.symbol = (*it).symbol;
			result.insert(J);
		}
	}
	result = make_closure(result,f,first,empty);
	return result;
}

int sum_dot(set<project> a)
{
	set<project>::iterator it;
	int sum = 0;
	for(it=a.begin();it!=a.end();it++)
	{
		sum += (*it).dot;
	}
	return sum;
}

table build_table(vector<form> f , map<string,set<string> > first , map<string,int> empty , set<string> vt , set<string> vn)//构造项目集族
{
	table tb;
	tb.vv = _union(vn,vt);
	tb.vv.insert("#");
	tb.w = tb.vv.size();
	for(int i=0;i<MAX_H;i++)
		for(int j=0;j<tb.w;j++)
			tb.fx[i][j].value=0;
	set<string>::iterator it6;
	int i_head = 0;
	for(it6=tb.vv.begin();it6!=tb.vv.end();it6++)
	{
		tb.head[i_head] = (*it6);
		i_head++;
	}
	vector<set<project> > C;//项目集族
	set<set<project> > set_C;
	set<project> I0;
	project I;
	I.start="X";
	I.dot=0;
	I.end="S";
	I.symbol="#";
	I0.insert(I);
	I0 = make_closure(I0,f,first,empty);//初始项目
	C.push_back(I0);
	set_C.insert(I0);

	for(int i=0;i<C.size();i++)
	{
		set<string> buf;
		point buf_table[MAX_W];
		set<project>::iterator it;
		for(it=C.at(i).begin();it!=C.at(i).end();it++)
		{
			string temp = dot_right_one((*it).end,(*it).dot);
			if(temp=="")//归约
			{
				point tp;
				tp.type = "r";
				tp.value = 0;
				tp.start = (*it).start;
				tp.end = (*it).end;
				if((*it).start=="X")
					tp.type = "acc";
				int index = 0;
				for(int k=0;k<tb.w;k++)
					if(tb.head[k]==(*it).symbol)
					{
						index = k;
						break;
					}
				buf_table[index]=tp;
				continue;
			}
			if(buf.find(temp)!=buf.end())
				continue;//已经跳转过,避免重复计算
			buf.insert(temp);
			if((*it).dot==1 && (*it).start=="X" && (*it).end=="S")//接受
			{
				point tp;
				tp.type = "acc";
				tp.value = 0;
				tp.start = (*it).start;
				tp.end = (*it).end;
				int index = 0;
				for(int k=0;k<tb.w;k++)
					if(tb.head[k]==(*it).symbol)
					{
						index = k;
						break;
					}
				buf_table[index]=tp;
				break;
			}
			set<project> J;
			J = go_closure(temp,C.at(i),f,first,empty);

			if(set_C.find(J)==set_C.end()||sum_dot(*set_C.find(J)) != sum_dot(J))//新的
			{
				C.push_back(J);
				set_C.insert(J);

				point tp;
				tp.value = set_C.size()-1;
				tp.start = (*it).start;
				tp.end = (*it).end;
				
				if(vt.find(temp)!=vt.end())//ACTION
					tp.type = "s";
				else if(vn.find(temp)!=vn.end())
					tp.type = "goto";
				int index = 0;
				for(int k=0;k<tb.w;k++)
					if(tb.head[k]==temp)
					{
						index = k;
						break;
					}
				buf_table[index]=tp;
				
				it=C.at(i).begin();//vector内存重新分配之后会导致迭代器失效!
			}
		}
		// table.push_back(buf_table);
		for(int px=0;px<tb.w;px++)
		{
			tb.fx[i][px]=buf_table[px];
			buf_table[px].value = 0;
		}
	}
	tb.h = C.size();
	return tb;
}

void print_table(table &tb)
{
	cout<<"============================================TABLE============================================"<<endl;
	cout<<"h="<<tb.h<<"  w="<<tb.w<<endl;
	int step=0;
	cout<<setw(6)<<"step";
	for(int i=0;i<tb.vv.size();i++)
	{
		cout<<setw(10)<<tb.head[i];
	}
	cout<<endl;
	for(int i=0;i<tb.h;i++)
	{
		cout<<setw(6)<<step++;
		for(int j=0;j<tb.w;j++)
		{
			//if(tb.fx[i][j].type=="r")
			
			if(tb.fx[i][j].type!="")
				cout<<setw(10)<<tb.fx[i][j].type<<tb.fx[i][j].value;
			else
				cout<<setw(10)<<"[ ]";
		}
		cout<<endl;
	}
	cout<<"============================================TABLE============================================"<<endl;
}

bool analysis(table &tb, string _str)//分析函数
{
	stack<int> status;//状态栈
	status.push(0);
	stack<string> str = str_stack(_str);//输入串
	stack<string> symbol;//符号栈
	symbol.push("#");
	int i=0,j=0;
	for(j=0;j<tb.w;j++)
	{
		if(tb.head[j]==str.top())
			break;
	}
	cout<<setw(5)<<"step"<<setw(20)<<"status"<<setw(20)<<"symbol"<<setw(20)<<"string"<<setw(20)<<"ACTION"<<endl;
	int step=0;
	while(tb.fx[i][j].type!="acc")
	{
		
		stack<int> copy_status;
		copy_status = status;
		cout<<setw(5)<<step++;
		cout<<setw(10);
		for(int k=0;k<status.size();k++)
		{
			cout<<copy_status.top();
			copy_status.pop();
		}
		cout<<"#"<<setw(10);
		stack<string> copy_symbol;
		copy_symbol = symbol;
		for(int k=0;k<symbol.size();k++)
		{
			cout<<copy_symbol.top();
			copy_symbol.pop();
		}
		cout<<setw(10);
		stack<string> copy_string;
		copy_string = str;
		for(int k=0;k<str.size();k++)
		{
			cout<<copy_string.top();
			copy_string.pop();
		}
		if(tb.fx[i][j].value==0)
			cout<<setw(10)<<tb.fx[i][j].type<<endl;
		else
			cout<<setw(10)<<tb.fx[i][j].type<<tb.fx[i][j].value<<endl;
		if(tb.fx[i][j].type=="s")
		{
			status.push(tb.fx[i][j].value);
			symbol.push(str.top());
			str.pop();
			i = tb.fx[i][j].value;
			for(j=0;j<tb.w;j++)
			{
				if(tb.head[j]==str.top())
					break;
			}
		}
		else if(tb.fx[i][j].type=="r")
		{
			int _size = tb.fx[i][j].end.size();
			for(int k=0;k<_size;k++)
			{
				symbol.pop();
				status.pop();
			}
			symbol.push(tb.fx[i][j].start);
			i = status.top();
			for(j=0;j<tb.w;j++)
				if(tb.head[j]==symbol.top())
				{
					status.push(tb.fx[i][j].value);
					break;
				}
			i = tb.fx[i][j].value;
			for(j=0;j<tb.w;j++)
			{
				if(tb.head[j]==str.top())
					break;
			}
			// cout<<"i="<<i<<" j="<<j<<endl;
		}
		else
			return false;
	}
	return true;
}

string get_input(vector<word> &token)
{
	string result = "";
	for(int i=0;i<token.size()-1;i++)
	{
		if(token.at(i).type==1)//key word
		{
			if(token.at(i).value=="cin")
				result += "n";
			else if(token.at(i).value=="cout")
				result += "u";
			else if(token.at(i).value=="int")
				result += "t";
			else if(token.at(i).value=="if")
				result += "r";
			else if(token.at(i).value=="else")
				result += "e";
			else if(token.at(i).value=="double")
				result += "d";
			else if(token.at(i).value=="endl")
				result += "k";
			else if(token.at(i).value=="for")
				result += "f";
			else 
				result +="*";//预留更改
		}
		else if(token.at(i).type==2)//identifier
		{
			result += "i";
		}
		else if(token.at(i).type==3)//operator
		{
			if(token.at(i).value.size()>1)
				result += "~";
			else
				result += token.at(i).value;
		}
		else if(token.at(i).type==4)//limiter
		{
			result += token.at(i).value;
		}
		else if(token.at(i).type==5)//const
		{
			result += "c";
		}
		else
			cout<<"The token table is error!"<<endl;
	}
	result += "#";
	return result;
}

int main()
{
	char s1[] = "input_grammar.txt";//2型文法路径
	char s2[] = "token_table.txt";//第一步生成的token表路径
	vector<form> v = read_grammar(s1);//从文件读取产生式
	set<string> vt = find_vt(v);//找出所有终结符
	set<string> vn = find_vn(v);//找出所有非终结符
	map<string,int> empty = get_empty(v);//构造可以推出空的非终结符表
	map<string,set<string> > first = make_first(vt,vn,v,empty);//构造FIRST集
	table tb = build_table(v,first,empty,vt,vn);//构造LR(1)分析表
	// print_table(tb);//预测打印分析表
	vector<word> token = read_token(s2);//读取生成的token表
	string input_str = get_input(token);//根据token表生成待查串
	if(analysis(tb, input_str))//进行LR(1)分析
		cout<<"\nacc!\n\n";
	else
		cout<<"\nerror!\n\n";
	return 0;
}