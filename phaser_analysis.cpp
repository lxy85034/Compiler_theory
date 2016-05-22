#include "stdio.h"
#include "iostream"
#include "fstream"
#include "string"
#include "cstring"
#include "set"
#include "vector"
#include "map"
using namespace std;

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

struct project//单个项目
{
	string start;
	string end;
	int dot;//表示项目中点的位置,初始为0
	string symbol;
	bool operator<(const project  &b)const //重载<运算符
	{
		char a_dot = (char)this->dot;
		string str1 = this->start+this->end+this->symbol+a_dot;
		char b_dot = (char)b.dot;
		string str2 = b.start+b.end+b.symbol+b_dot;
        if(str1 < str2)
         	return true;
        return false;
    }
    bool operator==(const project  &b)const //重载==运算符
	{
        if(this->end == b.end && this->start == b.end && this->dot == b.dot && this->symbol == b.symbol)
        	return true;
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
		bool flag = false;
		for(int j=0;j<right_size;j++)
			if(vt.find(right.at(j))!=vt.end())//产生式右部含有终结符
			{
				flag = true;
				break;
			}
		if(flag)
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
					it3--;
					if(it2!=v.begin())
						it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
				}
			}
		}
	}

	while(v.size()!=0)
	{
		for(it2=v.begin();it2!=v.end();it2++)
		{
			vector<string> right = split((*it2).end);//拆分产生式右部
			int right_size = right.size();
			bool flag = false;
			for(int j=0;j<right_size;j++)
			{
				if(empty[(string)right.at(j)]==0)//产生式右部含有终结符
				{
					flag = true;
					break;
				}
				else if(empty[(string)right.at(j)]==1)
				{
					right.at(j)="";
				}
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
						it3--;
						if(it2!=v.begin())
							it2--;//因为删除了一个产生式,所以迭代器要前移,否则会错过后一个值
					}
				}
			}
			if(flag)
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
			int right_size = right.size();
			for(int i=0;i<right_size;i++)
			{
				if(result.find("$")!=result.end()&&flag==0)
					result.erase("$");
				result = _union(result , get_first(right.at(i) , vt , vn , f , empty));
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

bool equal(set<project> a, set<project> b)
{
	if(a.size()!=b.size())
		return false;
	set<project>::iterator it1,it2;
	it1=a.begin();
	it2=b.begin();
	for(;it1!=a.end();)
	{
		if((*it1).start!=(*it2).start)
			return false;
		else if((*it1).end!=(*it2).end)
			return false;
		else if((*it1).symbol!=(*it2).symbol)
			return false;
		else if((*it1).dot!=(*it2).dot)
			return false;
	}
	return true;
}

bool contain(set<project> a, set<set<project> > b)
{
	set<set<project> >::iterator it;
	for(it=b.begin();it!=b.end();it++)
		if(equal((*it),a))
			return true;
	return false;
}

vector<map<string,int> > build_C(vector<form> f , map<string,set<string> > first , map<string,int> empty , set<string> vt , set<string> vn)//构造项目集族
{
	vector<map<string,int> > action;
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
		map<string,int> buf_action;
		set<project>::iterator it;
		for(it=C.at(i).begin();it!=C.at(i).end();it++)
		{
			string temp = dot_right_one((*it).end,(*it).dot);
			if(temp=="")
				buf_action.insert(pair<string,int>((*it).symbol,-1));
			if(buf.find(temp)!=buf.end())
				continue;//已经跳转过,避免重复计算
			buf.insert(temp);
			set<project> J;
			J = go_closure(temp,C.at(i),f,first,empty);

			if(set_C.find(J)==set_C.end())//新的
			{
				cout<<"i="<<i<<endl;
				C.push_back(J);
				set_C.insert(J);
				if(vt.find(temp)!=vt.end())
					buf_action.insert(pair<string,int>(temp,C.size()-1));
				it=C.at(i).begin();//vector内存重新分配之后会导致迭代器失效!
			}
			else
			{
				for(int j=0;j<C.size();j++)
					if(C.at(j)==J)
						buf_action.insert(pair<string,int>(temp,j));
			}
		}
		action.push_back(buf_action);
	}
	set<project>::iterator it;
	it=C[4].begin();
	cout<<(*it).start<<"->"<<(*it).end<<","<<(*it).symbol<<"  dot="<<(*it).dot<<endl;
	return action;
}

int main()
{
	char s[] = "input_grammar.txt";
	vector<form> v = read_grammar(s);
	set<string> vt = find_vt(v);
	set<string> vn = find_vn(v);
	map<string,int> empty = get_empty(v);
	map<string,set<string> > first,follow,select;
	first = make_first(vt,vn,v,empty);

	vector<map<string,int> > action;
	vector<map<string,int> > go_to;
	action = build_C(v, first, empty, vt, vn);
	cout<<"size:"<<action.size()<<endl<<endl;
	for(int i=0;i<action.size();i++)
	{
		cout<<"line:"<<i<<"   ";
		map<string,int>::iterator it;
	    for(it=action.at(i).begin();it!=action.at(i).end();++it)
	    {
	        cout<<"key: "<<it->first <<" value: "<<it->second<<endl;
	    }
	    cout<<endl;
	}





	// project I;
	// I.start="X";
	// I.dot=0;
	// I.end="S";
	// I.symbol="#";
	// set<project> CI0;
	// CI0.insert(I);
	// CI0 = make_closure(CI0,v,first,empty);
	// //CI0 = go_closure("b",CI0,v,first,empty);

	// cout<<"main:"<<endl;
	// set<project>::iterator it;
	// for(it=CI0.begin();it!=CI0.end();it++)
	// 	cout<<(*it).start<<"->"<<(*it).end<<","<<(*it).symbol<<":"<<(*it).dot<<endl;

	// set<string>::iterator it;
	// for(it=first["D"].begin();it!=first["D"].end();it++)
	// 	cout<<*it<<" ";
	return 0;
}