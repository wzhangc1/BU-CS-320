#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct trace
{
	string behavior;
	unsigned long long addr;
	unsigned long long target;
};

void alwaysTaken(ofstream &fout, vector<trace> v)
{
	int count = 0;
	
	for (int i = 0; i < v.size(); i++)
	{
		if (v[i].behavior == "T")
		{
			count++;
		}
	}
	
	fout << count << ',' << v.size() << ';' << endl;
}

void alwaysNonTaken(ofstream &fout, vector<trace> v)
{
	int count = 0;
	
	for (int i = 0; i < v.size(); i++)
	{
		if (v[i].behavior == "NT")
		{
			count++;
		}
	}
	
	fout << count << ',' << v.size() << ';' << endl;
}

int lg2(int base)
{
	int count = 0;
	
	while (base > 1)
	{
		base /= 2;
		
		count++;
	}
	
	return count;
}

int pw2(int exp)
{
	int tot = 1;
	
	for (int i = 0; i < exp; i++)
	{
		tot *= 2;
	}
	
	return tot;
}

void bimodalSingleBit(ofstream &fout, vector<trace> v)
{
	for (int i = 16; i <= 2048; i *= 2)
	{
		if (i == 64)
		{
			i *= 2;
		}
		
		int count = 0;
		
		bool table[i];
		
		for (int j = 0; j < i; j++)
		{
			table[j] = true;
		}
		
		for (int j = 0; j < v.size(); j++)
		{
			unsigned int addr = v[j].addr & ((1 << lg2(i)) - 1);
						
			if (table[addr])
			{
				if (v[j].behavior == "T")
				{
					count++;
				}
				else
				{
					table[addr] = false;
				}
			}
			else
			{
				if (v[j].behavior == "NT")
				{
					count++;
				}
				else
				{
					table[addr] = true;
				}
			}
		}
			
		fout << count << ',' << v.size() << "; ";
	}
	
	fout << endl;
}

void bimodalTwoBit(ofstream &fout, vector<trace> v)
{
	const int BITS = 2;
	
	for (int i = 16; i <= 2048; i *= 2)
	{
		if (i == 64)
		{
			i *= 2;
		}
		
		int count = 0;
		
		unsigned int table[i];
		
		for (int j = 0; j < i; j++)
		{
			table[j] = pw2(BITS) - 1;
		}
		
		for (int j = 0; j < v.size(); j++)
		{
			unsigned int addr = v[j].addr & ((1 << lg2(i)) - 1);
			
			if (((table[addr] >> (BITS - 1)) & 1) == 1)
			{
				if (v[j].behavior == "T")
				{
					count++;
					
					if ((table[addr] & ((1 << BITS) - 1)) != ((1 << BITS) - 1))
					{
						table[addr]++;
					}
				}
				else
				{
					table[addr]--;
				}
			}
			else
			{
				if (v[j].behavior == "NT")
				{
					count++;
					
					if ((table[addr] & ((1 << BITS) - 1)) != 0)
					{
						table[addr]--;
					}
				}
				else
				{
					table[addr]++;
				}
			}
		}
		
		fout << count << ',' << v.size() << "; ";
	}
	
	fout << endl;
}

void gshare(ofstream &fout, vector<trace> v)
{
	const int BITS = 2;
	const int SIZE = 2048;
	
	for(int i = 3; i <= 11; i++)
	{
		int count = 0;
		unsigned int gr = 0;
		
		unsigned int table[SIZE];
		
		for (int j = 0; j < SIZE; j++)
		{
			table[j] = pw2(BITS) - 1;
		}
		
		for (int j = 0; j < v.size(); j++)
		{
			unsigned int addr = (v[j].addr & ((1 << lg2(SIZE)) - 1)) ^ (gr & ((1 << i) - 1));
			
			if (((table[addr] >> (BITS - 1)) & 1) == 1)
			{
				if (v[j].behavior == "T")
				{
					count++;
					
					if ((table[addr] & ((1 << BITS) - 1)) != ((1 << BITS) - 1))
					{
						table[addr]++;
					}
					
					gr = (gr << 1) + 1;
				}
				else
				{
					table[addr]--;
					
					gr = gr << 1;
				}
			}
			else
			{
				if (v[j].behavior == "NT")
				{
					count++;
					
					if ((table[addr] & ((1 << BITS) - 1)) != 0)
					{
						table[addr]--;
					}
					
					gr = gr << 1;
				}
				else
				{
					table[addr]++;
					
					gr = (gr << 1) + 1;
				}
			}
		}
		
		fout << count << ',' << v.size() << "; ";
	}
	
	fout << endl;
}

