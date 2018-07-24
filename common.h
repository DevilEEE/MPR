#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <iostream>
#include <omp.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include "time.h"
#include <cfloat>
#include <climits>
#include <unordered_set>
#include <random>
#include <windows.h>
#include <cctype>
#include<functional>

using namespace std;

FILE* fopen_(const char* p, const char* m);

typedef struct vote
{
	int user;
	int item;
	int label;
	long long voteTime;
} vote;

inline double inner(double* x, double* y, int K)
{
	double res = 0;
	for (int k = 0; k < K; k++)
	{
		res += x[k] * y[k];
	}
	return res;
}

inline double square(double x)
{
	return x * x;
}

inline double dsquare(double x)
{
	return 2 * x;
}

inline double sigmoid(double x)
{
	return 1.0 / (1.0 + exp(-x));
}

static inline string &ltrim(string &s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

static inline string &rtrim(string &s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

static inline string &trim(string &s)
{
	return ltrim(rtrim(s));
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);













