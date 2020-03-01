#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct trace
{
	char type;
	unsigned long long address;
};

struct cache
{
	bool isValid = false;
	unsigned long long tag = 0;
};
	
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

void directMapped(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZES[4] = {1024, 4096, 16384, 32768};
	const int LINE_SIZE = 32;
	
	for (int i = 0; i < sizeof(CACHE_SIZES)/sizeof(CACHE_SIZES[0]); i++)
	{
		cache table[CACHE_SIZES[i]/LINE_SIZE];
		int count = 0;
		
		for (int j = 0; j < traces.size(); j++)
		{
			int index = (traces[j].address >> lg2(LINE_SIZE)) & ((1 << lg2(CACHE_SIZES[i]/LINE_SIZE)) - 1);
			
			if (table[index].isValid && table[index].tag == traces[j].address >> lg2(CACHE_SIZES[i]))
			{
				count++;
			}
			else
			{
				table[index].isValid = true;
				table[index].tag = traces[j].address >> lg2(CACHE_SIZES[i]);
			}
		}
		
		fout << count << ',' << traces.size() << "; ";
	}
	
	fout << endl;
}

void setAssociative(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	for (int i = 2; i <= 16; i *= 2)
	{
		cache table[CACHE_SIZE/(LINE_SIZE*i)][i];
		int count = 0;
		int lru[CACHE_SIZE/(LINE_SIZE*i)][i];
		
		for (int j = 0; j < CACHE_SIZE/(LINE_SIZE*i); j++)
		{
			for (int k = 0; k < i; k++)
			{
				lru[j][k] = -1;
			}
		}
		
		for (int j = 0; j < traces.size(); j++)
		{
			bool isFound = false;
			int index = (traces[j].address >> lg2(LINE_SIZE)) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
			
			for (int k = 0; !isFound && k < i; k++)
			{
				if (table[index][k].isValid && table[index][k].tag == traces[j].address >> lg2(CACHE_SIZE/i))
				{
					isFound = true;
					count++;
					
					lru[index][k] = j;
				}
			}
			
			if (!isFound)
			{
				int lruIndex = 0;
				
				for (int k = 1; k < i; k++)
				{
					if (lru[index][k] < lru[index][lruIndex])
					{
						lruIndex = k;
					}
				}
				
				table[index][lruIndex].isValid = true;
				table[index][lruIndex].tag = traces[j].address >> lg2(CACHE_SIZE/i);
				
				lru[index][lruIndex] = j;
			}
		}
		
		fout << count << ',' << traces.size() << "; ";
	}
	
	fout << endl;
}

void fullyAssociativeLru(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	cache table[CACHE_SIZE/LINE_SIZE];
	int count = 0;
	int lru[CACHE_SIZE/LINE_SIZE];
	
	for (int i = 0; i < CACHE_SIZE/LINE_SIZE; i++)
	{
		lru[i] = -1;
	}
	
	for (int i = 0; i < traces.size(); i++)
	{
		bool isFound = false;
		
		for (int j = 0; !isFound && j < CACHE_SIZE/LINE_SIZE; j++)
		{
			if (table[j].isValid && table[j].tag == traces[i].address >> lg2(LINE_SIZE))
			{
				isFound = true;
				count++;
				
				lru[j] = i;
			}
		}
		
		if (!isFound)
		{
			int lruIndex = 0;
			
			for (int j = 1; j < CACHE_SIZE/LINE_SIZE; j++)
			{
				if (lru[j] < lru[lruIndex])
				{
					lruIndex = j;
				}
			}
			
			table[lruIndex].isValid = true;
			table[lruIndex].tag = traces[i].address >> lg2(LINE_SIZE);
			
			lru[lruIndex] = i;
		}
	}
	
	fout << count << ',' << traces.size() << "; " << endl;
}

