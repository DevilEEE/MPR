#pragma once
#include "common.h"

class corpus
{
public:
	corpus() {}
	~corpus() {}
	vector<vote*> V;
	int nUsers; //Number of users;
	int nItems; //Number of items;
	int nVotes; //Number of ratings

	map<string, int> userIds; //Maps a user's string-valued ID to an integer
	map<string, int> itemIds; //Maps a item's string-valued ID to an integer

	map<int, string> rUserIds; //Inverse of the above maps
	map<int, string> rItemIds;

	map<string, int> uCounts;
	map<string, int> iCounts;

	void loadData(const char* voteFile, int userMin, int itemMin);
	void cleanUp();
	
private:
	void generateVotes(map<pair<int, int>, long long>& voteMap);
};
