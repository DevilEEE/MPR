#pragma once
#include "common.h"
#include "corpus.h"

enum action_t {COPY, INIT, FREE};

class model 
{
public:
	model(corpus* corp) : corp(corp)
	{
		nUsers = corp->nUsers;
		nItems = corp->nItems;
		nVotes = corp->nVotes;

		//leave out two for each user
		test_per_user = new pair<int, long long>[nUsers];
		val_per_user = new pair<int, long long>[nUsers];
		for (int u = 0; u < nUsers; u++)
		{
			test_per_user[u] = make_pair(-1, -1);  // -1 denotes empty
			val_per_user[u] = make_pair(-1, -1);
		}

		//leave out two for each item
		test_per_item = new pair<int, long long>[nItems];
		val_per_item = new pair<int, long long>[nItems];
		for (int i = 0; i < nItems; i++)
		{
			test_per_item[i] = make_pair(-1, -1);  // -1 denotes empty
			val_per_item[i] = make_pair(-1, -1);
		}

		// split into training set and valid set and test set
		pos_per_user = new map<int, long long>[nUsers];
		pos_per_item = new map<int, long long>[nItems];
		for (int x = 0; x < nVotes; x++)
		{
			vote* V = corp->V.at(x);
			int user = V->user;
			int item = V->item;
			long long voteTime = V->voteTime;

			if (test_per_user[user].first == -1)
				test_per_user[user] = make_pair(item, voteTime);
			else if (val_per_user[user].first == -1)
				val_per_user[user] = make_pair(item, voteTime);
			else
				pos_per_user[user][item] = voteTime;

			if (test_per_item[item].first == -1)
				test_per_item[item] = make_pair(user, voteTime);
			else if (val_per_item[item].first == -1)
				val_per_item[item] = make_pair(user, voteTime);
			else
				pos_per_item[item][user] = voteTime;
		}

		num_pos_events = 0;
		for (int u = 0; u < nUsers; u++)
		{
			num_pos_events += pos_per_user[u].size();
		}
	}

	~model()
	{
		delete[] pos_per_user;
		delete[] pos_per_item;
		delete[] test_per_user;
		delete[] val_per_user;
	}
	/* Model parameters */
	int NW; // Total number of parameters
	double* W; // Contiguous version of all parameters
	double* bestW;

	/* Corpus related */
	corpus* corp;
	int nUsers; // Number of users
	int nItems; // Number of items
	int nVotes; // Number of ratings

	map<int, long long>* pos_per_user;
	map<int, long long>* pos_per_item;

	pair<int, long long>* val_per_user;
	pair<int, long long>* test_per_user;

	pair<int, long long>* val_per_item;
	pair<int, long long>* test_per_item;

	int num_pos_events;

	virtual void AUC_IR(double* AUC_val, double* AUC_test, double* std); // AUC for item ranking
	virtual void AUC_AR(double* AUC_val, double* AUC_test, double* std); // AUC for audience retrieval
	virtual void copyBestModel();
	virtual void saveModel(const char* path);
	virtual void loadModel(const char* path);

	virtual string tostring();

private:
	virtual double prediction(int user, int item) = 0;
};