void fullyAssociativeHotCold(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	bool lru[(CACHE_SIZE/LINE_SIZE) - 1];
	cache table[CACHE_SIZE/LINE_SIZE];
	int count = 0;
	
	for (int i = 0; i < (CACHE_SIZE/LINE_SIZE) - 1; i++)
	{
		lru[i] = false;
	}
	
	for (int i = 0; i < traces.size(); i++)
	{
		bool isFound = false;
		
		for (int j = 0; !isFound && j < CACHE_SIZE/LINE_SIZE; j++)
		{
			if (table[j].isValid && table[j].tag == traces[i].address >> lg2(LINE_SIZE))
			{
				isFound = true;
				count++;
				
				for (int k = j + (CACHE_SIZE/LINE_SIZE) - 1; k > 0; k = (k - 1)/2)
				{
					if (k % 2 == 0)
					{
						lru[(k - 1)/2] = true;
					}
					else
					{
						lru[(k - 1)/2] = false;
					}
				}
			}
		}
		
		if (!isFound)
		{
			int j = 0;
			
			while (j < (CACHE_SIZE/LINE_SIZE) - 1)
			{
				if (!lru[j])
				{
					lru[j] = true;
					
					j = (j*2) + 2;
				}
				else
				{
					lru[j] = false;
					
					j = (j*2) + 1;
				}
			}
			
			table[j - (CACHE_SIZE/LINE_SIZE) + 1].isValid = true;
			table[j - (CACHE_SIZE/LINE_SIZE) + 1].tag = traces[i].address >> lg2(LINE_SIZE);
		}
	}
	
	fout << count << ',' << traces.size() << "; " << endl;
}

void fullyAssociative(ofstream& fout, vector<trace> traces)
{
	fullyAssociativeLru(fout, traces);
	fullyAssociativeHotCold(fout, traces);
}

void setAssociativeNoAllocationWriteMiss(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	for (int i = 2; i <= 16; i *= 2)
	{
		cache table[CACHE_SIZE/(LINE_SIZE*i)][i];
		int count = 0;
		int lru[CACHE_SIZE/(LINE_SIZE*i)][i];
		
		for (int j = 0; j < CACHE_SIZE/(LINE_SIZE*i); j++)
		{
			for (int k = 0; k < i; k++)
			{
				lru[j][k] = -1;
			}
		}
		
		for (int j = 0; j < traces.size(); j++)
		{
			bool isFound = false;
			int index = (traces[j].address >> lg2(LINE_SIZE)) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
			
			for (int k = 0; !isFound && k < i; k++)
			{
				if (table[index][k].isValid && table[index][k].tag == traces[j].address >> lg2(CACHE_SIZE/i))
				{
					isFound = true;
					count++;
					
					lru[index][k] = j;
				}
			}
			
			if (!isFound && traces[j].type != 'S')
			{
				int lruIndex = 0;
				
				for (int k = 1; k < i; k++)
				{
					if (lru[index][k] < lru[index][lruIndex])
					{
						lruIndex = k;
					}
				}
				
				table[index][lruIndex].isValid = true;
				table[index][lruIndex].tag = traces[j].address >> lg2(CACHE_SIZE/i);
				
				lru[index][lruIndex] = j;
			}
		}
		
		fout << count << ',' << traces.size() << "; ";
	}
	
	fout << endl;
}

void setAssociativeNextLinePrefetching(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	for (int i = 2; i <= 16; i *= 2)
	{
		cache table[CACHE_SIZE/(LINE_SIZE*i)][i];
		int count = 0;
		int lru[CACHE_SIZE/(LINE_SIZE*i)][i];
		
		for (int j = 0; j < CACHE_SIZE/(LINE_SIZE*i); j++)
		{
			for (int k = 0; k < i; k++)
			{
				lru[j][k] = -1;
			}
		}
		
		for (int j = 0; j < traces.size(); j++)
		{
			bool isFound = false;
			int index = (traces[j].address >> lg2(LINE_SIZE)) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
			
			for (int k = 0; !isFound && k < i; k++)
			{
				if (table[index][k].isValid && table[index][k].tag == traces[j].address >> lg2(CACHE_SIZE/i))
				{
					isFound = true;
					count++;
					
					lru[index][k] = j*2;
				}
			}
			
			if (!isFound)
			{
				int lruIndex = 0;
				
				for (int k = 1; k < i; k++)
				{
					if (lru[index][k] < lru[index][lruIndex])
					{
						lruIndex = k;
					}
				}
				
				table[index][lruIndex].isValid = true;
				table[index][lruIndex].tag = traces[j].address >> lg2(CACHE_SIZE/i);
				
				lru[index][lruIndex] = j*2;
			}
			
			isFound = false;
			index = ((traces[j].address >> lg2(LINE_SIZE)) + 1) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
			
			for (int k = 0; !isFound && k < i; k++)
			{
				if (table[index][k].isValid && table[index][k].tag == (traces[j].address + LINE_SIZE) >> lg2(CACHE_SIZE/i))
				{
					isFound = true;
					
					lru[index][k] = (j*2) + 1;
				}
			}
			
			if (!isFound)
			{
				int lruIndex = 0;
				
				for (int k = 1; k < i; k++)
				{
					if (lru[index][k] < lru[index][lruIndex])
					{
						lruIndex = k;
					}
				}
				
				table[index][lruIndex].isValid = true;
				table[index][lruIndex].tag = (traces[j].address + LINE_SIZE) >> lg2(CACHE_SIZE/i);
				
				lru[index][lruIndex] = (j*2) + 1;
			}
		}
		
		fout << count << ',' << traces.size() << "; ";
		cout << count << ',' << traces.size() << "; ";
	}
	
	fout << endl;
}