void tournament(ofstream &fout, vector<trace> v)
{
	const int BITS = 2;
	const int SIZE = 2048;
	
	int count = 0;
	unsigned int gr = 0;
	
	unsigned int table[SIZE]; //gshare
	unsigned int table2[SIZE]; //bimodal
	unsigned int selector[SIZE];
	
	for (int j = 0; j < SIZE; j++)
	{
		table[j] = pw2(BITS) - 1;
		table2[j] = pw2(BITS) - 1;
		selector[j] = 0;
	}
	
	for (int j = 0; j < v.size(); j++)
	{
		bool bimodalCount = false;
		bool gshareCount = false;
		unsigned int addr = (v[j].addr & ((1 << lg2(SIZE)) - 1)) ^ (gr & ((1 << 11) - 1));
		unsigned int addr2 = v[j].addr & ((1 << 11) - 1);
		
		//gshare
		if (((table[addr] >> (BITS - 1)) & 1) == 1)
		{
			if (v[j].behavior == "T")
			{
				gshareCount = true;
				
				if ((table[addr] & (1 << BITS) - 1) != ((1 << BITS) - 1))
				{
					table[addr]++;
				}
				
				gr = (gr << 1) + 1;
			}
			else
			{
				table[addr]--;
				
				gr = gr << 1;
			}
		}
		else
		{
			if (v[j].behavior == "NT")
			{
				gshareCount = true;
				
				if ((table[addr] & ((1 << BITS) - 1)) != 0)
				{
					table[addr]--;
				}
				
				gr = gr << 1;
			}
			else
			{
				table[addr]++;
				
				gr = (gr << 1) + 1;
			}
		}
		
		//bimodal
		if (((table2[addr2] >> (BITS - 1)) & 1) == 1)
		{
			if (v[j].behavior == "T")
			{
				bimodalCount = true;
				
				if ((table2[addr2] & ((1 << BITS) - 1)) != ((1 << BITS) - 1))
				{
					table2[addr2]++;
				}
			}
			else
			{
				table2[addr2]--;
			}
		}
		else
		{
			if (v[j].behavior == "NT")
			{
				bimodalCount = true;
				
				if ((table2[addr2] & (1 << BITS) - 1) != 0)
				{
					table2[addr2]--;
				}
			}
			else
			{
				table2[addr2]++;
			}
		}
		
		//selector
		if (((selector[v[j].addr & ((1 << 11) - 1)] >> (BITS - 1)) & 1) == 1)
		{
			if (bimodalCount)
			{
				if (!gshareCount)
				{
					if ((selector[v[j].addr & ((1 << 11) - 1)] & ((1 << BITS) - 1)) != ((1 << BITS) - 1))
					{
						selector[v[j].addr & ((1 << 11) - 1)]++;
					}
				}
				
				count++;
			}
			else
			{
				if (gshareCount)
				{
					selector[v[j].addr & ((1 << 11) - 1)]--;
				}
			}
		}
		else
		{
			if (gshareCount)
			{
				if (!bimodalCount)
				{
					if ((selector[v[j].addr & ((1 << 11) - 1)] & ((1 << BITS) - 1)) != 0)
					{
						selector[v[j].addr & ((1 << 11) - 1)]--;
					}
				}
				
				count++;
			}
			else
			{
				if (bimodalCount)
				{
					selector[v[j].addr & ((1 << 11) - 1)]++;
				}
			}
		}
	}
	
	fout << count << ',' << v.size() << ';' << endl;
}

void branchTargetBuffer(ofstream &fout, vector<trace> v)
{
	const int PREDICTOR_SIZE = 512;
	const int BTB_SIZE = 128;
	
	int accessCount = 0;
	int count = 0;
	
	bool table[PREDICTOR_SIZE];
	unsigned long long btb[BTB_SIZE][2];
	
	for (int j = 0; j < PREDICTOR_SIZE; j++)
	{
		table[j] = true;
	}
	
	for (int j = 0; j < v.size(); j++)
	{
		unsigned int addr = v[j].addr & ((1 << lg2(PREDICTOR_SIZE)) - 1);
					
		if (table[addr])
		{
			accessCount++;
			
			if (v[j].behavior == "NT")
			{
				table[addr] = false;
			}
			
			int addr2 = v[j].addr & ((1 << lg2(BTB_SIZE)) - 1);
			
			if (btb[addr2][0] == v[j].addr && btb[addr2][1] == v[j].target)
			{
				count++;
			}
			else
			{
				btb[addr2][0] = v[j].addr;
				btb[addr2][1] = v[j].target;
			}
		}
		else
		{
			if (v[j].behavior == "T")
			{
				table[addr] = true;
			}
		}		
	}
	
	fout << accessCount << ',' << count << ';' << endl;
}

int main(int argc, char *argv[])
{
	string behavior;
	unsigned long long addr;
	unsigned long long target;
	vector<trace> v;
	
	ifstream fin(argv[1]);
	
	while (fin >> std::hex >> addr >> behavior >> std::hex >> target)
	{
		trace t;
		
		t.addr = addr;
		t.behavior = behavior;
		t.target = target;
		
		v.push_back(t);
	}
	
	fin.close();
	
	ofstream fout(argv[2]);
	
	alwaysTaken(fout, v);
	alwaysNonTaken(fout, v);
	bimodalSingleBit(fout, v);
	bimodalTwoBit(fout, v);
	gshare(fout, v);
	tournament(fout, v);
	branchTargetBuffer(fout, v);
	
	fout.close();
	
	return 0;
}
