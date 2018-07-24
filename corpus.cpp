#include "stdafx.h"
#include "corpus.h"

void corpus::loadData(const char* voteFile, int userMin, int itemMin)
{
	nItems = 0;
	nUsers = 0;
	nVotes = 0;

	string uName; //User name
	string iName; //Item name
	float value; //Rating
	long long voteTime; //VoteTime
	map<pair<int, int>, long long> voteMap;

	int nRead = 0; //Progress
	string line;

	ifstream in;
	in.open(voteFile);
	if (in.good())
	{
		fprintf(stderr, "success open file from %s.\n", voteFile);
	}

	if (!in.good())
	{
		fprintf(stderr, "Can't read votes from %s.\n", voteFile);
		exit(1);
	}

	while (getline(in, line))
	{
		vector<string> v;
		SplitString(line, v, ",");
		uName = v[0];
		iName = v[1];
		value = atof(v[2].c_str());
		
		nRead++;
		if (nRead % 100000 == 0)
		{
			fprintf(stderr, ".");
			fflush(stderr);
		}

		if (value > 5 or value < 0)
		{
			printf("Got bad Value");
			exit(1);
		}

		if (uCounts.find(uName) == uCounts.end())
		{
			uCounts[uName] = 0;
		}

		if (iCounts.find(iName) == iCounts.end())
		{
			iCounts[iName] = 0;
		}
		uCounts[uName]++;
		iCounts[iName]++;
	}
	in.close();

	ifstream in2;
	in2.open(voteFile);
	if (!in2.good())
	{
		fprintf(stderr, "Can't read votes from %s.\n", voteFile);
		exit(1);
	}

	nRead = 0;
	while (getline(in2, line))
	{
		vector<string> v;
		SplitString(line, v, ",");
		uName = v[0];
		iName = v[1];
		value = atof(v[2].c_str());

		nRead++;
		if (nRead % 100000 == 0)
		{
			fprintf(stderr, ".");
			fflush(stderr);
		}

		if (uCounts[uName] < userMin or iCounts[iName] < itemMin)
			continue;

		if (itemIds.find(iName) == itemIds.end())
		{
			rItemIds[nItems] = iName;
			itemIds[iName] = nItems++;
		}

		if (userIds.find(uName) == userIds.end())
		{
			rUserIds[nUsers] = uName;
			userIds[uName] = nUsers++;
		}
		voteMap[make_pair(userIds[uName], itemIds[iName])] = 13141314;
	}
	in2.close();

	fprintf(stderr, "\n");
	generateVotes(voteMap);
}

void corpus::generateVotes(map<pair<int, int>, long long>& voteMap)
{
	fprintf(stderr, "\n Generating votes data ");

	for (map<pair<int, int>, long long>::iterator it = voteMap.begin(); it != voteMap.end(); it++)
	{
		vote* v = new vote();
		v->user = it->first.first;
		v->item = it->first.second;
		v->voteTime = it->second;
		V.push_back(v);
	}

	nVotes = V.size();
	random_shuffle(V.begin(), V.end());
}

void corpus::cleanUp()
{
	for (vector<vote*>::iterator it = V.begin(); it != V.end(); it++)
	{
		delete *it;
	}
}