void prefetchMiss(ofstream& fout, vector<trace> traces)
{
	const int CACHE_SIZE = 16384;
	const int LINE_SIZE = 32;
	
	for (int i = 2; i <= 16; i *= 2)
	{
		cache table[CACHE_SIZE/(LINE_SIZE*i)][i];
		int count = 0;
		int lru[CACHE_SIZE/(LINE_SIZE*i)][i];
		
		for (int j = 0; j < CACHE_SIZE/(LINE_SIZE*i); j++)
		{
			for (int k = 0; k < i; k++)
			{
				lru[j][k] = -1;
			}
		}
		
		for (int j = 0; j < traces.size(); j++)
		{
			bool isFound = false;
			int index = (traces[j].address >> lg2(LINE_SIZE)) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
			
			for (int k = 0; !isFound && k < i; k++)
			{
				if (table[index][k].isValid && table[index][k].tag == traces[j].address >> lg2(CACHE_SIZE/i))
				{
					isFound = true;
					count++;
					
					lru[index][k] = j*2;
				}
			}
			
			if (!isFound)
			{
				int lruIndex = 0;
				
				for (int k = 1; k < i; k++)
				{
					if (lru[index][k] < lru[index][lruIndex])
					{
						lruIndex = k;
					}
				}
				
				table[index][lruIndex].isValid = true;
				table[index][lruIndex].tag = traces[j].address >> lg2(CACHE_SIZE/i);
				
				lru[index][lruIndex] = j*2;
				
				isFound = false;
				index = ((traces[j].address >> lg2(LINE_SIZE)) + 1) & ((1 << lg2(CACHE_SIZE/(LINE_SIZE*i))) - 1);
				
				for (int k = 0; !isFound && k < i; k++)
				{
					if (table[index][k].isValid && table[index][k].tag == (traces[j].address + LINE_SIZE) >> lg2(CACHE_SIZE/i))
					{
						isFound = true;
						
						lru[index][k] = (j*2) + 1;
					}
				}
				
				if (!isFound)
				{
					int lruIndex = 0;
					
					for (int k = 1; k < i; k++)
					{
						if (lru[index][k] < lru[index][lruIndex])
						{
							lruIndex = k;
						}
					}
					
					table[index][lruIndex].isValid = true;
					table[index][lruIndex].tag = (traces[j].address + LINE_SIZE) >> lg2(CACHE_SIZE/i);
					
					lru[index][lruIndex] = (j*2) + 1;
				}
			}
		}
		
		fout << count << ',' << traces.size() << "; ";
	}
	
	fout << endl;
}

int main(int argc, char *argv[])
{
	trace tempTrace;
	vector<trace> traces;
	
	ifstream fin(argv[1]);
	
	while (fin >> tempTrace.type >> std::hex >> tempTrace.address)
	{
		traces.push_back(tempTrace);
	}
	
	fin.close();
	
	ofstream fout(argv[2]);
	
	directMapped(fout, traces);
	setAssociative(fout, traces);
	fullyAssociative(fout, traces);
	setAssociativeNoAllocationWriteMiss(fout, traces);
	setAssociativeNextLinePrefetching(fout, traces);
	prefetchMiss(fout, traces);
	
	fout.close();
	
	return 0;
}
