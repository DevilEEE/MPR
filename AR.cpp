#include "stdafx.h"
#include "AR.h"

void AR::init()
{
	NW = nUsers + K * (nUsers + nItems);
	W = new double[NW];
	bestW = new double[NW];

	getParametersFromVector(W, &beta_user, &gamma_user, &gamma_item, INIT);

	for (int u = 0; u < nUsers; u++)
	{
		for (int k = 0; k < K; k++)
		{
			gamma_user[u][k] = rand()*1.0 / RAND_MAX;
		}
	}

	for (int i = 0; i < nItems; i++)
	{
		for (int k = 0; k < K; k++)
		{
			gamma_item[i][k] = rand()*1.0 / RAND_MAX;
		}
	}
}

void AR::cleanUp()
{
	getParametersFromVector(W, &beta_user, &gamma_user, &gamma_item, FREE);

	delete[] W;
	delete[] bestW;
}

void AR::getParametersFromVector(double* g, double** beta_user, double*** gamma_user, double*** gamma_item, action_t action)
{
	if (action == FREE)
	{
		delete[](*gamma_user);
		delete[](*gamma_item);
		return;
	}

	if (action == INIT)
	{
		*gamma_user = new double*[nUsers];
		*gamma_item = new double*[nItems];
	}

	int ind = 0;

	*beta_user = g + ind;
	ind += nUsers;

	for (int u = 0; u < nUsers; u++)
	{
		(*gamma_user)[u] = g + ind;
		ind += K;
	}
	for (int i = 0; i < nItems; i++)
	{
		(*gamma_item)[i] = g + ind;
		ind += K;
	}

	if (ind != NW)
	{
		printf("Got bad index");
		exit(1);
	}
}

double AR::prediction(int user, int item)
{
	return beta_user[user] + inner(gamma_user[user], gamma_item[item], K);
}

int AR::sampleItem()
{
	while (true)
	{
		int item_id = rand() % nItems;
		if (pos_per_item[item_id].size() == 0 || (int)pos_per_item[item_id].size() == nUsers)
		{
			continue;
		}
		return item_id;
	}
}

void AR::updateFactors(int item_id, int pos_user_id, int neg_user_id, double learn_rate)
{
	double x_uij = beta_user[pos_user_id] - beta_user[neg_user_id];
	x_uij += inner(gamma_user[pos_user_id], gamma_item[item_id], K) - inner(gamma_user[neg_user_id], gamma_item[item_id], K);

	double deri = 1.0 / (1 + exp(x_uij));

	beta_user[pos_user_id] += learn_rate * (deri - biasReg * beta_user[pos_user_id]);
	beta_user[neg_user_id] += learn_rate * (-deri - biasReg * beta_user[neg_user_id]);

	// adjust latent factors
	for (int f = 0; f < K; f++)
	{
		double w_uf = gamma_item[item_id][f];
		double h_if = gamma_user[pos_user_id][f];
		double h_jf = gamma_user[neg_user_id][f];

		gamma_item[item_id][f] += learn_rate * (deri * (h_if - h_jf) - lambda * w_uf);
		gamma_user[pos_user_id][f] += learn_rate * (deri * w_uf - lambda * h_if);
		gamma_user[neg_user_id][f] += learn_rate * (-deri * w_uf - lambda * h_jf);
	}
}

void AR::oneiteration(double learn_rate)
{
	int item_id, pos_user_id, neg_user_id;
	vector<int>* item_matrix = new vector<int>[nItems];
	for (int i = 0; i < nItems; i++)
	{
		for (map<int, long long>::iterator it = pos_per_item[i].begin(); it != pos_per_item[i].end(); it++)
		{
			item_matrix[i].push_back(it->first);
		}
	}

	for (int i = 0; i < num_pos_events; i++)
	{
		// sample an item
		item_id = sampleItem();
		vector<int>& item_users = item_matrix[item_id];

		if (item_users.size() == 0)
		{
			for (map<int, long long>::iterator it = pos_per_item[item_id].begin(); it != pos_per_item[item_id].end(); it++)
			{
				item_users.push_back(it->first);
			}
		}

		// sample pos user
		int rand_num = rand() % item_users.size();
		pos_user_id = item_users.at(rand_num);
		item_users.at(rand_num) = item_users.back();
		item_users.pop_back();

		// sample neg user
		do
		{
			neg_user_id = rand() % nUsers;
		} while (pos_per_item[item_id].find(neg_user_id) != pos_per_item[item_id].end());

		// update factors
		updateFactors(item_id, pos_user_id, neg_user_id, learn_rate);
	}
	delete[] item_matrix;
}

void AR::train(int iterations, double learn_rate)
{
	fprintf(stderr, "%s", ("\n<<<" + toString() + ">>>\n\n").c_str());

	double bestValidAUC = -1;
	int best_iter = 0;

	// SGD
	for (int iter = 1; iter <= iterations; iter++)
	{
		double l_dlStart = clock();
		oneiteration(learn_rate);
		fprintf(stderr, "Iter: %d, took %f\n", iter, clock() - l_dlStart);

		if (iter % 5 == 0)
		{
			double valid, test, std;
			AUC_AR(&valid, &test, &std);
			fprintf(stderr, "[Valid AUC = %f, Test AUC = %f, Test std = %f\n", valid, test, std);

			if (bestValidAUC < valid)
			{
				bestValidAUC = valid;
				best_iter = iter;
				copyBestModel();
			}
			else if (valid < bestValidAUC && iter > best_iter + 50)
			{
				fprintf(stderr, "Overfitted. Exiting... \n");
				break;
			}
		}
	}
	// copy back bestW
	for (int w = 0; w < NW; w++)
	{
		W[w] = bestW[w];
	}

	double valid, test, std;
	AUC_AR(&valid, &test, &std);
	fprintf(stderr, "\n\n <<<BPR>> Test AUC = %f, Test std = %f \n", test, std);
}

string AR::toString()
{
	char str[10000];
	sprintf_s(str, "AR_K_%d_lambada_%.2f_biasReg_%.2f", K, lambda, biasReg);
	return str;
}